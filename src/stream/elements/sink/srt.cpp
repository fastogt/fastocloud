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

#include "stream/elements/sink/srt.h"

#include <string>

namespace fastocloud {
namespace stream {
namespace elements {
namespace sink {

void ElementSrtSink::SetUri(const std::string& uri) {
  SetProperty("uri", uri);
}

void ElementSrtSink::SetMode(int mode) {
  SetProperty("mode", mode);
}

ElementSrtSink* make_srt_sink(const std::string& uri, int mode, element_id_t sink_id) {
  ElementSrtSink* sink = make_sink<ElementSrtSink>(sink_id);
  sink->SetMode(mode);
  sink->SetUri(uri);
  return sink;
}

}  // namespace sink
}  // namespace elements
}  // namespace stream
}  // namespace fastocloud
