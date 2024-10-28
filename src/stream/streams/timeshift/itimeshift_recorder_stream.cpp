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

#include "stream/streams/timeshift/itimeshift_recorder_stream.h"

namespace fastocloud {
namespace stream {
namespace streams {

ITimeShiftRecorderStream::ITimeShiftRecorderStream(const RelayConfig* config,
                                                   const TimeShiftInfo& info,
                                                   IStreamClient* client,
                                                   StreamStruct* stats)
    : base_class(config, client, stats), timeshift_info_(info) {
  CHECK(GetType() == fastotv::TIMESHIFT_RECORDER || GetType() == fastotv::CATCHUP);
}

const char* ITimeShiftRecorderStream::ClassName() const {
  return "TimeShiftRecorderStream";
}

TimeShiftInfo ITimeShiftRecorderStream::GetTimeshiftInfo() const {
  return timeshift_info_;
}

}  // namespace streams
}  // namespace stream
}  // namespace fastocloud
