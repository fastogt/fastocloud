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

#include "server/config.h"

#include <fstream>
#include <utility>

#include <common/convert2string.h>
#include <common/file_system/path.h>
#include <common/value.h>

#define SETTINGS_SECTION "[settings]"
#define SERVICE_LOG_PATH_FIELD "log_path"
#define SERVICE_LOG_LEVEL_FIELD "log_level"
#define SERVICE_HOST_FIELD "host"
#define SERVICE_ALIAS_FIELD "alias"

#define SERVICE_HLS_HOST_FIELD "hls_host"
#define SERVICE_VODS_HOST_FIELD "vods_host"
#define SERVICE_CODS_HOST_FIELD "cods_host"

#define SERVICE_HLS_DIR_FIELD "hls_dir"
#define SERVICE_VODS_DIR_FIELD "vods_dir"
#define SERVICE_CODS_DIR_FIELD "cods_dir"
#define SERVICE_TIMESHIFTS_DIR_FIELD "timeshifts_dir"
#define SERVICE_FEEDBACK_DIR_FIELD "feedback_dir"
#define SERVICE_PROXY_DIR_FIELD "proxy_dir"
#define SERVICE_DATA_DIR_FIELD "data_dir"

#define SERVICE_CODS_TTL_FIELD "cods_ttl"
#define SERVICE_FILES_TTL_FIELD "files_ttl"
#define SERVICE_PYFASTOSTREAM_PATH_FIELD "pyfastostream_path"
#define SERVICE_REPORT_NODE_STATS_FIELD "report_node_stats"

#define DUMMY_LOG_FILE_PATH "/dev/null"
#define REPORT_NODE_STATS 10

namespace {
common::Optional<std::pair<std::string, std::string>> GetKeyValue(const std::string& line, char separator) {
  const size_t pos = line.find(separator);
  if (pos != std::string::npos) {
    const std::string key = line.substr(0, pos);
    const std::string value = line.substr(pos + 1);
    return std::make_pair(key, value);
  }

  return common::Optional<std::pair<std::string, std::string>>();
}

common::ErrnoError ReadConfigFile(const std::string& path, common::HashValue** args) {
  if (!args) {
    return common::make_errno_error_inval();
  }

  if (path.empty()) {
    return common::make_errno_error("Invalid config path", EINVAL);
  }

  std::ifstream config(path);
  if (!config.is_open()) {
    return common::make_errno_error("Failed to open config file", EINVAL);
  }

  common::HashValue* options = new common::HashValue;
  std::string line;
  while (getline(config, line)) {
    if (line == SETTINGS_SECTION) {
      continue;
    }

    const auto kv = GetKeyValue(line, '=');
    if (!kv) {
      continue;
    }

    const auto pair = *kv;
    if (pair.first == SERVICE_LOG_PATH_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_LOG_LEVEL_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_HOST_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_ALIAS_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_HLS_HOST_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_VODS_HOST_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_CODS_HOST_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_HLS_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_VODS_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_CODS_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_TIMESHIFTS_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_FEEDBACK_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_PROXY_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_DATA_DIR_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_PYFASTOSTREAM_PATH_FIELD) {
      options->Insert(pair.first, common::Value::CreateStringValueFromBasicString(pair.second));
    } else if (pair.first == SERVICE_CODS_TTL_FIELD) {
      common::utctime_t ttl;
      if (common::ConvertFromString(pair.second, &ttl)) {
        options->Insert(pair.first, common::Value::CreateTimeValue(ttl));
      }
    } else if (pair.first == SERVICE_FILES_TTL_FIELD) {
      common::utctime_t ttl;
      if (common::ConvertFromString(pair.second, &ttl)) {
        options->Insert(pair.first, common::Value::CreateTimeValue(ttl));
      }
    } else if (pair.first == SERVICE_REPORT_NODE_STATS_FIELD) {
      common::utctime_t report;
      if (common::ConvertFromString(pair.second, &report)) {
        options->Insert(pair.first, common::Value::CreateTimeValue(report));
      }
    }
  }

  *args = options;
  return common::ErrnoError();
}
}  // namespace

namespace fastocloud {
namespace server {

Config::Config()
    : host(GetDefaultHost()),
      log_path(DUMMY_LOG_FILE_PATH),
      log_level(common::logging::LOG_LEVEL_INFO),
      hls_dir(STREAMER_SERVICE_HLS_DIR),
      vods_dir(STREAMER_SERVICE_VODS_DIR),
      cods_dir(STREAMER_SERVICE_CODS_DIR),
      timeshifts_dir(STREAMER_SERVICE_TIMESHIFTS_DIR),
      feedback_dir(STREAMER_SERVICE_FEEDBACK_DIR),
      proxy_dir(STREAMER_SERVICE_PROXY_DIR),
      data_dir(STREAMER_SERVICE_DATA_DIR),
      pyfastostream_path(STREAMER_SERVICE_PYFASTOSTREAM_PATH),
      cods_ttl(CODS_TTL),
      files_ttl(FILES_TTL),
      report_node(REPORT_NODE_STATS) {}

common::net::HostAndPort Config::GetDefaultHost() {
  return common::net::HostAndPort::CreateLocalHostIPV4(CLIENT_PORT);
}

bool Config::IsValid() const {
  return host.IsValid();
}

common::ErrnoError load_config_from_file(const std::string& config_absolute_path, Config* config) {
  if (!config) {
    return common::make_errno_error_inval();
  }

  Config lconfig;
  common::HashValue* slave_config_args = nullptr;
  common::ErrnoError err = ReadConfigFile(config_absolute_path, &slave_config_args);
  if (err) {
    return err;
  }

  common::Value* log_path_field = slave_config_args->Find(SERVICE_LOG_PATH_FIELD);
  if (!log_path_field || !log_path_field->GetAsBasicString(&lconfig.log_path)) {
    lconfig.log_path = DUMMY_LOG_FILE_PATH;
  }

  std::string level_str;
  common::logging::LOG_LEVEL level = common::logging::LOG_LEVEL_INFO;
  common::Value* level_field = slave_config_args->Find(SERVICE_LOG_LEVEL_FIELD);
  if (level_field && level_field->GetAsBasicString(&level_str)) {
    if (!common::logging::text_to_log_level(level_str.c_str(), &level)) {
      level = common::logging::LOG_LEVEL_INFO;
    }
  }
  lconfig.log_level = level;

  common::Value* host_field = slave_config_args->Find(SERVICE_HOST_FIELD);
  std::string host_str;
  if (!host_field || !host_field->GetAsBasicString(&host_str) || !common::ConvertFromString(host_str, &lconfig.host)) {
    lconfig.host = Config::GetDefaultHost();
  }

  common::Value* alias_field = slave_config_args->Find(SERVICE_ALIAS_FIELD);
  if (!alias_field || !alias_field->GetAsBasicString(&lconfig.alias)) {
    lconfig.alias = STREAMER_SERVICE_ALIAS;
  }

  common::Value* http_host_field = slave_config_args->Find(SERVICE_HLS_HOST_FIELD);
  std::string http_host_str;
  if (http_host_field && http_host_field->GetAsBasicString(&http_host_str)) {
    common::uri::GURL gurl(http_host_str);
    if (gurl.is_valid() && gurl.SchemeIsHTTPOrHTTPS()) {
      lconfig.hls_host = gurl;
    } else {
      lconfig.hls_host = common::uri::GURL(SERVICE_HLS_HOST_FIELD);
    }
  }

  common::Value* vods_host_field = slave_config_args->Find(SERVICE_VODS_HOST_FIELD);
  std::string vods_host_str;
  if (vods_host_field && vods_host_field->GetAsBasicString(&vods_host_str)) {
    common::uri::GURL gurl(vods_host_str);
    if (gurl.is_valid() && gurl.SchemeIsHTTPOrHTTPS()) {
      lconfig.vods_host = gurl;
    } else {
      lconfig.vods_host = common::uri::GURL(SERVICE_VODS_HOST_FIELD);
    }
  }

  common::Value* cods_host_field = slave_config_args->Find(SERVICE_CODS_HOST_FIELD);
  std::string cods_host_str;
  if (cods_host_field && cods_host_field->GetAsBasicString(&cods_host_str)) {
    common::uri::GURL gurl(cods_host_str);
    if (gurl.is_valid() && gurl.SchemeIsHTTPOrHTTPS()) {
      lconfig.cods_host = gurl;
    } else {
      lconfig.cods_host = common::uri::GURL(SERVICE_CODS_HOST_FIELD);
    }
  }

  common::Value* hls_field = slave_config_args->Find(SERVICE_HLS_DIR_FIELD);
  if (!hls_field || !hls_field->GetAsBasicString(&lconfig.hls_dir)) {
    lconfig.hls_dir = STREAMER_SERVICE_HLS_DIR;
  }
  lconfig.hls_dir = common::file_system::stable_dir_path(lconfig.hls_dir);

  common::Value* vods_field = slave_config_args->Find(SERVICE_VODS_DIR_FIELD);
  if (!vods_field || !vods_field->GetAsBasicString(&lconfig.vods_dir)) {
    lconfig.vods_dir = STREAMER_SERVICE_VODS_DIR;
  }
  lconfig.vods_dir = common::file_system::stable_dir_path(lconfig.vods_dir);

  common::Value* cods_field = slave_config_args->Find(SERVICE_CODS_DIR_FIELD);
  if (!cods_field || !cods_field->GetAsBasicString(&lconfig.cods_dir)) {
    lconfig.cods_dir = STREAMER_SERVICE_CODS_DIR;
  }
  lconfig.cods_dir = common::file_system::stable_dir_path(lconfig.cods_dir);

  common::Value* timeshifts_field = slave_config_args->Find(SERVICE_TIMESHIFTS_DIR_FIELD);
  if (!timeshifts_field || !timeshifts_field->GetAsBasicString(&lconfig.timeshifts_dir)) {
    lconfig.timeshifts_dir = STREAMER_SERVICE_TIMESHIFTS_DIR;
  }
  lconfig.timeshifts_dir = common::file_system::stable_dir_path(lconfig.timeshifts_dir);

  common::Value* feedback_field = slave_config_args->Find(SERVICE_FEEDBACK_DIR_FIELD);
  if (!feedback_field || !feedback_field->GetAsBasicString(&lconfig.feedback_dir)) {
    lconfig.feedback_dir = STREAMER_SERVICE_FEEDBACK_DIR;
  }
  lconfig.feedback_dir = common::file_system::stable_dir_path(lconfig.feedback_dir);

  common::Value* proxy_field = slave_config_args->Find(SERVICE_PROXY_DIR_FIELD);
  if (!proxy_field || !proxy_field->GetAsBasicString(&lconfig.proxy_dir)) {
    lconfig.proxy_dir = STREAMER_SERVICE_PROXY_DIR;
  }
  lconfig.proxy_dir = common::file_system::stable_dir_path(lconfig.proxy_dir);

  common::Value* data_field = slave_config_args->Find(SERVICE_DATA_DIR_FIELD);
  if (!data_field || !data_field->GetAsBasicString(&lconfig.data_dir)) {
    lconfig.data_dir = STREAMER_SERVICE_DATA_DIR;
  }
  lconfig.data_dir = common::file_system::stable_dir_path(lconfig.data_dir);

  common::Value* PyFastoStream_field = slave_config_args->Find(SERVICE_PYFASTOSTREAM_PATH_FIELD);
  if (!PyFastoStream_field || !PyFastoStream_field->GetAsBasicString(&lconfig.pyfastostream_path)) {
    lconfig.pyfastostream_path = STREAMER_SERVICE_PYFASTOSTREAM_PATH;
  }
  lconfig.pyfastostream_path = common::file_system::prepare_path(lconfig.pyfastostream_path);

  common::Value* cods_ttl_field = slave_config_args->Find(SERVICE_CODS_TTL_FIELD);
  if (!cods_ttl_field || !cods_ttl_field->GetAsTime(&lconfig.cods_ttl)) {
    lconfig.cods_ttl = CODS_TTL;
  }

  common::Value* files_ttl_field = slave_config_args->Find(SERVICE_FILES_TTL_FIELD);
  if (!files_ttl_field || !files_ttl_field->GetAsTime(&lconfig.files_ttl)) {
    lconfig.files_ttl = FILES_TTL;
  }

  common::Value* report_node_stats_field = slave_config_args->Find(SERVICE_REPORT_NODE_STATS_FIELD);
  if (!report_node_stats_field || !report_node_stats_field->GetAsTime(&lconfig.report_node)) {
    lconfig.report_node = REPORT_NODE_STATS;
  }

  *config = lconfig;
  delete slave_config_args;
  return common::ErrnoError();
}

}  // namespace server
}  // namespace fastocloud
