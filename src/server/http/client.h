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

#include <common/libev/http/http_client.h>

typedef struct ssl_st SSL;

namespace fastocloud {
namespace server {

class HttpClient : public common::libev::http::HttpServerClient {
 public:
  typedef common::libev::http::HttpServerClient base_class;

  HttpClient(common::libev::IoLoop* server, const common::net::socket_info& info);

  const char* ClassName() const override;
};

class HttpsClient : public common::libev::http::HttpServerClient {
 public:
  typedef common::libev::http::HttpServerClient base_class;

  HttpsClient(common::libev::IoLoop* server, const common::net::socket_info& info, SSL* ssl);

  const char* ClassName() const override;
};

}  // namespace server
}  // namespace fastocloud
