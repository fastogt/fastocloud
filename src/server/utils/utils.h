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

#include <common/error.h>
#include <common/net/socket_info.h>

namespace fastocloud {
namespace server {

#if defined(OS_POSIX)
common::ErrnoError CreatePipe(common::net::socket_descr_t* read_client_fd,
                              common::net::socket_descr_t* write_client_fd);
#endif
common::ErrnoError CreateSocketPair(common::net::socket_descr_t* parent_sock, common::net::socket_descr_t* child_sock);

}  // namespace server
}  // namespace fastocloud
