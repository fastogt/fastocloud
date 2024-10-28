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
    along with fastocloud. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <common/optional.h>

#include <fastotv/types/input_uri.h>
#include <fastotv/types/output_uri.h>

#define TS_EXTENSION "ts"
#define M3U8_EXTENSION "m3u8"
#define DASH_EXTENSION "mpd"
#define M3U8_CHUNK_MARKER "#EXTINF"
#define CHUNK_EXT "." TS_EXTENSION

#define DUMP_FILE_NAME "dump.html"
#define CONFIG_FILE_NAME "config.json"

namespace fastocloud {

typedef common::Optional<double> volume_t;
typedef double alpha_t;
typedef common::Optional<int> bit_rate_t;

typedef std::vector<fastotv::InputUri> input_t;
typedef std::vector<fastotv::OutputUri> output_t;

}  // namespace fastocloud
