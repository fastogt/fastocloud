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

#include <fastotv/protocol/types.h>

#include <string>

#include <common/json/json.h>

#if defined(MACHINE_LEARNING)
#include <fastotv/commands_info/ml/notification_info.h>
#endif

#include "server/daemon/commands_info/service/server_info.h"
#include "server/daemon/commands_info/stream/quit_status_info.h"

#include "stream_commands/commands_info/changed_sources_info.h"
#include "stream_commands/commands_info/statistic_info.h"

// daemon
// client commands

#define DAEMON_START_STREAM "start_stream"  // {"config": {...}, "command_line": {...} }
#define DAEMON_STOP_STREAM "stop_stream"
#define DAEMON_RESTART_STREAM "restart_stream"
#define DAEMON_GET_LOG_STREAM "get_log_stream"
#define DAEMON_GET_PIPELINE_STREAM "get_pipeline_stream"
#define DAEMON_GET_CONFIG_JSON_STREAM "get_config_json_stream"

#define DAEMON_STOP_SERVICE "stop_service"        // {"delay": 0 }
#define DAEMON_RESTART_SERVICE "restart_service"  // {"delay": 0 }
#define DAEMON_STATS_SERVICE "stats"

#define DAEMON_GET_LOG_SERVICE "get_log_service"  // {"path":"http://localhost/service/id"}

// Broadcast
#define BROADCAST_CHANGED_SOURCES_STREAM "changed_source_stream"
#define BROADCAST_STATISTIC_STREAM "statistic_stream"
#define STREAM_QUIT_STATUS_STREAM "quit_status_stream"
#if defined(MACHINE_LEARNING)
#define STREAM_ML_NOTIFICATION_STREAM "ml_notification_stream"
#endif
#define STREAM_STATISTIC_SERVICE "statistic_service"

namespace fastocloud {
namespace server {

// Broadcast
common::Error ChangedSourcesStreamBroadcast(const ChangedSouresInfo& params,
                                            common::json::WsDataJson* req) WARN_UNUSED_RESULT;
common::Error StatisitcStreamBroadcast(const StatisticInfo& params, common::json::WsDataJson* req) WARN_UNUSED_RESULT;
#if defined(MACHINE_LEARNING)
common::Error MlNotificationStreamBroadcast(const fastotv::commands_info::ml::NotificationInfo& params,
                                            fastotv::protocol::request_t* req);
#endif
common::Error StatisitcServiceBroadcast(const service::ServerInfo& params,
                                        common::json::WsDataJson* req) WARN_UNUSED_RESULT;

common::Error QuitStatusStreamBroadcast(const stream::QuitStatusInfo& params, common::json::WsDataJson* req) WARN_UNUSED_RESULT;

}  // namespace server
}  // namespace fastocloud
