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

#include "stream_commands/commands_factory.h"

#include "stream_commands/commands.h"

namespace fastocloud {

common::Error RestartStreamResponseSuccess(fastotv::protocol::sequance_id_t id, fastotv::protocol::response_t* resp) {
  if (!resp) {
    return common::make_error_inval();
  }

  *resp =
      fastotv::protocol::response_t::MakeMessage(id, common::protocols::json_rpc::JsonRPCMessage::MakeSuccessMessage());
  return common::Error();
}

common::Error StopStreamResponseSuccess(fastotv::protocol::sequance_id_t id, fastotv::protocol::response_t* resp) {
  if (!resp) {
    return common::make_error_inval();
  }

  *resp =
      fastotv::protocol::response_t::MakeMessage(id, common::protocols::json_rpc::JsonRPCMessage::MakeSuccessMessage());
  return common::Error();
}

common::Error RestartStreamRequest(fastotv::protocol::sequance_id_t id, fastotv::protocol::request_t* req) {
  if (!req) {
    return common::make_error_inval();
  }

  fastotv::protocol::request_t lreq;
  lreq.id = id;
  lreq.method = REQUEST_RESTART_STREAM;
  *req = lreq;
  return common::Error();
}

common::Error StopStreamRequest(fastotv::protocol::sequance_id_t id, fastotv::protocol::request_t* req) {
  if (!req) {
    return common::make_error_inval();
  }

  fastotv::protocol::request_t lreq;
  lreq.id = id;
  lreq.method = REQUEST_STOP_STREAM;
  *req = lreq;
  return common::Error();
}

}  // namespace fastocloud
