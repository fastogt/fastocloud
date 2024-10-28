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

#include "server/daemon/commands_info/stream/stop_info.h"

#define FORCE_FIELD "force"

namespace fastocloud {
namespace server {
namespace stream {

StopInfo::StopInfo() : base_class(), force_(false) {}

StopInfo::StopInfo(const fastotv::stream_id_t& stream_id, bool force) : base_class(stream_id), force_(force) {}

bool StopInfo::GetForce() const {
  return force_;
}

common::Error StopInfo::DoDeSerialize(json_object* serialized) {
  StopInfo res;
  common::Error err = res.base_class::DoDeSerialize(serialized);
  if (err) {
    return err;
  }

  bool force;
  err = GetBoolField(serialized, FORCE_FIELD, &force);
  if (err) {
    return err;
  }

  res.force_ = force;
  *this = res;
  return common::Error();
}

common::Error StopInfo::SerializeFields(json_object* out) const {
  ignore_result(SetBoolField(out, FORCE_FIELD, force_));
  return base_class::SerializeFields(out);
}

}  // namespace stream
}  // namespace server
}  // namespace fastocloud
