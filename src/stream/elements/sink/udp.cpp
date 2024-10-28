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

#include "stream/elements/sink/udp.h"

#include <string>

namespace fastocloud {
namespace stream {
namespace elements {
namespace sink {

void ElementUDPSink::SetHost(const std::string& host) {
  SetProperty("host", host);
}

void ElementUDPSink::SetPort(uint16_t port) {
  SetProperty("port", port);
}

ElementUDPSink* make_udp_sink(const common::net::HostAndPort& host, element_id_t sink_id) {
  ElementUDPSink* udp_out = make_sink<ElementUDPSink>(sink_id);
  udp_out->SetHost(host.GetHost());
  udp_out->SetPort(host.GetPort());
  return udp_out;
}

}  // namespace sink
}  // namespace elements
}  // namespace stream
}  // namespace fastocloud
