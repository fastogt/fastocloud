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

#include "stream/elements/sources/rtmpsrc.h"

#include <string>

namespace fastocloud {
namespace stream {
namespace elements {
namespace sources {

void ElementRTMPSrc::SetTimeOut(gint secs) {
  SetProperty("timeout", secs);
}

ElementRTMPSrc* make_rtmp_src(const std::string& location, gint timeout_secs, element_id_t input_id) {
  ElementRTMPSrc* rtmp_src = make_sources<ElementRTMPSrc>(input_id);
  rtmp_src->SetLocation(location);
  rtmp_src->SetTimeOut(timeout_secs);
  return rtmp_src;
}

}  // namespace sources
}  // namespace elements
}  // namespace stream
}  // namespace fastocloud
