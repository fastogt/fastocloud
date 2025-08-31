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

#pragma once

#include <common/daemon/commands/hhash_info.h>
#include <common/json/json.h>
#include <common/libev/websocket/websocket_client.h>
#include <common/license/types.h>
#include <fastotv/protocol/protocol.h>
#include <fastotv/protocol/types.h>

#include <string>

#include "server/daemon/commands_info/service/server_info.h"

namespace fastocloud {
namespace server {

class ProtocoledDaemonClient : public common::libev::websocket::WebSocketServerClient {
 public:
  typedef common::libev::websocket::WebSocketServerClient base_class;
  ProtocoledDaemonClient(common::libev::IoLoop* server, const common::net::socket_info& info);

  bool IsVerified() const;
  void SetVerified(bool verified, common::time64_t exp_time);

  common::time64_t GetExpTime() const;
  bool IsExpired() const;
  bool HaveFullAccess() const;

  common::http::http_protocol GetProtocol() const;
  common::libev::http::HttpServerInfo GetServerInfo() const;

  common::ErrnoError StopMe(const common::uri::GURL& url) WARN_UNUSED_RESULT;
  common::ErrnoError RestartMe(const common::uri::GURL& url) WARN_UNUSED_RESULT;

  common::ErrnoError StopFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError StopSuccess() WARN_UNUSED_RESULT;

  common::ErrnoError RestartFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError RestartSuccess() WARN_UNUSED_RESULT;

  common::ErrnoError GetHardwareHashFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetHardwareHashSuccess(const common::daemon::commands::HardwareHashProject& params)
      WARN_UNUSED_RESULT;

  common::ErrnoError GetStatsFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetStatsSuccess(const service::FullServiceInfo& stats) WARN_UNUSED_RESULT;

  common::ErrnoError GetBalanceFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetBalanceSuccess(const service::BalanceInfo& balance) WARN_UNUSED_RESULT;

  common::ErrnoError GetLogServiceFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetLogServiceSuccess(const std::string& path) WARN_UNUSED_RESULT;

  common::ErrnoError GetLogStreamFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetLogStreamSuccess(const std::string& path) WARN_UNUSED_RESULT;

  common::ErrnoError GetPipeStreamFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetPipeStreamSuccess(const std::string& path) WARN_UNUSED_RESULT;

  common::ErrnoError StartStreamFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError StartStreamSuccess() WARN_UNUSED_RESULT;

  common::ErrnoError ReStartStreamFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError ReStartStreamSuccess() WARN_UNUSED_RESULT;

  common::ErrnoError StopStreamFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError StopStreamSuccess() WARN_UNUSED_RESULT;

  common::ErrnoError GetConfigStreamFail(common::http::http_status code, common::Error err) WARN_UNUSED_RESULT;
  common::ErrnoError GetConfigStreamSuccess(const std::string& path) WARN_UNUSED_RESULT;

  common::ErrnoError UnknownMethodError(common::http::http_method method, const std::string& route) WARN_UNUSED_RESULT;

  common::ErrnoError Broadcast(const common::json::WsDataJson& request) WARN_UNUSED_RESULT;

 private:
  common::ErrnoError SendDataJson(common::http::http_status code, const common::json::DataJson& data);
  common::ErrnoError SendErrorJson(common::http::http_status code, common::Error err);
  common::ErrnoError SendHeadersInternal(common::http::http_status code,
                                         const char* mime_type,
                                         size_t* length,
                                         time_t* mod);

  bool is_verified_;
  common::time64_t exp_time_;
};

}  // namespace server
}  // namespace fastocloud
