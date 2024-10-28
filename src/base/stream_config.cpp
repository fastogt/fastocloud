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

#include "base/stream_config.h"

#include <string>

#include "base/config_fields.h"
#include "base/utils.h"

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <common/convert2string.h>
#include <common/file_system/file_system.h>

namespace fastocloud {
namespace {

bool ConvertFromString(common::ArrayValue* output_urls, fastocloud::output_t* out) {
  if (!output_urls || !out) {
    return false;
  }

  fastocloud::output_t output;
  for (size_t i = 0; i < output_urls->GetSize(); ++i) {
    common::Value* url = nullptr;
    common::HashValue* url_hash = nullptr;
    if (output_urls->Get(i, &url) && url->GetAsHash(&url_hash)) {
      const auto murl = fastotv::OutputUri::Make(url_hash);
      if (murl) {
        output.push_back(*murl);
      }
    }
  }
  *out = output;
  return true;
}

bool ConvertFromString(common::ArrayValue* input_urls, fastocloud::input_t* out) {
  if (!input_urls || !out) {
    return false;
  }

  fastocloud::input_t input;
  for (size_t i = 0; i < input_urls->GetSize(); ++i) {
    common::Value* url = nullptr;
    common::HashValue* url_hash = nullptr;
    if (input_urls->Get(i, &url) && url->GetAsHash(&url_hash)) {
      const auto murl = fastotv::InputUri::Make(url_hash);
      if (murl) {
        input.push_back(*murl);
      }
    }
  }
  *out = input;
  return true;
}

}  // namespace

common::Optional<input_t> ReadInput(const StreamConfig& config) {
  if (!config) {
    return common::Optional<input_t>();
  }

  common::Value* input_field = config->Find(INPUT_FIELD);
  common::ArrayValue* input_hash = nullptr;
  if (!input_field || !input_field->GetAsList(&input_hash)) {
    return common::Optional<input_t>();
  }

  input_t linput;
  if (!ConvertFromString(input_hash, &linput)) {
    return common::Optional<input_t>();
  }

  return common::Optional<input_t>(linput);
}

common::Optional<output_t> ReadOutput(const StreamConfig& config) {
  if (!config) {
    return common::Optional<output_t>();
  }

  common::Value* output_field = config->Find(OUTPUT_FIELD);
  common::ArrayValue* output_hash = nullptr;
  if (!output_field || !output_field->GetAsList(&output_hash)) {
    return common::Optional<output_t>();
  }

  output_t loutput;
  if (!ConvertFromString(output_hash, &loutput)) {
    return common::Optional<output_t>();
  }

  return common::Optional<output_t>(loutput);
}

common::Optional<fastotv::stream_id_t> GetSid(const StreamConfig& config) {
  if (!config) {
    return common::Optional<fastotv::stream_id_t>();
  }

  common::Value* id_field = config->Find(ID_FIELD);
  fastotv::stream_id_t lsid;
  if (id_field && id_field->GetAsBasicString(&lsid)) {
    return common::Optional<fastotv::stream_id_t>(lsid);
  }

  return common::Optional<fastotv::stream_id_t>();
}

common::Optional<fastotv::StreamTTL> GetTTL(const StreamConfig& config) {
  if (!config) {
    return common::Optional<fastotv::StreamTTL>();
  }

  common::HashValue* ttl_hash = nullptr;
  common::Value* frame_field = config->Find(AUTO_EXIT_TIME_FIELD);
  if (frame_field && frame_field->GetAsHash(&ttl_hash)) {
    return fastotv::StreamTTL::Make(ttl_hash);
  }

  return common::Optional<fastotv::StreamTTL>();
}

common::ErrnoError MakeStreamInfo(const StreamConfig& config,
                                  bool check_folders,
                                  StreamInfo* sha,
                                  std::string* feedback_dir,
                                  std::string* data_dir,
                                  common::logging::LOG_LEVEL* logs_level) {
  if (!sha || !data_dir || !feedback_dir || !logs_level) {
    return common::make_errno_error_inval();
  }

  auto sid = GetSid(config);
  if (!sid) {
    return common::make_errno_error("Define " ID_FIELD " variable and make it valid", EAGAIN);
  }

  StreamInfo lsha;
  lsha.id = *sid;

  int64_t type;
  common::Value* type_field = config->Find(TYPE_FIELD);
  if (!type_field || !type_field->GetAsInteger64(&type)) {
    return common::make_errno_error("Define " TYPE_FIELD " variable and make it valid", EAGAIN);
  }
  lsha.type = static_cast<fastotv::StreamType>(type);

  if (lsha.type == fastotv::PROXY || lsha.type == fastotv::VOD_PROXY) {
    return common::make_errno_error("Proxy streams not handled for now", EINVAL);
  }

  std::string lfeedback_dir;
  common::Value* feedback_field = config->Find(FEEDBACK_DIR_FIELD);
  if (!feedback_field || !feedback_field->GetAsBasicString(&lfeedback_dir)) {
    return common::make_errno_error("Define " FEEDBACK_DIR_FIELD " variable and make it valid", EAGAIN);
  }

  std::string ldata_dir;
  common::Value* data_field = config->Find(DATA_DIR_FIELD);
  if (!data_field || !data_field->GetAsBasicString(&ldata_dir)) {
    return common::make_errno_error("Define " DATA_DIR_FIELD " variable and make it valid", EAGAIN);
  }

  int64_t llogs_level;
  common::Value* log_level_field = config->Find(LOG_LEVEL_FIELD);
  if (!log_level_field || !log_level_field->GetAsInteger64(&llogs_level)) {
    llogs_level = common::logging::LOG_LEVEL_DEBUG;
  }

  common::ErrnoError errn = CreateAndCheckDir(lfeedback_dir);
  if (errn) {
    return errn;
  }

  errn = CreateAndCheckDir(ldata_dir);
  if (errn) {
    return errn;
  }

  const auto input = ReadInput(config);
  if (!input) {
    return common::make_errno_error("Define " INPUT_FIELD " variable and make it valid", EAGAIN);
  }
  lsha.input = *input;

  if (check_folders) {
    bool is_timeshift_rec = type == fastotv::TIMESHIFT_RECORDER;  // no outputs
    if (is_timeshift_rec || type == fastotv::CATCHUP) {
      std::string timeshift_dir;
      common::Value* timeshift_dir_field = config->Find(TIMESHIFT_DIR_FIELD);
      if (!timeshift_dir_field || !timeshift_dir_field->GetAsBasicString(&timeshift_dir)) {
        return common::make_errno_error("Define " TIMESHIFT_DIR_FIELD " variable and make it valid", EAGAIN);
      }

      errn = CreateAndCheckDir(timeshift_dir);
      if (errn) {
        return errn;
      }
    }

    if (!is_timeshift_rec) {
      const auto output = ReadOutput(config);
      if (!output) {
        return common::make_errno_error("Define " OUTPUT_FIELD " variable and make it valid", EAGAIN);
      }

      for (const auto& out_uri : *output) {
        auto ouri = out_uri.GetUrl();
        if (ouri.SchemeIsHTTPOrHTTPS()) {
          if (out_uri.IsHls()) {
            const auto http_root = out_uri.GetHttpRoot();
            if (!http_root) {
              return common::make_errno_error_inval();
            }

            const std::string http_root_str = http_root->GetPath();
            common::ErrnoError errn = CreateAndCheckDir(http_root_str);
            if (errn) {
              return errn;
            }
          }
        } else if (ouri.SchemeIsFile()) {
          const auto file_path = common::file_system::ascii_file_string_path(ouri.path());
          if (!file_path.IsValid()) {
            return common::make_errno_error_inval();
          }

          const std::string file_root_str = file_path.GetDirectory();
          common::ErrnoError errn = CreateAndCheckDir(file_root_str);
          if (errn) {
            return errn;
          }
        }
        lsha.output.push_back(out_uri);
      }
    }
  }

  *logs_level = static_cast<common::logging::LOG_LEVEL>(llogs_level);
  *feedback_dir = lfeedback_dir;
  *data_dir = ldata_dir;
  *sha = lsha;
  return common::ErrnoError();
}

common::ErrnoError CleanStream(const StreamConfig& config) {
  auto sid = GetSid(config);
  if (!sid) {
    return common::make_errno_error("Define " ID_FIELD " variable and make it valid", EAGAIN);
  }

  StreamInfo lsha;
  lsha.id = *sid;

  int64_t type;
  common::Value* type_field = config->Find(TYPE_FIELD);
  if (!type_field || !type_field->GetAsInteger64(&type)) {
    return common::make_errno_error("Define " TYPE_FIELD " variable and make it valid", EAGAIN);
  }
  lsha.type = static_cast<fastotv::StreamType>(type);

  if (lsha.type == fastotv::PROXY || lsha.type == fastotv::VOD_PROXY) {
    return common::make_errno_error("Proxy streams not handled for now", EINVAL);
  }

  std::string lfeedback_dir;
  common::Value* feedback_field = config->Find(FEEDBACK_DIR_FIELD);
  if (!feedback_field || !feedback_field->GetAsBasicString(&lfeedback_dir)) {
    return common::make_errno_error("Define " FEEDBACK_DIR_FIELD " variable and make it valid", EAGAIN);
  }

  std::string ldata_dir;
  common::Value* data_field = config->Find(DATA_DIR_FIELD);
  if (!data_field || !data_field->GetAsBasicString(&ldata_dir)) {
    return common::make_errno_error("Define " DATA_DIR_FIELD " variable and make it valid", EAGAIN);
  }

  int64_t llogs_level;
  common::Value* log_level_field = config->Find(LOG_LEVEL_FIELD);
  if (!log_level_field || !log_level_field->GetAsInteger64(&llogs_level)) {
    llogs_level = common::logging::LOG_LEVEL_DEBUG;
  }

  common::ErrnoError errn = common::file_system::remove_directory(lfeedback_dir, true);
  if (errn) {
    return errn;
  }

  errn = common::file_system::remove_directory(ldata_dir, true);
  if (errn) {
    return errn;
  }

  const auto input = ReadInput(config);
  if (!input) {
    return common::make_errno_error("Define " INPUT_FIELD " variable and make it valid", EAGAIN);
  }
  lsha.input = *input;

  bool is_timeshift_rec = type == fastotv::TIMESHIFT_RECORDER;  // no outputs
  if (is_timeshift_rec || type == fastotv::CATCHUP) {
    std::string timeshift_dir;
    common::Value* timeshift_dir_field = config->Find(TIMESHIFT_DIR_FIELD);
    if (!timeshift_dir_field || !timeshift_dir_field->GetAsBasicString(&timeshift_dir)) {
      return common::make_errno_error("Define " TIMESHIFT_DIR_FIELD " variable and make it valid", EAGAIN);
    }

    errn = common::file_system::remove_directory(timeshift_dir, true);
    if (errn) {
      return errn;
    }
  }

  if (!is_timeshift_rec) {
    const auto output = ReadOutput(config);
    if (!output) {
      return common::make_errno_error("Define " OUTPUT_FIELD " variable and make it valid", EAGAIN);
    }

    for (const auto& out_uri : *output) {
      auto ouri = out_uri.GetUrl();
      if (ouri.SchemeIsHTTPOrHTTPS()) {
        if (out_uri.IsHls()) {
          const auto http_root = out_uri.GetHttpRoot();
          if (!http_root) {
            return common::make_errno_error_inval();
          }

          const std::string http_root_str = http_root->GetPath();
          common::ErrnoError errn = common::file_system::remove_directory(http_root_str, true);
          if (errn) {
            return errn;
          }
        }
      } else if (ouri.SchemeIsFile()) {
        const auto file_path = common::file_system::ascii_file_string_path(ouri.path());
        if (!file_path.IsValid()) {
          return common::make_errno_error_inval();
        }

        const std::string file_root_str = file_path.GetDirectory();
        common::ErrnoError errn = common::file_system::remove_directory(file_root_str, true);
        if (errn) {
          return errn;
        }
      }
      lsha.output.push_back(out_uri);
    }
  }

  return common::ErrnoError();
}

}  // namespace fastocloud
