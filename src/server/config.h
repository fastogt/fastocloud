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

#include <string>

#include <common/error.h>
#include <common/net/types.h>
#include <common/types.h>
#include <common/uri/gurl.h>

namespace fastocloud {
namespace server {

struct Config {
  typedef common::utctime_t seconds_t;

  Config();

  static common::net::HostAndPort GetDefaultHost();

  bool IsValid() const;

  common::net::HostAndPort host;
  std::string alias;

  std::string log_path;
  common::logging::LOG_LEVEL log_level;
  // info
  common::uri::GURL hls_host;
  common::uri::GURL vods_host;
  common::uri::GURL cods_host;

  std::string hls_dir;
  std::string vods_dir;
  std::string cods_dir;
  std::string timeshifts_dir;
  std::string feedback_dir;
  std::string proxy_dir;
  std::string data_dir;

  std::string pyfastostream_path;
  seconds_t cods_ttl;
  seconds_t files_ttl;
  seconds_t report_node;
};

common::ErrnoError load_config_from_file(const std::string& config_absolute_path, Config* config) WARN_UNUSED_RESULT;

}  // namespace server
}  // namespace fastocloud
