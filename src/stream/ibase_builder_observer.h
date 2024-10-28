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

#include <common/uri/gurl.h>

#include "stream/stypes.h"

namespace fastocloud {
namespace stream {

namespace pad {
class Pad;
}

class IBaseBuilderObserver {
 public:
  typedef common::uri::GURL url_t;
  virtual void OnInpudSrcPadCreated(pad::Pad* src_pad, element_id_t id, const url_t& url) = 0;
  virtual void OnOutputSinkPadCreated(pad::Pad* sink_pad, element_id_t id, const url_t& url, bool need_push) = 0;

  virtual ~IBaseBuilderObserver();
};

}  // namespace stream
}  // namespace fastocloud
