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

#include "server/daemon/client.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <common/json/json.h>

#include <common/daemon/commands/restart_info.h>
#include <common/daemon/commands/stop_info.h>

namespace {
const std::string kErrorInvalid = "{\"error\":{\"code\":-1, \"message\":\"NOTREACHED\"}}";
const auto kDataOK = common::json::MakeSuccessDataJson();
}  // namespace

namespace fastocloud {
namespace server {

ProtocoledDaemonClient::ProtocoledDaemonClient(common::libev::IoLoop* server, const common::net::socket_info& info)
    : base_class(server, info), is_verified_(false), exp_time_(0) {}

bool ProtocoledDaemonClient::IsVerified() const {
  return is_verified_;
}

common::time64_t ProtocoledDaemonClient::GetExpTime() const {
  return exp_time_;
}

void ProtocoledDaemonClient::SetVerified(bool verified, common::time64_t exp_time) {
  is_verified_ = verified;
  exp_time_ = exp_time;
}

bool ProtocoledDaemonClient::IsExpired() const {
  return exp_time_ < common::time::current_utc_mstime();
}

bool ProtocoledDaemonClient::HaveFullAccess() const {
  return IsVerified() && !IsExpired();
}

common::http::http_protocol ProtocoledDaemonClient::GetProtocol() const {
  return common::http::HP_1_0;
}

common::libev::http::HttpServerInfo ProtocoledDaemonClient::GetServerInfo() const {
  return common::libev::http::HttpServerInfo();
}

common::ErrnoError ProtocoledDaemonClient::StopMe(const common::uri::GURL& url) {
  const common::daemon::commands::StopInfo params;
  std::string result;
  common::Error err_ser = params.SerializeToString(&result);
  if (err_ser) {
    return common::make_errno_error(err_ser->GetDescription(), EAGAIN);
  }

  return PostJson(url, result.c_str(), result.size(), false);
}

common::ErrnoError ProtocoledDaemonClient::Broadcast(const common::json::WsDataJson& request) {
  std::string result;
  ignore_result(request.SerializeToString(&result));
  return SendFrame(result.c_str(), result.size());
}

common::ErrnoError ProtocoledDaemonClient::RestartMe(const common::uri::GURL& url) {
  const common::daemon::commands::RestartInfo params;
  std::string result;
  common::Error err_ser = params.SerializeToString(&result);
  if (err_ser) {
    return common::make_errno_error(err_ser->GetDescription(), EAGAIN);
  }
  return PostJson(url, result.c_str(), result.size(), false);
}

common::ErrnoError ProtocoledDaemonClient::StopFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::StopSuccess() {
  return SendDataJson(common::http::HS_OK, kDataOK);
}

common::ErrnoError ProtocoledDaemonClient::RestartFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::RestartSuccess() {
  return SendDataJson(common::http::HS_OK, kDataOK);
}

common::ErrnoError ProtocoledDaemonClient::GetHardwareHashFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::GetHardwareHashSuccess(
    const common::daemon::commands::HardwareHashInfo& params) {
  json_object* jrequest_init = nullptr;
  common::Error err_ser = params.Serialize(&jrequest_init);
  if (err_ser) {
    return GetHardwareHashFail(common::http::HS_INTERNAL_ERROR, err_ser);
  }

  std::string result;
  common::json::DataJson js = common::json::MakeSuccessDataJson(jrequest_init);  // take ownerships
  err_ser = js.SerializeToString(&result);
  if (err_ser) {
    return GetHardwareHashFail(common::http::HS_INTERNAL_ERROR, err_ser);
  }

  return SendDataJson(common::http::HS_OK, js);
}

common::ErrnoError ProtocoledDaemonClient::GetStatsFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::GetStatsSuccess(const service::FullServiceInfo& stats) {
  json_object* jrequest_init = nullptr;
  common::Error err_ser = stats.Serialize(&jrequest_init);
  if (err_ser) {
    return GetHardwareHashFail(common::http::HS_INTERNAL_ERROR, err_ser);
  }

  std::string result;
  common::json::DataJson js = common::json::MakeSuccessDataJson(jrequest_init);  // take ownerships
  err_ser = js.SerializeToString(&result);
  if (err_ser) {
    return GetStatsFail(common::http::HS_INTERNAL_ERROR, err_ser);
  }

  return SendDataJson(common::http::HS_OK, js);
}

common::ErrnoError ProtocoledDaemonClient::GetLogServiceFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::GetLogServiceSuccess(const std::string& path) {
  const char* file_path_str_ptr = path.c_str();
  struct stat sb;
  if (stat(file_path_str_ptr, &sb) < 0) {
    return GetLogServiceFail(common::http::HS_FORBIDDEN, common::make_error("File not found."));
  }

  if (S_ISDIR(sb.st_mode)) {
    return GetLogServiceFail(common::http::HS_FORBIDDEN, common::make_error("Bad filename."));
  }

  int file = open(file_path_str_ptr, O_RDONLY);
  if (file == INVALID_DESCRIPTOR) { /* open the file for reading */
    return GetLogServiceFail(common::http::HS_FORBIDDEN, common::make_error("File is protected."));
  }

  size_t cast = sb.st_size;
  common::ErrnoError err = SendHeadersInternal(common::http::HS_OK, "text/plain", &cast, &sb.st_mtime);
  if (err) {
    ::close(file);
    return GetLogServiceFail(common::http::HS_INTERNAL_ERROR, common::make_error(err->GetDescription()));
  }

  err = SendFileByFd(file, sb.st_size);
  ::close(file);
  return err;
}

common::ErrnoError ProtocoledDaemonClient::GetLogStreamFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::GetLogStreamSuccess(const std::string& path) {
  const char* file_path_str_ptr = path.c_str();
  struct stat sb;
  if (stat(file_path_str_ptr, &sb) < 0) {
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("File not found."));
  }

  if (S_ISDIR(sb.st_mode)) {
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("Bad filename."));
  }

  int file = open(file_path_str_ptr, O_RDONLY);
  if (file == INVALID_DESCRIPTOR) { /* open the file for reading */
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("File is protected."));
  }

  size_t cast = sb.st_size;
  common::ErrnoError err = SendHeadersInternal(common::http::HS_OK, "text/plain", &cast, &sb.st_mtime);
  if (err) {
    ::close(file);
    return GetLogStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error(err->GetDescription()));
  }

  err = SendFileByFd(file, sb.st_size);
  ::close(file);
  return err;
}

common::ErrnoError ProtocoledDaemonClient::GetPipeStreamFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::GetPipeStreamSuccess(const std::string& path) {
  const char* file_path_str_ptr = path.c_str();
  struct stat sb;
  if (stat(file_path_str_ptr, &sb) < 0) {
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("File not found."));
  }

  if (S_ISDIR(sb.st_mode)) {
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("Bad filename."));
  }

  int file = open(file_path_str_ptr, O_RDONLY);
  if (file == INVALID_DESCRIPTOR) { /* open the file for reading */
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("File is protected."));
  }

  size_t cast = sb.st_size;
  common::ErrnoError err = SendHeadersInternal(common::http::HS_OK, "text/plain", &cast, &sb.st_mtime);
  if (err) {
    ::close(file);
    return GetLogStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error(err->GetDescription()));
  }

  err = SendFileByFd(file, sb.st_size);
  ::close(file);
  return err;
}

common::ErrnoError ProtocoledDaemonClient::GetConfigStreamFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::GetConfigStreamSuccess(const std::string& path) {
  const char* file_path_str_ptr = path.c_str();
  struct stat sb;
  if (stat(file_path_str_ptr, &sb) < 0) {
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("File not found."));
  }

  if (S_ISDIR(sb.st_mode)) {
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("Bad filename."));
  }

  int file = open(file_path_str_ptr, O_RDONLY);
  if (file == INVALID_DESCRIPTOR) { /* open the file for reading */
    return GetLogStreamFail(common::http::HS_FORBIDDEN, common::make_error("File is protected."));
  }

  size_t cast = sb.st_size;
  common::ErrnoError err =
      SendHeadersInternal(common::http::HS_OK, "text/plain", &cast, &sb.st_mtime);  // "application/json"
  if (err) {
    ::close(file);
    return GetLogStreamFail(common::http::HS_INTERNAL_ERROR, common::make_error(err->GetDescription()));
  }

  err = SendFileByFd(file, sb.st_size);
  ::close(file);
  return err;
}

common::ErrnoError ProtocoledDaemonClient::StartStreamFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::StartStreamSuccess() {
  return SendDataJson(common::http::HS_OK, kDataOK);
}

common::ErrnoError ProtocoledDaemonClient::ReStartStreamFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::ReStartStreamSuccess() {
  return SendDataJson(common::http::HS_OK, kDataOK);
}

common::ErrnoError ProtocoledDaemonClient::StopStreamFail(common::http::http_status code, common::Error err) {
  return SendErrorJson(code, err);
}

common::ErrnoError ProtocoledDaemonClient::StopStreamSuccess() {
  return SendDataJson(common::http::HS_OK, kDataOK);
}

common::ErrnoError ProtocoledDaemonClient::UnknownMethodError(common::http::http_method method,
                                                              const std::string& route) {
  common::Error err =
      common::make_error(common::MemSPrintf("not handled %s for route: %s", common::ConvertToString(method), route));
  return SendErrorJson(common::http::HS_NOT_FOUND, err);
}

common::ErrnoError ProtocoledDaemonClient::SendHeadersInternal(common::http::http_status code,
                                                               const char* mime_type,
                                                               size_t* length,
                                                               time_t* mod) {
  return SendHeaders(GetProtocol(), code, {}, mime_type, length, mod, false, GetServerInfo());
}

common::ErrnoError ProtocoledDaemonClient::SendErrorJson(common::http::http_status code, common::Error err) {
  auto errj = common::json::ErrorJson(common::json::MakeErrorJsonMessage(code, err));
  std::string result = kErrorInvalid;
  ignore_result(errj.SerializeToString(&result));
  return SendJson(GetProtocol(), code, {}, result.c_str(), result.size(), false, GetServerInfo());
}

common::ErrnoError ProtocoledDaemonClient::SendDataJson(common::http::http_status code,
                                                        const common::json::DataJson& data) {
  std::string result;
  common::Error err_ser = data.SerializeToString(&result);
  if (err_ser) {
    return SendErrorJson(common::http::HS_INTERNAL_ERROR, err_ser);
  }

  return SendJson(GetProtocol(), code, {}, result.c_str(), result.size(), false, GetServerInfo());
}

}  // namespace server
}  // namespace fastocloud
