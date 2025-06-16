/*  Copyright (C) 2014-2022 FastoGT. All right reserved.
    This file is part of fastocloud.
    fastocloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    fastocloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with fastocloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "server/process_slave_wrapper.h"

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <common/convert2string.h>
#include <common/daemon/commands/activate_info.h>
#include <common/daemon/commands/get_log_info.h>
#include <common/daemon/commands/restart_info.h>
#include <common/daemon/commands/stop_info.h>
#include <common/file_system/string_path_utils.h>
#include <common/license/expire_license.h>
#include <common/net/http_client.h>
#include <common/net/net.h>

#include "base/config_fields.h"
#include "base/constants.h"
#include "base/utils.h"

#include "gpu_stats/perf_monitor.h"

#include "server/child_stream.h"
#include "server/daemon/client.h"
#include "server/daemon/commands.h"
#include "server/daemon/commands_info/service/details/shots.h"
#include "server/daemon/commands_info/service/server_info.h"
#include "server/daemon/commands_info/stream/get_log_info.h"
#include "server/daemon/commands_info/stream/restart_info.h"
#include "server/daemon/commands_info/stream/start_info.h"
#include "server/daemon/commands_info/stream/stop_info.h"
#include "server/daemon/server.h"
#include "server/http/handler.h"
#include "server/http/server.h"
#include "server/options/options.h"

#include "stream_commands/commands.h"

#if defined(OS_WIN)
#undef SetPort
#endif

#define API_KEY_PARAM "API-KEY"

#define WS_UPDATES "updates"

namespace {

const auto kDaysInMonth = 365.2425 / 12;
const auto kServicePriceInUSDPerDay = 25.0 / kDaysInMonth;
const auto kSecondsInTheDay = 24 * 3600;

const auto kServicePriceInUSDPerSec = kServicePriceInUSDPerDay / kSecondsInTheDay;
const auto kClosedDaemon = common::make_errno_error("Connection closed", EAGAIN);

common::Optional<common::file_system::ascii_file_string_path> MakeStreamConfigJsonPath(
    const std::string& feedback_dir) {
  common::file_system::ascii_directory_string_path dir(feedback_dir);
  return dir.MakeFileStringPath(CONFIG_FILE_NAME);
}

common::Optional<common::file_system::ascii_file_string_path> MakeStreamLogPath(const std::string& feedback_dir) {
  common::file_system::ascii_directory_string_path dir(feedback_dir);
  return dir.MakeFileStringPath(LOGS_FILE_NAME);
}

common::Optional<common::file_system::ascii_file_string_path> MakeStreamPipelinePath(const std::string& feedback_dir) {
  common::file_system::ascii_directory_string_path dir(feedback_dir);
  return dir.MakeFileStringPath(DUMP_FILE_NAME);
}

}  // namespace

namespace fastocloud {
namespace server {
namespace {
template <typename T>
common::ErrnoError ParsePostClientRequest(ProtocoledDaemonClient* dclient,
                                          const common::http::HttpRequest& req,
                                          common::serializer::JsonSerializer<T>* result,
                                          bool access = true) {
  if (!dclient || !result) {
    return common::make_errno_error_inval();
  }

  if (access) {
    common::http::header_t found_key_in_headers;
    if (!req.FindHeaderByKey(API_KEY_PARAM, false, &found_key_in_headers)) {
      return common::make_errno_error("Don't have permissions", EACCES);
    }

    const auto expire_key = common::license::make_license<common::license::expire_key_t>(found_key_in_headers.value);
    if (!expire_key) {
      return common::make_errno_error("Invalid access key", EACCES);
    }

    common::time64_t tm;
    bool is_valid = common::license::GetExpireTimeFromKey(PROJECT_NAME_LOWERCASE, *expire_key, &tm);
    if (!is_valid) {
      common::Error err = common::make_error("Invalid expire key");
      return common::make_errno_error(err->GetDescription(), EINVAL);
    }

    dclient->SetVerified(true, tm);

    if (!dclient->HaveFullAccess()) {
      return common::make_errno_error("Don't have permissions", EACCES);
    }
  }

  const auto params_ptr = req.GetBody();
  const char* data = params_ptr.data();
  json_object* jinfo = json_tokener_parse(data);
  if (!jinfo) {
    return common::make_errno_error("Can't parse request", EINVAL);
  }

  common::Error err_des = result->DeSerialize(jinfo);
  json_object_put(jinfo);
  if (err_des) {
    const std::string err_str = err_des->GetDescription();
    return common::make_errno_error(err_str, EINVAL);
  }
  return common::ErrnoError();
}

common::ErrnoError ParseGetClientRequest(ProtocoledDaemonClient* dclient,
                                         const common::http::HttpRequest& req,
                                         bool access = true) {
  if (!dclient) {
    return common::make_errno_error_inval();
  }

  if (access) {
    common::http::header_t found_key_in_headers;
    if (!req.FindHeaderByKey(API_KEY_PARAM, false, &found_key_in_headers)) {
      return common::make_errno_error("Don't have permissions", EACCES);
    }

    const auto expire_key = common::license::make_license<common::license::expire_key_t>(found_key_in_headers.value);
    if (!expire_key) {
      return common::make_errno_error("Invalid access key", EACCES);
    }

    common::time64_t tm;
    bool is_valid = common::license::GetExpireTimeFromKey(PROJECT_NAME_LOWERCASE, *expire_key, &tm);
    if (!is_valid) {
      common::Error err = common::make_error("Invalid expire key");
      return common::make_errno_error(err->GetDescription(), EINVAL);
    }

    dclient->SetVerified(true, tm);

    if (!dclient->HaveFullAccess()) {
      return common::make_errno_error("Don't have permissions", EACCES);
    }
  }

  return common::ErrnoError();
}

template <typename T>
common::ErrnoError ParseStreamRequest(ProcessSlaveWrapper::stream_client_t* pclient,
                                      const fastotv::protocol::request_t* req,
                                      common::serializer::JsonSerializer<T>* result) {
  if (!req->params || !pclient || !req || !result) {
    return common::make_errno_error_inval();
  }

  const char* params_ptr = req->params->c_str();
  json_object* jinfo = json_tokener_parse(params_ptr);
  if (!jinfo) {
    return common::make_errno_error_inval();
  }

  common::Error err_des = result->DeSerialize(jinfo);
  json_object_put(jinfo);
  if (err_des) {
    const std::string err_str = err_des->GetDescription();
    return common::make_errno_error(err_str, EAGAIN);
  }

  return common::ErrnoError();
}

typedef HttpHandler VodsHandler;
typedef HttpServer VodsServer;
typedef VodsHandler CodsHandler;
typedef VodsServer CodsServer;

}  // namespace

struct ProcessSlaveWrapper::NodeStats {
  NodeStats()
      : prev(),
        prev_nshot(),
        timestamp(common::time::current_utc_mstime()),
        balance(0.0),
        gpu_load_nvidia_(0.0),
        perf_monitor_nvidia_(nullptr),
        perf_monitor_nvidia_thread_(),
        gpu_load_intel_(0.0),
        perf_monitor_intel_(nullptr),
        perf_monitor_intel_thread_() {
    perf_monitor_nvidia_ = gpu_stats::CreateNvidiaPerfMonitor(&gpu_load_nvidia_);
    perf_monitor_intel_ = gpu_stats::CreateIntelPerfMonitor(&gpu_load_intel_);
  }

  ~NodeStats() {
    delete perf_monitor_nvidia_;
    delete perf_monitor_intel_;
  }

  void Start() {
    if (perf_monitor_nvidia_) {
      perf_monitor_nvidia_thread_ = std::thread([this] {
        INFO_LOG() << "Nvidia GPU monitor have started";
        bool status = perf_monitor_nvidia_->Exec();
        INFO_LOG() << "Nvidia GPU monitor finished: " << status;
      });
    }
    if (perf_monitor_intel_) {
      perf_monitor_intel_thread_ = std::thread([this] {
        INFO_LOG() << "Intel GPU monitor have started";
        bool status = perf_monitor_intel_->Exec();
        INFO_LOG() << "Intel GPU monitor finished: " << status;
      });
    }
  }

  void Stop() {
    if (perf_monitor_nvidia_) {
      perf_monitor_nvidia_->Stop();
      if (perf_monitor_nvidia_thread_.joinable()) {
        perf_monitor_nvidia_thread_.join();
      }
    }

    if (perf_monitor_intel_) {
      perf_monitor_intel_->Stop();
      if (perf_monitor_intel_thread_.joinable()) {
        perf_monitor_intel_thread_.join();
      }
    }
  }

  double CalcGPULoad() const {
    if (perf_monitor_nvidia_ && perf_monitor_intel_) {
      return static_cast<double>(gpu_load_nvidia_ + gpu_load_intel_) / 2.0;
    }

    if (perf_monitor_nvidia_) {
      return gpu_load_nvidia_;
    }

    if (perf_monitor_intel_) {
      return gpu_load_intel_;
    }

    return 0.0;
  }

  service::CpuShot prev;
  service::NetShot prev_nshot;
  fastotv::timestamp_t timestamp;
  service::ServerInfo::cost_t balance;

 private:
  double gpu_load_nvidia_;
  gpu_stats::IPerfMonitor* perf_monitor_nvidia_;
  std::thread perf_monitor_nvidia_thread_;

  double gpu_load_intel_;
  gpu_stats::IPerfMonitor* perf_monitor_intel_;
  std::thread perf_monitor_intel_thread_;
};

ProcessSlaveWrapper::ProcessSlaveWrapper(const Config& config)
    : config_(config),
      process_argc_(0),
      process_argv_(nullptr),
      loop_(nullptr),
      http_server_(nullptr),
      http_handler_(nullptr),
      vods_server_(nullptr),
      vods_handler_(nullptr),
      cods_server_(nullptr),
      cods_handler_(nullptr),
      ping_client_timer_(INVALID_TIMER_ID),
      check_cods_vods_timer_(INVALID_TIMER_ID),
      check_old_files_timer_(INVALID_TIMER_ID),
      node_stats_timer_(INVALID_TIMER_ID),
      quit_cleanup_timer_(INVALID_TIMER_ID),
      check_license_timer_(INVALID_TIMER_ID),
      node_stats_(new NodeStats),
      folders_for_monitor_(),
      stoped_(false),
      online_clients_(0),
      requests_count_(0),
      connections_count_(0)  {
  loop_ = new DaemonServer(config.host, this);
  loop_->SetName("client_server");

  http_handler_ = new HttpHandler(config.hls_dir);
  auto hls = GetHostAndPortFromGurl(config.hls_host);
  CHECK(hls && config.hls_host.SchemeIsHTTPOrHTTPS());
  http_server_ = new HttpServer(*hls, http_handler_);
  http_server_->SetName("http_server");

  vods_handler_ = new HttpHandler(config.vods_dir);
  auto vods = GetHostAndPortFromGurl(config.vods_host);
  CHECK(vods && config.vods_host.SchemeIsHTTPOrHTTPS());
  vods_server_ = new HttpServer(*vods, vods_handler_);
  vods_server_->SetName("vods_server");

  cods_handler_ = new HttpHandler(config.cods_dir);
  auto cods = GetHostAndPortFromGurl(config.cods_host);
  CHECK(cods && config.vods_host.SchemeIsHTTPOrHTTPS());
  cods_server_ = new HttpServer(*cods, cods_handler_);
  cods_server_->SetName("cods_server");
}

common::ErrnoError ProcessSlaveWrapper::SendStopDaemonRequest(const common::net::HostAndPort& host) {
  common::net::socket_info client_info;
  common::ErrnoError err = common::net::connect(host, common::net::ST_SOCK_STREAM, nullptr, &client_info);
  if (err) {
    return err;
  }

  std::unique_ptr<ProtocoledDaemonClient> connection(new ProtocoledDaemonClient(nullptr, client_info));
  auto url = common::uri::GURL("http://" + common::ConvertToString(host) + "/" + DAEMON_STOP_SERVICE);
  err = connection->StopMe(url);
  if (err) {
    ignore_result(connection->Close());
    return err;
  }

  ignore_result(connection->Close());
  return common::ErrnoError();
}

common::ErrnoError ProcessSlaveWrapper::SendRestartDaemonRequest(const common::net::HostAndPort& host) {
  common::net::socket_info client_info;
  common::ErrnoError err = common::net::connect(host, common::net::ST_SOCK_STREAM, nullptr, &client_info);
  if (err) {
    return err;
  }

  std::unique_ptr<ProtocoledDaemonClient> connection(new ProtocoledDaemonClient(nullptr, client_info));
  auto url = common::uri::GURL("http://" + common::ConvertToString(host) + "/" + DAEMON_RESTART_SERVICE);
  err = connection->RestartMe(url);
  if (err) {
    ignore_result(connection->Close());
    return err;
  }

  ignore_result(connection->Close());
  return common::ErrnoError();
}

ProcessSlaveWrapper::~ProcessSlaveWrapper() {
  destroy(&cods_server_);
  destroy(&cods_handler_);
  destroy(&vods_server_);
  destroy(&vods_handler_);
  destroy(&http_server_);
  destroy(&http_handler_);
  destroy(&loop_);
  destroy(&node_stats_);
}

common::ErrnoError ProcessSlaveWrapper::Prepare() {
  common::ErrnoError errn = CreateAndCheckDir(config_.feedback_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(config_.hls_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(config_.vods_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(config_.cods_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(config_.timeshifts_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(config_.proxy_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(config_.data_dir);
  if (errn) {
    return errn;
  }

  const common::file_system::ascii_directory_string_path http_root(config_.hls_dir);
  folders_for_monitor_.push_back(http_root);

  const common::file_system::ascii_directory_string_path vods_root(config_.vods_dir);
  folders_for_monitor_.push_back(vods_root);

  const common::file_system::ascii_directory_string_path cods_root(config_.cods_dir);
  folders_for_monitor_.push_back(cods_root);

  const common::file_system::ascii_directory_string_path timeshift_root(config_.timeshifts_dir);
  folders_for_monitor_.push_back(timeshift_root);

  return common::ErrnoError();
}

int ProcessSlaveWrapper::Exec(int argc, char** argv) {
  process_argc_ = argc;
  process_argv_ = argv;

  common::ErrnoError err = Prepare();
  if (err) {
    ERROR_LOG() << "Service prepare error: " << err->GetDescription();
    return EXIT_FAILURE;
  }

  // gpu statistic monitor
  HttpServer* http_server = static_cast<HttpServer*>(http_server_);
  auto http_thread = std::thread([http_server] {
    common::ErrnoError err = http_server->Bind(true);
    if (err) {
      ERROR_LOG() << "HttpServer bind error: " << err->GetDescription();
      return;
    }

    err = http_server->Listen(5);
    if (err) {
      ERROR_LOG() << "HttpServer listen error: " << err->GetDescription();
      return;
    }

    INFO_LOG() << "HttpServer have started address: " << common::ConvertToString(http_server->GetHost());
    int res = http_server->Exec();
    INFO_LOG() << "HttpServer have finished: " << res;
  });

  HttpServer* vods_server = static_cast<HttpServer*>(vods_server_);
  auto vods_thread = std::thread([vods_server] {
    common::ErrnoError err = vods_server->Bind(true);
    if (err) {
      ERROR_LOG() << "VodsServer bind error: " << err->GetDescription();
      return;
    }

    err = vods_server->Listen(5);
    if (err) {
      ERROR_LOG() << "VodsServer listen error: " << err->GetDescription();
      return;
    }

    INFO_LOG() << "VodsServer have started address: " << common::ConvertToString(vods_server->GetHost());
    int res = vods_server->Exec();
    INFO_LOG() << "VodsServer have finished: " << res;
  });

  HttpServer* cods_server = static_cast<HttpServer*>(cods_server_);
  auto cods_thread = std::thread([cods_server] {
    common::ErrnoError err = cods_server->Bind(true);
    if (err) {
      ERROR_LOG() << "CodsServer bind error: " << err->GetDescription();
      return;
    }

    err = cods_server->Listen(5);
    if (err) {
      ERROR_LOG() << "CodsServer listen error: " << err->GetDescription();
      return;
    }

    INFO_LOG() << "CodsServer have started address: " << common::ConvertToString(cods_server->GetHost());
    int res = cods_server->Exec();
    INFO_LOG() << "CodsServer have finished: " << res;
  });

  int res = EXIT_FAILURE;
  DaemonServer* server = static_cast<DaemonServer*>(loop_);
  err = server->Bind(true);
  if (err) {
    ERROR_LOG() << "DaemonServer bind error: " << err->GetDescription();
    vods_server->Stop();
    cods_server->Stop();
    http_server->Stop();
    goto finished;
  }

  err = server->Listen(5);
  if (err) {
    ERROR_LOG() << "DaemonServer listen error: " << err->GetDescription();
    vods_server->Stop();
    cods_server->Stop();
    http_server->Stop();
    goto finished;
  }

  node_stats_->Start();
  node_stats_->prev = service::GetMachineCpuShot();
  node_stats_->prev_nshot = service::GetMachineNetShot();
  node_stats_->timestamp = common::time::current_utc_mstime();
  node_stats_->balance = 0.0;

  INFO_LOG() << "DaemonServer have started address: " << common::ConvertToString(server->GetHost());
  res = server->Exec();

finished:
  if (vods_thread.joinable()) {
    vods_thread.join();
  }
  if (cods_thread.joinable()) {
    cods_thread.join();
  }
  if (http_thread.joinable()) {
    http_thread.join();
  }
  node_stats_->Stop();
  return res;
}

bool ProcessSlaveWrapper::IsStoped() const {
  return stoped_;
}

void ProcessSlaveWrapper::PreLooped(common::libev::IoLoop* server) {
  ping_client_timer_ = server->CreateTimer(ping_timeout_clients_seconds, true);
  check_cods_vods_timer_ = server->CreateTimer(config_.cods_ttl / 2, true);
  check_old_files_timer_ = server->CreateTimer(config_.files_ttl / 10, true);
  node_stats_timer_ = server->CreateTimer(node_stats_send_seconds, true);
  check_license_timer_ = server->CreateTimer(check_license_timeout_seconds, true);
}

void ProcessSlaveWrapper::Accepted(common::libev::IoClient* client) {
  UNUSED(client);
  online_clients_++;
  connections_count_++;
}

void ProcessSlaveWrapper::Moved(common::libev::IoLoop* server, common::libev::IoClient* client) {
  UNUSED(server);
  UNUSED(client);
  online_clients_--;
}

void ProcessSlaveWrapper::Closed(common::libev::IoClient* client) {
  ProtocoledDaemonClient* dclient = dynamic_cast<ProtocoledDaemonClient*>(client);
  if (dclient && dclient->IsVerified()) {
    auto step = dclient->Step();
    if (step != common::libev::websocket::ZERO) {
      ignore_result(dclient->SendEOS());
    }
  }
  online_clients_--;
}

void ProcessSlaveWrapper::TimerEmited(common::libev::IoLoop* server, common::libev::timer_id_t id) {
  if (ping_client_timer_ == id) {
    std::vector<common::libev::IoClient*> online_clients = server->GetClients();
    for (size_t i = 0; i < online_clients.size(); ++i) {
      common::libev::IoClient* client = online_clients[i];
      ProtocoledDaemonClient* dclient = dynamic_cast<ProtocoledDaemonClient*>(client);
      if (dclient) {
        if (!dclient->IsVerified()) {
          if (dclient->GetTTL() >= dead_client_ttl_in_seconds * 1000) {
            INFO_LOG() << "Seems dead client[" << client->GetFormatedName() << "], from server["
                       << server->GetFormatedName() << "] let's close it, " << online_clients.size()
                       << " client(s) connected.";
            ignore_result(client->Close());
            delete client;
          }
          continue;
        }
      }
    }
  } else if (check_cods_vods_timer_ == id) {
    fastotv::timestamp_t current_time = common::time::current_utc_mstime();
    auto childs = server->GetChilds();
    for (auto* child : childs) {
      auto channel = dynamic_cast<ChildStream*>(child);
      if (channel && channel->IsCOD()) {
        fastotv::timestamp_t cod_last_update = channel->GetLastUpdate();
        fastotv::timestamp_t ts_diff = current_time - cod_last_update;
        if (ts_diff > config_.cods_ttl * 1000) {
          ignore_result(channel->Stop());
        }
      }
    }
  } else if (check_old_files_timer_ == id) {
    for (auto it = folders_for_monitor_.begin(); it != folders_for_monitor_.end(); ++it) {
      const common::file_system::ascii_directory_string_path folder = *it;
      const time_t max_life_time = common::time::current_utc_mstime() / 1000 - config_.files_ttl;
      RemoveOldFilesByTime(folder, max_life_time, "*" CHUNK_EXT, true);
    }
  } else if (node_stats_timer_ == id) {
    const auto node_stats = MakeServiceInfoStats();
    common::json::WsDataJson req;
    common::Error err_ser = StatisitcServiceBroadcast(node_stats, &req);
    if (err_ser) {
      return;
    }

    BroadcastClients(req);
  } else if (quit_cleanup_timer_ == id) {
    loop_->Stop();
  } else if (check_license_timer_ == id) {
    CheckLicenseExpired(server);
  }
}

void ProcessSlaveWrapper::Accepted(common::libev::IoChild* child) {
  UNUSED(child);
}

void ProcessSlaveWrapper::Moved(common::libev::IoLoop* server, common::libev::IoChild* child) {
  UNUSED(server);
  UNUSED(child);
}

void ProcessSlaveWrapper::ChildStatusChanged(common::libev::IoChild* child, int status, int signal) {
  auto channel = static_cast<ChildStream*>(child);
  channel->CleanUp();
  const auto sid = channel->GetStreamID();

  INFO_LOG() << "Successful finished children id: " << sid << "\nStream id: " << sid
             << ", exit with status: " << (status ? "FAILURE" : "SUCCESS") << ", signal: " << signal;

  loop_->UnRegisterChild(channel);

  delete channel;

  stream::QuitStatusInfo ch_status_info(sid, status, signal);
  common::json::WsDataJson req;
  common::Error err_ser = QuitStatusStreamBroadcast(ch_status_info, &req);
  if (err_ser) {
    return;
  }

  BroadcastClients(req);
}

void ProcessSlaveWrapper::StopImpl() {
  DaemonServer* server = static_cast<DaemonServer*>(loop_);
  auto childs = server->GetChilds();
  for (auto* child : childs) {
    auto channel = static_cast<ChildStream*>(child);
    ignore_result(channel->Stop());
  }

  vods_server_->Stop();
  cods_server_->Stop();
  http_server_->Stop();

  quit_cleanup_timer_ = loop_->CreateTimer(cleanup_seconds, false);
}

ChildStream* ProcessSlaveWrapper::FindChildByID(fastotv::stream_id_t cid) const {
  CHECK(loop_->IsLoopThread());
  DaemonServer* server = static_cast<DaemonServer*>(loop_);
  auto childs = server->GetChilds();
  for (auto* child : childs) {
    ChildStream* channel = static_cast<ChildStream*>(child);
    if (channel->GetStreamID() == cid) {
      return channel;
    }
  }

  return nullptr;
}

void ProcessSlaveWrapper::BroadcastClients(const common::json::WsDataJson& req) {
  std::vector<common::libev::IoClient*> clients = loop_->GetClients();
  for (size_t i = 0; i < clients.size(); ++i) {
    ProtocoledDaemonClient* dclient = dynamic_cast<ProtocoledDaemonClient*>(clients[i]);
    if (dclient && dclient->IsVerified()) {
      common::ErrnoError err = dclient->Broadcast(req);
      if (err) {
        WARNING_LOG() << "BroadcastClients error: " << err->GetDescription();
      }
    }
  }
}

common::ErrnoError ProcessSlaveWrapper::ProcessReceived(ProtocoledDaemonClient* hclient,
                                                        const char* request,
                                                        size_t req_len) {
  static const common::libev::http::HttpServerInfo hinf(PROJECT_NAME_TITLE "/" PROJECT_VERSION, PROJECT_DOMAIN);
  common::http::HttpRequest hrequest;
  const auto result = common::http::parse_http_request(request, req_len, &hrequest);
  DEBUG_LOG() << "Http request:\n" << request;

  common::http::headers_t extra_headers = {{"Access-Control-Allow-Origin", "*"}};
  if (result.second) {
    const std::string err_str = result.second->GetDescription();
    return common::make_errno_error(err_str, EAGAIN);
  }

  auto url = hrequest.GetURL();
  auto route = url.path();
  auto method = hrequest.GetMethod();
  if (method == common::http::HM_POST) {
    if (route == "/" DAEMON_STOP_SERVICE) {  // +
      return HandleRequestClientStopService(hclient, hrequest);
    } else if (route == "/" DAEMON_RESTART_SERVICE) {  // +
      return HandleRequestClientRestartService(hclient, hrequest);
    } else if (route == "/" DAEMON_START_STREAM) {  // +
      return HandleRequestClientStartStream(hclient, hrequest);
    } else if (route == "/" DAEMON_RESTART_STREAM) {  // +
      return HandleRequestClientRestartStream(hclient, hrequest);
    } else if (route == "/" DAEMON_STOP_STREAM) {  // +
      return HandleRequestClientStopStream(hclient, hrequest);
    } else if (route == "/" DAEMON_GET_PIPELINE_STREAM) {  // +
      return HandleRequestClientGetPipelineStream(hclient, hrequest);
    } else if (route == "/" DAEMON_GET_LOG_STREAM) {  // +
      return HandleRequestClientGetLogStream(hclient, hrequest);
    } else if (route == "/" DAEMON_GET_CONFIG_JSON_STREAM) {  // +
      return HandleRequestClientGetConfigJsonStream(hclient, hrequest);
    }

  } else if (method == common::http::http_method::HM_GET || method == common::http::http_method::HM_HEAD) {
    if (method == common::http::http_method::HM_GET) {
      if (route == "/" DAEMON_STATS_SERVICE) {  // +
        return HandleRequestClientGetStats(hclient, hrequest);
      } else if (route == "/" DAEMON_BALANCE_SERVICE) {  // +
        return HandleRequestClientGetBalance(hclient, hrequest);
      } else if (route == "/" DAEMON_GET_LOG_SERVICE) {  // +
        return HandleRequestClientGetLogService(hclient, hrequest);
      } else if (route == "/" WS_UPDATES) {
        common::http::header_t head;
        if (!hrequest.FindHeaderByKey("Sec-WebSocket-Key", false, &head)) {
          return common::make_errno_error("not found Sec-WebSocket-Key header", EAGAIN);
        }

        common::http::header_t found_key_in_headers;
        if (!hrequest.FindHeaderByKey(API_KEY_PARAM, false, &found_key_in_headers)) {
          auto query_str = url.query();
          common::uri::Component key, value;
          common::uri::Component query(0, query_str.length());
          common::Optional<std::string> found_key_in_params;
          while (common::uri::ExtractQueryKeyValue(query_str.c_str(), &query, &key, &value)) {
            std::string key_string(query_str.substr(key.begin, key.len));
            std::string param_text(query_str.substr(value.begin, value.len));
            if (common::EqualsASCII(key_string, API_KEY_PARAM, false)) {
              found_key_in_params = param_text;
              break;
            }
          }

          if (!found_key_in_params) {
            return common::make_errno_error("Don't have permissions", EACCES);
          }
          found_key_in_headers.value = *found_key_in_params;
        }

        const auto expire_key =
            common::license::make_license<common::license::expire_key_t>(found_key_in_headers.value);
        if (!expire_key) {
          common::Error err = common::make_error("Invalid expire key");
          return common::make_errno_error(err->GetDescription(), EINVAL);
        }

        common::time64_t tm;
        bool is_valid = common::license::GetExpireTimeFromKey(PROJECT_NAME_LOWERCASE, *expire_key, &tm);
        if (!is_valid) {
          common::Error err = common::make_error("Failed to get expire key time");
          return common::make_errno_error(err->GetDescription(), EINVAL);
        }

        hclient->SetVerified(true, tm);
        if (!hclient->HaveFullAccess()) {
          return common::make_errno_error("Don't have permissions", EACCES);
        }

        common::http::headers_t extra_headers = {{"Access-Control-Allow-Origin", "*"}};
        return hclient->SendSwitchProtocolsResponse(head.value, extra_headers, hclient->GetServerInfo());
      }
    }
  }
  return hclient->UnknownMethodError(method, route);
}

common::ErrnoError ProcessSlaveWrapper::DaemonDataReceived(ProtocoledDaemonClient* dclient) {
  CHECK(loop_->IsLoopThread());
  char buff[BUF_SIZE] = {0};
  size_t nread = 0;
  common::ErrnoError errn = dclient->SingleRead(buff, BUF_SIZE - 1, &nread);
  if (errn || nread == 0) {
    if (nread == 0) {
      return kClosedDaemon;
    }
    return errn;
  }

  return ProcessReceived(dclient, buff, nread);
}

common::ErrnoError ProcessSlaveWrapper::StreamDataReceived(stream_client_t* pipe_client) {
  CHECK(loop_->IsLoopThread());
  std::string input_command;
  common::ErrnoError err = pipe_client->ReadCommand(&input_command);
  if (err) {
    return err;  // i don't want to handle spam, command must be formated according protocol
  }

  fastotv::protocol::request_t* req = nullptr;
  fastotv::protocol::response_t* resp = nullptr;
  common::Error err_parse = common::protocols::json_rpc::ParseJsonRPC(input_command, &req, &resp);
  if (err_parse) {
    const std::string err_str = err_parse->GetDescription();
    return common::make_errno_error(err_str, EAGAIN);
  }

  if (req) {
    DEBUG_LOG() << "Received stream request: " << input_command;
    err = HandleRequestStreamsCommand(pipe_client, req);
    if (err) {
      DEBUG_MSG_ERROR(err, common::logging::LOG_LEVEL_ERR);
    }
    delete req;
    return common::ErrnoError();
  } else if (resp) {
    DEBUG_LOG() << "Received stream responce: " << input_command;
    err = HandleResponceStreamsCommand(pipe_client, resp);
    if (err) {
      DEBUG_MSG_ERROR(err, common::logging::LOG_LEVEL_ERR);
    }
    delete resp;
    return common::ErrnoError();
  }

  WARNING_LOG() << "Received unknown stream message: " << input_command;
  return common::make_errno_error("Invalid command type.", EINVAL);
}

void ProcessSlaveWrapper::DataReceived(common::libev::IoClient* client) {
  if (ProtocoledDaemonClient* dclient = dynamic_cast<ProtocoledDaemonClient*>(client)) {
    auto err = DaemonDataReceived(dclient);
    if (err) {
      ERROR_LOG() << "DataReceived daemon error: " << err->GetDescription();
      ignore_result(dclient->Close());
      delete dclient;
    } else {
      auto step = dclient->Step();
      if (step == common::libev::websocket::ZERO) {
        ignore_result(dclient->Close());
        delete dclient;
      }
    }
  } else if (stream_client_t* pipe_client = dynamic_cast<stream_client_t*>(client)) {
    common::ErrnoError err = StreamDataReceived(pipe_client);
    if (err) {
      DEBUG_MSG_ERROR(err, common::logging::LOG_LEVEL_ERR);
      DaemonServer* server = static_cast<DaemonServer*>(loop_);
      auto childs = server->GetChilds();
      for (auto* child : childs) {
        auto channel = static_cast<ChildStream*>(child);
        if (pipe_client == channel->GetClient()) {
          channel->SetClient(nullptr);
          break;
        }
      }

      ignore_result(pipe_client->Close());
      delete pipe_client;
    }
  } else {
    NOTREACHED();
  }
}

void ProcessSlaveWrapper::DataReadyToWrite(common::libev::IoClient* client) {
  UNUSED(client);
}

void ProcessSlaveWrapper::PostLooped(common::libev::IoLoop* server) {
  if (check_cods_vods_timer_ != INVALID_TIMER_ID) {
    server->RemoveTimer(check_cods_vods_timer_);
    check_cods_vods_timer_ = INVALID_TIMER_ID;
  }

  if (check_old_files_timer_ != INVALID_TIMER_ID) {
    server->RemoveTimer(check_old_files_timer_);
    check_old_files_timer_ = INVALID_TIMER_ID;
  }

  if (ping_client_timer_ != INVALID_TIMER_ID) {
    server->RemoveTimer(ping_client_timer_);
    ping_client_timer_ = INVALID_TIMER_ID;
  }

  if (node_stats_timer_ != INVALID_TIMER_ID) {
    server->RemoveTimer(node_stats_timer_);
    node_stats_timer_ = INVALID_TIMER_ID;
  }

  if (quit_cleanup_timer_ != INVALID_TIMER_ID) {
    server->RemoveTimer(quit_cleanup_timer_);
    quit_cleanup_timer_ = INVALID_TIMER_ID;
  }

  if (check_license_timer_ != INVALID_TIMER_ID) {
    server->RemoveTimer(check_license_timer_);
    check_license_timer_ = INVALID_TIMER_ID;
  }
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientRestartService(ProtocoledDaemonClient* dclient,
                                                                          const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  const auto info = dclient->GetInfo();
  if (!common::net::IsLocalHost(info.host())) {
    common::Error err(common::MemSPrintf("Skipped restart request from: %s", info.host()));
    ignore_result(dclient->RestartFail(common::http::HS_FORBIDDEN, err));
    return common::make_errno_error(err->GetDescription(), EINVAL);
  };

  common::daemon::commands::RestartInfo restart_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &restart_info, false);
  if (errn) {
    ignore_result(dclient->RestartFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  if (quit_cleanup_timer_ != INVALID_TIMER_ID) {
    // in progress
    common::Error err = common::make_error("Restart service in progress...");
    ignore_result(dclient->RestartFail(common::http::HS_INTERNAL_ERROR, err));
    return common::make_errno_error(err->GetDescription(), EAGAIN);
  }

  StopImpl();
  return dclient->RestartSuccess();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientStopService(ProtocoledDaemonClient* dclient,
                                                                       const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  const auto info = dclient->GetInfo();
  if (!common::net::IsLocalHost(info.host())) {
    common::Error err(common::MemSPrintf("Skipped restart request from: %s", info.host()));
    ignore_result(dclient->StopFail(common::http::HS_FORBIDDEN, err));
    return common::make_errno_error(err->GetDescription(), EINVAL);
  }

  common::daemon::commands::StopInfo stop_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &stop_info, false);
  if (errn) {
    ignore_result(dclient->StopFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  if (quit_cleanup_timer_ != INVALID_TIMER_ID) {
    // in progress
    common::Error err = common::make_error("Stop service in progress...");
    ignore_result(dclient->StopFail(common::http::HS_INTERNAL_ERROR, err));
    return common::make_errno_error(err->GetDescription(), EAGAIN);
  }

  StopImpl();
  stoped_ = true;
  return dclient->StopSuccess();
}

common::ErrnoError ProcessSlaveWrapper::CreateChildStream(const serialized_stream_t& config_args) {
  CHECK(loop_->IsLoopThread());

  common::ErrnoError err = options::ValidateConfig(config_args);
  if (err) {
    return err;
  }

  StreamInfo sha;
  std::string feedback_dir;
  std::string data_dir;
  common::logging::LOG_LEVEL logs_level;
  err = MakeStreamInfo(config_args, true, &sha, &feedback_dir, &data_dir, &logs_level);
  if (err) {
    return err;
  }

  Child* stream = FindChildByID(sha.id);
  if (stream) {
    return common::make_errno_error(common::MemSPrintf("Stream with id: %s exist, skip request.", sha.id), EEXIST);
  }

  config_args->Insert(PYFASTOSTREAM_PATH_FIELD,
                      common::Value::CreateStringValueFromBasicString(config_.pyfastostream_path));
  return CreateChildStreamImpl(config_args, sha);
}

common::ErrnoError ProcessSlaveWrapper::StopChildStreamImpl(fastotv::stream_id_t sid, bool force) {
  CHECK(loop_->IsLoopThread());

  auto stream = FindChildByID(sid);
  if (!stream) {
    return common::make_errno_error(common::MemSPrintf("Stream with id: %s does not exist, skip request.", sid),
                                    EINVAL);
  }

  if (force) {
    return stream->Terminate();
  }

  return stream->Stop();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestChangedSourcesStream(stream_client_t* pclient,
                                                                          const fastotv::protocol::request_t* req) {
  CHECK(loop_->IsLoopThread());

  ChangedSouresInfo ch_sources_info;
  common::ErrnoError errn = ParseStreamRequest(pclient, req, &ch_sources_info);
  if (errn) {
    return errn;
  }

  common::json::WsDataJson breq;
  common::Error err_ser = ChangedSourcesStreamBroadcast(ch_sources_info, &breq);
  if (err_ser) {
    const std::string err_str = err_ser->GetDescription();
    return common::make_errno_error(err_str, EAGAIN);
  }

  BroadcastClients(breq);
  return common::ErrnoError();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestStatisticStream(stream_client_t* pclient,
                                                                     const fastotv::protocol::request_t* req) {
  CHECK(loop_->IsLoopThread());

  StatisticInfo stat;
  common::ErrnoError errn = ParseStreamRequest(pclient, req, &stat);
  if (errn) {
    return errn;
  }

  common::json::WsDataJson breq;
  common::Error err_ser = StatisitcStreamBroadcast(stat, &breq);
  if (err_ser) {
    const std::string err_str = err_ser->GetDescription();
    return common::make_errno_error(err_str, EAGAIN);
  }

  BroadcastClients(breq);
  return common::ErrnoError();
}

#if defined(MACHINE_LEARNING)
common::ErrnoError ProcessSlaveWrapper::HandleRequestMlNotificationStream(stream_client_t* pclient,
                                                                          const fastotv::protocol::request_t* req) {
  UNUSED(pclient);
  CHECK(loop_->IsLoopThread());
  if (req->params) {
    const char* params_ptr = req->params->c_str();
    json_object* jrequest_notif = json_tokener_parse(params_ptr);
    if (!jrequest_notif) {
      return common::make_errno_error_inval();
    }

    fastotv::commands_info::ml::NotificationInfo notif;
    common::Error err_des = notif.DeSerialize(jrequest_notif);
    json_object_put(jrequest_notif);
    if (err_des) {
      const std::string err_str = err_des->GetDescription();
      return common::make_errno_error(err_str, EAGAIN);
    }

    fastotv::protocol::request_t req;
    common::Error err_ser = MlNotificationStreamBroadcast(notif, &req);
    if (err_ser) {
      const std::string err_str = err_ser->GetDescription();
      return common::make_errno_error(err_str, EAGAIN);
    }

    BroadcastClients(req);
    return common::ErrnoError();
  }

  return common::make_errno_error_inval();
}
#endif

common::ErrnoError ProcessSlaveWrapper::RestartChildStreamImpl(fastotv::stream_id_t sid) {
  CHECK(loop_->IsLoopThread());

  auto stream = FindChildByID(sid);
  if (!stream) {
    return common::make_errno_error(common::MemSPrintf("Stream with id: %s does not exist, skip request.", sid),
                                    EINVAL);
  }

  return stream->Restart();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientRestartStream(ProtocoledDaemonClient* dclient,
                                                                         const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  stream::RestartInfo restart_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &restart_info);
  if (errn) {
    ignore_result(dclient->ReStartStreamFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  errn = RestartChildStreamImpl(restart_info.GetStreamID());
  if (errn) {
    ignore_result(dclient->ReStartStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->ReStartStreamSuccess();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientGetLogService(ProtocoledDaemonClient* dclient,
                                                                         const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  common::ErrnoError errn = ParseGetClientRequest(dclient, req);
  if (errn) {
    ignore_result(dclient->GetLogServiceFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->GetLogServiceSuccess(config_.log_path);
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientGetLogStream(ProtocoledDaemonClient* dclient,
                                                                        const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  stream::GetLogInfo log_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &log_info);
  if (errn) {
    ignore_result(dclient->GetLogStreamFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  const auto stream_log_file = MakeStreamLogPath(log_info.GetFeedbackDir());
  if (!stream_log_file) {
    common::ErrnoError errn = common::make_errno_error("Can't generate log stream path", EAGAIN);
    ignore_result(dclient->GetLogStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->GetLogStreamSuccess(stream_log_file->GetPath());
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientGetPipelineStream(ProtocoledDaemonClient* dclient,
                                                                             const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());
  stream::GetPipelineInfo pipeline_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &pipeline_info);
  if (errn) {
    ignore_result(dclient->GetPipeStreamFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  const auto pipe_file = MakeStreamPipelinePath(pipeline_info.GetFeedbackDir());
  if (!pipe_file) {
    common::ErrnoError errn = common::make_errno_error("Can't generate pipeline stream path", EAGAIN);
    ignore_result(dclient->GetPipeStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->GetPipeStreamSuccess(pipe_file->GetPath());
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientGetConfigJsonStream(ProtocoledDaemonClient* dclient,
                                                                               const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());
  stream::GetConfigJsonInfo config_json;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &config_json);
  if (errn) {
    ignore_result(dclient->GetConfigStreamFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  const auto pipe_file = MakeStreamConfigJsonPath(config_json.GetFeedbackDir());
  if (!pipe_file) {
    common::ErrnoError errn = common::make_errno_error("Can't generate config.json stream path", EAGAIN);
    ignore_result(dclient->GetConfigStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->GetConfigStreamSuccess(pipe_file->GetPath());
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientStopStream(ProtocoledDaemonClient* dclient,
                                                                      const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  stream::StopInfo stop_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &stop_info);
  if (errn) {
    ignore_result(dclient->StopStreamFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  errn = StopChildStreamImpl(stop_info.GetStreamID(), stop_info.GetForce());
  if (errn) {
    ignore_result(dclient->StopStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->StopStreamSuccess();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientStartStream(ProtocoledDaemonClient* dclient,
                                                                       const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  stream::StartInfo start_info;
  common::ErrnoError errn = ParsePostClientRequest(dclient, req, &start_info);
  if (errn) {
    ignore_result(dclient->StartStreamFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  errn = CreateChildStream(start_info.GetConfig());
  if (errn) {
    ignore_result(dclient->StartStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->StartStreamSuccess();
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientGetStats(ProtocoledDaemonClient* dclient,
                                                                    const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  common::ErrnoError errn = ParseGetClientRequest(dclient, req);
  if (errn) {
    ignore_result(dclient->GetStatsFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  return dclient->GetStatsSuccess(MakeServiceStats(dclient->GetExpTime()));
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestClientGetBalance(ProtocoledDaemonClient* dclient,
                                                                      const common::http::HttpRequest& req) {
  CHECK(loop_->IsLoopThread());

  common::ErrnoError errn = ParseGetClientRequest(dclient, req);
  if (errn) {
    ignore_result(dclient->GetBalanceFail(common::http::HS_BAD_REQUEST, common::make_error_from_errno(errn)));
    return errn;
  }

  auto cost_per_second = PricePerSecond();
  return dclient->GetBalanceSuccess(service::BalanceInfo(node_stats_->balance, cost_per_second));
}

common::ErrnoError ProcessSlaveWrapper::HandleRequestStreamsCommand(stream_client_t* pclient,
                                                                    const fastotv::protocol::request_t* req) {
  if (req->method == BROADCAST_CHANGED_SOURCES_STREAM) {
    return HandleRequestChangedSourcesStream(pclient, req);
  } else if (req->method == BROADCAST_STATISTIC_STREAM) {
    return HandleRequestStatisticStream(pclient, req);
  }
#if defined(MACHINE_LEARNING)
  else if (req->method == ML_NOTIFICATION_STREAM) {
    return HandleRequestMlNotificationStream(pclient, req);
  }
#endif

  WARNING_LOG() << "Received unknown command: " << req->method;
  return common::ErrnoError();
}

common::ErrnoError ProcessSlaveWrapper::HandleResponceStreamsCommand(stream_client_t* pclient,
                                                                     const fastotv::protocol::response_t* resp) {
  fastotv::protocol::request_t req;
  const auto sid = resp->id;
  if (pclient->PopRequestByID(sid, &req)) {
    if (req.method == REQUEST_STOP_STREAM) {
    } else if (req.method == REQUEST_RESTART_STREAM) {
    } else {
      WARNING_LOG() << "HandleResponceStreamsCommand not handled responce id: " << *sid << ", command: " << req.method;
    }
  } else {
    WARNING_LOG() << "HandleResponceStreamsCommand not found responce id: " << *sid;
  }
  return common::ErrnoError();
}

void ProcessSlaveWrapper::CheckLicenseExpired(common::libev::IoLoop* server) {
  std::vector<common::libev::IoClient*> online_clients = server->GetClients();
  for (size_t i = 0; i < online_clients.size(); ++i) {
    common::libev::IoClient* client = online_clients[i];
    ProtocoledDaemonClient* dclient = dynamic_cast<ProtocoledDaemonClient*>(client);
    if (dclient && dclient->IsVerified()) {
      if (dclient->IsExpired()) {
        NOTICE_LOG() << "Expired key for client: " << dclient->GetFormatedName();
        ignore_result(dclient->Close());
        delete dclient;
      }
    }
  }
}

service::FullServiceInfo ProcessSlaveWrapper::MakeServiceStats(common::time64_t expiration_time) const {
  common::uri::Replacements<char> replacements;
  replacements.SetHost(config_.alias.data(), common::uri::Component(0, static_cast<int>(config_.alias.length())));
  auto stabled_hls_host = config_.hls_host.ReplaceComponents(replacements);
  auto stabled_vods_host = config_.vods_host.ReplaceComponents(replacements);
  auto stabled_cods_host = config_.cods_host.ReplaceComponents(replacements);
  service::FullServiceInfo fstat(stabled_hls_host, stabled_vods_host, stabled_cods_host, expiration_time,
                                 config_.hls_dir, config_.vods_dir, config_.cods_dir, config_.timeshifts_dir,
                                 config_.feedback_dir, config_.proxy_dir, config_.data_dir, MakeServiceInfoStats());

  return fstat;
}

service::ServerInfo ProcessSlaveWrapper::MakeServiceInfoStats() const {
  service::CpuShot next = service::GetMachineCpuShot();
  double cpu_load = service::GetCpuMachineLoad(node_stats_->prev, next);
  node_stats_->prev = next;

  service::NetShot next_nshot = service::GetMachineNetShot();
  uint64_t bytes_recv = (next_nshot.bytes_recv - node_stats_->prev_nshot.bytes_recv);
  uint64_t bytes_send = (next_nshot.bytes_send - node_stats_->prev_nshot.bytes_send);
  node_stats_->prev_nshot = next_nshot;

  service::MemoryShot mem_shot = service::GetMachineMemoryShot();
  service::HddShot hdd_shot = service::GetMachineHddShot();
  service::SysinfoShot sshot = service::GetMachineSysinfoShot();
  std::string uptime_str = common::MemSPrintf("%.2f %.2f %.2f", sshot.loads[0], sshot.loads[1], sshot.loads[2]);
  fastotv::timestamp_t current_time = common::time::current_utc_mstime();
  fastotv::timestamp_t ts_diff = (current_time - node_stats_->timestamp) / 1000;
  if (ts_diff == 0) {
    ts_diff = 1;  // divide by zero
  }
  node_stats_->timestamp = current_time;

  size_t daemons_client_count = 0;
  std::vector<common::libev::IoClient*> clients = loop_->GetClients();
  for (size_t i = 0; i < clients.size(); ++i) {
    ProtocoledDaemonClient* dclient = dynamic_cast<ProtocoledDaemonClient*>(clients[i]);
    if (dclient && dclient->IsVerified()) {
      daemons_client_count++;
    }
  }
  service::OnlineUsers online(daemons_client_count, static_cast<HttpHandler*>(http_handler_)->GetOnlineClients(),
                              static_cast<HttpHandler*>(vods_handler_)->GetOnlineClients(),
                              static_cast<HttpHandler*>(cods_handler_)->GetOnlineClients());

  auto cost_per_second = PricePerSecond();
  node_stats_->balance += cost_per_second * ts_diff;
  service::ConnectionsStats http(online_clients_, connections_count_, requests_count_);
  service::ServerInfo stat(cpu_load, node_stats_->CalcGPULoad(), uptime_str, mem_shot.ram_bytes_total,
                           mem_shot.ram_bytes_free, hdd_shot.hdd_bytes_total, hdd_shot.hdd_bytes_free,
                           bytes_recv / ts_diff, bytes_send / ts_diff, sshot.uptime, current_time, online, http,
                           next_nshot.bytes_recv, next_nshot.bytes_send, cost_per_second);

  return stat;
}

double ProcessSlaveWrapper::PricePerSecond() const {
  const auto price = kServicePriceInUSDPerSec;
  const auto add = 0;
  const auto hw = 0;
  const auto cost_per_second = price + add + hw;
  return cost_per_second;
}

}  // namespace server
}  // namespace fastocloud
