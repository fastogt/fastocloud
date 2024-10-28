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

#include "server/daemon/server.h"

#include "server/daemon/client.h"

namespace fastocloud {
namespace server {

DaemonServer::DaemonServer(const common::net::HostAndPort& host, common::libev::IoLoopObserver* observer)
    : base_class(new common::net::ServerSocketEvTcp(host), true, observer) {}

common::libev::tcp::TcpClient* DaemonServer::CreateClient(const common::net::socket_info& info, void* user) {
  UNUSED(user);
  return new ProtocoledDaemonClient(this, info);
}

}  // namespace server
}  // namespace fastocloud
