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

#include "server/daemon/commands.h"

// broadcast
#define SERVICE_STATISTIC_SERVICE "statistic_service"

namespace fastocloud {
namespace server {

common::Error ChangedSourcesStreamBroadcast(const ChangedSouresInfo& params, common::json::WsDataJson* req) {
  if (!req) {
    return common::make_error_inval();
  }

  json_object* result_json = nullptr;
  common::Error err_ser = params.Serialize(&result_json);
  if (err_ser) {
    return err_ser;
  }

  *req = common::json::WsDataJson(BROADCAST_CHANGED_SOURCES_STREAM, result_json);
  return common::Error();
}

common::Error StatisitcStreamBroadcast(const StatisticInfo& params, common::json::WsDataJson* req) {
  if (!req) {
    return common::make_error_inval();
  }

  json_object* result_json = nullptr;
  common::Error err_ser = params.Serialize(&result_json);
  if (err_ser) {
    return err_ser;
  }

  *req = common::json::WsDataJson(BROADCAST_STATISTIC_STREAM, result_json);
  return common::Error();
}

#if defined(MACHINE_LEARNING)
common::Error MlNotificationStreamBroadcast(const fastotv::commands_info::ml::NotificationInfo& params,
                                            fastotv::protocol::request_t* req) {
  if (!req) {
    return common::make_error_inval();
  }

  std::string stat_json;
  common::Error err_ser = params.SerializeToString(&stat_json);
  if (err_ser) {
    return err_ser;
  }

  *req = fastotv::protocol::request_t::MakeNotification(STREAM_ML_NOTIFICATION_STREAM, stat_json);
  return common::Error();
}
#endif

common::Error StatisitcServiceBroadcast(const service::ServerInfo& params, common::json::WsDataJson* req) {
  if (!req) {
    return common::make_error_inval();
  }

  json_object* stat_json = nullptr;
  common::Error err_ser = params.Serialize(&stat_json);
  if (err_ser) {
    return err_ser;
  }

  *req = common::json::WsDataJson(SERVICE_STATISTIC_SERVICE, stat_json);
  return common::Error();
}

common::Error QuitStatusStreamBroadcast(const stream::QuitStatusInfo& params, common::json::WsDataJson* req) {
  if (!req) {
    return common::make_error_inval();
  }

  json_object* quit_json = nullptr;
  common::Error err_ser = params.Serialize(&quit_json);
  if (err_ser) {
    return err_ser;
  }

  *req = common::json::WsDataJson(STREAM_QUIT_STATUS_STREAM, quit_json);
  return common::Error();
}

}  // namespace server
}  // namespace fastocloud
