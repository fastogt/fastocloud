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

namespace fastocloud {

common::Error RestartStreamRequest(fastotv::protocol::sequance_id_t id,
                                   fastotv::protocol::request_t* req) WARN_UNUSED_RESULT;
common::Error StopStreamRequest(fastotv::protocol::sequance_id_t id,
                                fastotv::protocol::request_t* req) WARN_UNUSED_RESULT;

common::Error RestartStreamResponseSuccess(fastotv::protocol::sequance_id_t id,
                                           fastotv::protocol::response_t* resp) WARN_UNUSED_RESULT;
common::Error StopStreamResponseSuccess(fastotv::protocol::sequance_id_t id,
                                        fastotv::protocol::response_t* resp) WARN_UNUSED_RESULT;

}  // namespace fastocloud
