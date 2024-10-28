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

#include <common/libev/tcp/tcp_client.h>

#include <fastotv/protocol/protocol.h>

namespace fastocloud {
namespace server {
namespace tcp {

class Client : public fastotv::protocol::protocol_client_t {
 public:
  typedef fastotv::protocol::protocol_client_t base_class;

  Client(common::libev::IoLoop* server, const common::net::socket_info& info);

  const char* ClassName() const override;

 protected:
  descriptor_t GetFd() const override;

 private:
  common::ErrnoError DoSingleWrite(const void* data, size_t size, size_t* nwrite_out) override;
  common::ErrnoError DoSingleRead(void* out, size_t max_size, size_t* nread) override;
  common::ErrnoError DoClose() override;

  common::libev::tcp::TcpClient* client_;
};

}  // namespace tcp
}  // namespace server
}  // namespace fastocloud
