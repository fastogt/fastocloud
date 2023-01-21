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
    along with fastocloud. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>
#include <string>

#include "base/stream_info.h"

#include <fastotv/types/stream_ttl.h>

namespace fastocloud {

typedef std::shared_ptr<common::HashValue> StreamConfig;

common::Optional<fastotv::stream_id_t> GetSid(const StreamConfig& config) WARN_UNUSED_RESULT;
common::Optional<fastotv::StreamTTL> GetTTL(const StreamConfig& config) WARN_UNUSED_RESULT;
common::Optional<input_t> ReadInput(const StreamConfig& config) WARN_UNUSED_RESULT;
common::Optional<output_t> ReadOutput(const StreamConfig& config) WARN_UNUSED_RESULT;

common::ErrnoError MakeStreamInfo(const StreamConfig& config,
                                  bool check_folders,
                                  StreamInfo* sha,
                                  std::string* feedback_dir,
                                  std::string* data_dir,
                                  common::logging::LOG_LEVEL* logs_level) WARN_UNUSED_RESULT;

common::ErrnoError CleanStream(const StreamConfig& config) WARN_UNUSED_RESULT;

}  // namespace fastocloud
