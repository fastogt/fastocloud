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

#define REQUEST_STOP_STREAM "stop"
#define REQUEST_RESTART_STREAM "restart"

#define CHANGED_SOURCES_STREAM "changed_source_stream"
#define STATISTIC_STREAM "statistic_stream"

#if defined(MACHINE_LEARNING)
#define ML_NOTIFICATION_STREAM "ml_notification_stream"
#endif
