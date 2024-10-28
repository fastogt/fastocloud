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

#include <common/libev/http/http_server.h>

namespace fastocloud {
namespace server {

class HttpServer : public common::libev::http::HttpServer {
 public:
  typedef common::libev::http::HttpServer base_class;
  explicit HttpServer(const common::net::HostAndPort& host, common::libev::IoLoopObserver* observer = nullptr);

 private:
  common::libev::tcp::TcpClient* CreateClient(const common::net::socket_info& info, void* user) override;
};

class HttpsServer : public common::libev::http::HttpServer {
 public:
  typedef common::libev::http::HttpServer base_class;
  explicit HttpsServer(const common::net::HostAndPort& host, common::libev::IoLoopObserver* observer = nullptr);

  common::ErrnoError LoadCertificates(const std::string& cert, const std::string& key);

 private:
  common::libev::tcp::TcpClient* CreateClient(const common::net::socket_info& info, void* user) override;
};

}  // namespace server
}  // namespace fastocloud
