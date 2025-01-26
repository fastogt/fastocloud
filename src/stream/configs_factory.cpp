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

#include "stream/configs_factory.h"

#include <string>

#include <common/sprintf.h>

#include <fastotv/types/utils.h>

#include "base/config_fields.h"
#include "base/link_generator/ilink_generator.h"

#include "stream/streams/configs/encode_config.h"
#include "stream/streams/configs/relay_config.h"

namespace fastocloud {
namespace stream {

namespace {

const size_t kDefaultRestartAttempts = 10;

}  // namespace

common::Error make_config(const StreamConfig& config_args, Config** config) {
  if (!config_args || !config) {
    return common::make_error_inval();
  }

  int64_t type;
  common::Value* type_field = config_args->Find(TYPE_FIELD);
  if (!type_field || !type_field->GetAsInteger64(&type)) {
    return common::make_error("Define " TYPE_FIELD " variable and make it valid");
  }

  fastotv::StreamType stream_type = static_cast<fastotv::StreamType>(type);
  if (stream_type == fastotv::PROXY || stream_type == fastotv::VOD_PROXY) {
    return common::make_error("Proxy streams not handled for now");
  }

  const auto input_urls = ReadInput(config_args);
  if (!input_urls) {
    return common::make_error("Define " INPUT_FIELD " variable and make it valid");
  }

  const auto output_urls = ReadOutput(config_args);
  if (!output_urls) {
    return common::make_error("Define " OUTPUT_FIELD " variable and make it valid");
  }

  int64_t max_restart_attempts;
  common::Value* restart_attempts_field = config_args->Find(RESTART_ATTEMPTS_FIELD);
  if (!restart_attempts_field || restart_attempts_field->GetAsInteger64(&max_restart_attempts)) {
    max_restart_attempts = kDefaultRestartAttempts;
  }
  CHECK(max_restart_attempts > 0) << "restart attempts must be grether than 0";

  Config conf(static_cast<fastotv::StreamType>(stream_type), max_restart_attempts, *input_urls, *output_urls);

  conf.SetTimeToLifeStream(GetTTL(config_args));

  streams::AudioVideoConfig aconf(conf);
  bool have_video;
  common::Value* have_video_field = config_args->Find(HAVE_VIDEO_FIELD);
  if (have_video_field && have_video_field->GetAsBoolean(&have_video)) {
    aconf.SetHaveVideo(have_video);
  }

  bool have_audio;
  common::Value* have_audio_field = config_args->Find(HAVE_AUDIO_FIELD);
  if (have_audio_field && have_audio_field->GetAsBoolean(&have_audio)) {
    aconf.SetHaveAudio(have_audio);
  }

  bool have_subtitle;
  common::Value* have_subtitle_field = config_args->Find(HAVE_SUBTITLE_FIELD);
  if (have_subtitle_field && have_subtitle_field->GetAsBoolean(&have_subtitle)) {
    aconf.SetHaveSubtitle(have_subtitle);
  }

  int64_t audio_select;
  common::Value* audio_select_field = config_args->Find(AUDIO_SELECT_FIELD);
  if (audio_select_field && audio_select_field->GetAsInteger64(&audio_select)) {
    aconf.SetAudioSelect(audio_select);
  }

  bool loop;
  common::Value* loop_field = config_args->Find(LOOP_FIELD);
  if (loop_field && loop_field->GetAsBoolean(&loop)) {
    aconf.SetLoop(loop);
  }

  if (stream_type == fastotv::RELAY || stream_type == fastotv::TIMESHIFT_PLAYER ||
      stream_type == fastotv::VOD_RELAY || stream_type == fastotv::COD_RELAY) {
    streams::RelayConfig* rconfig = new streams::RelayConfig(aconf);

    std::string video_parser;
    common::Value* video_parser_field = config_args->Find(VIDEO_PARSER_FIELD);
    if (video_parser_field && video_parser_field->GetAsBasicString(&video_parser)) {
      rconfig->SetVideoParser(video_parser);
    }

    std::string audio_parser;
    common::Value* audio_parser_field = config_args->Find(AUDIO_PARSER_FIELD);
    if (audio_parser_field && audio_parser_field->GetAsBasicString(&audio_parser)) {
      rconfig->SetAudioParser(audio_parser);
    }

    if (stream_type == fastotv::VOD_RELAY) {
      streams::VodRelayConfig* vconf = new streams::VodRelayConfig(*rconfig);
      delete rconfig;
      rconfig = vconf;
      bool cleanup_ts;
      common::Value* cleanup_ts_field = config_args->Find(CLEANUP_TS_FIELD);
      if (cleanup_ts_field && cleanup_ts_field->GetAsBoolean(&cleanup_ts)) {
        vconf->SetCleanupTS(cleanup_ts);
      }
    }

    *config = rconfig;
    return common::Error();
  } else if (stream_type == fastotv::ENCODE || stream_type == fastotv::VOD_ENCODE ||
             stream_type == fastotv::COD_ENCODE || stream_type == fastotv::EVENT) {
    streams::EncodeConfig* econfig = new streams::EncodeConfig(aconf);
    bool relay_audio;
    common::Value* relay_audio_field = config_args->Find(RELAY_AUDIO_FIELD);
    if (relay_audio_field && relay_audio_field->GetAsBoolean(&relay_audio)) {
      econfig->SetRelayAudio(relay_audio);
    }

    bool relay_video;
    common::Value* relay_video_field = config_args->Find(RELAY_VIDEO_FIELD);
    if (relay_video_field && relay_video_field->GetAsBoolean(&relay_video)) {
      econfig->SetRelayVideo(relay_video);
    }

    bool deinterlace;
    common::Value* deinterlace_field = config_args->Find(DEINTERLACE_FIELD);
    if (deinterlace_field && deinterlace_field->GetAsBoolean(&deinterlace)) {
      econfig->SetDeinterlace(deinterlace);
    }

    common::media::Rational frame_rate;
    common::Value* frame_field = config_args->Find(FRAME_RATE_FIELD);
    std::string frame_str;
    if (frame_field && frame_field->GetAsBasicString(&frame_str) && common::ConvertFromString(frame_str, &frame_rate)) {
      econfig->SetFrameRate(frame_rate);
    }

    double volume;
    common::Value* volume_field = config_args->Find(VOLUME_FIELD);
    if (volume_field && volume_field->GetAsDouble(&volume)) {
      econfig->SetVolume(volume);
    }

    std::string video_codec;
    common::Value* video_codec_field = config_args->Find(VIDEO_CODEC_FIELD);
    if (video_codec_field && video_codec_field->GetAsBasicString(&video_codec)) {
      econfig->SetVideoEncoder(video_codec);
    }

    std::string audio_codec;
    common::Value* audio_codec_field = config_args->Find(AUDIO_CODEC_FIELD);
    if (audio_codec_field && audio_codec_field->GetAsBasicString(&audio_codec)) {
      econfig->SetAudioEncoder(audio_codec);
    }

    int64_t audio_channels;
    common::Value* audio_channels_field = config_args->Find(AUDIO_CHANNELS_FIELD);
    if (audio_channels_field && audio_channels_field->GetAsInteger64(&audio_channels)) {
      econfig->SetAudioChannelsCount(audio_channels);
    }

    common::HashValue* size_hash = nullptr;
    common::Value* size_field = config_args->Find(SIZE_FIELD);
    if (size_field && size_field->GetAsHash(&size_hash)) {
      auto size = fastotv::MakeSize(size_hash);
      econfig->SetSize(size);
    }

    int64_t v_bitrate;
    common::Value* video_bitrate_field = config_args->Find(VIDEO_BIT_RATE_FIELD);
    if (video_bitrate_field && video_bitrate_field->GetAsInteger64(&v_bitrate)) {
      econfig->SetVideoBitrate(v_bitrate);
    }

    int64_t a_bitrate;
    common::Value* audio_bitrate_field = config_args->Find(AUDIO_BIT_RATE_FIELD);
    if (audio_bitrate_field && audio_bitrate_field->GetAsInteger64(&a_bitrate)) {
      econfig->SetAudioBitrate(a_bitrate);
    }

#if defined(MACHINE_LEARNING)
    common::HashValue* deep_learning_hash = nullptr;
    common::Value* deep_learning_field = config_args->Find(DEEP_LEARNING_FIELD);
    if (deep_learning_field && deep_learning_field->GetAsHash(&deep_learning_hash)) {
      auto deep_learning = machine_learning::DeepLearning::MakeDeepLearning(deep_learning_hash);
      if (deep_learning) {
        econfig->SetDeepLearning(*deep_learning);
      }
    }

    common::HashValue* deep_learning_overlay_hash = nullptr;
    common::Value* deep_learning_overlay_field = config_args->Find(DEEP_LEARNING_OVERLAY_FIELD);
    if (deep_learning_overlay_field && deep_learning_overlay_field->GetAsHash(&deep_learning_overlay_hash)) {
      auto deep_learning_overlay =
          machine_learning::DeepLearningOverlay::MakeDeepLearningOverlay(deep_learning_overlay_hash);
      if (deep_learning_overlay) {
        econfig->SetDeepLearningOverlay(*deep_learning_overlay);
      }
    }
#endif

    common::HashValue* logo_hash = nullptr;
    common::Value* logo_field = config_args->Find(LOGO_FIELD);
    if (logo_field && logo_field->GetAsHash(&logo_hash)) {
      auto logo = fastotv::Logo::Make(logo_hash);
      econfig->SetLogo(logo);
    }

    common::HashValue* rsvg_logo_hash = nullptr;
    common::Value* rsvg_logo_field = config_args->Find(RSVG_LOGO_FIELD);
    if (rsvg_logo_field && rsvg_logo_field->GetAsHash(&rsvg_logo_hash)) {
      auto logo = fastotv::RSVGLogo::Make(rsvg_logo_hash);
      econfig->SetRSVGLogo(logo);
    }

    common::HashValue* rat_hash = nullptr;
    common::Value* rat_field = config_args->Find(ASPECT_RATIO_FIELD);
    if (rat_field && rat_field->GetAsHash(&rat_hash)) {
      auto rat = fastotv::MakeRational(rat_hash);
      econfig->SetAspectRatio(rat);
    }

    int64_t decl_vm;
    common::Value* decl_vm_field = config_args->Find(DECKLINK_VIDEO_MODE_FIELD);
    if (decl_vm_field && decl_vm_field->GetAsInteger64(&decl_vm)) {
      econfig->SetDecklinkMode(decl_vm);
    }

    if (stream_type == fastotv::VOD_ENCODE) {
      streams::VodEncodeConfig* vconf = new streams::VodEncodeConfig(*econfig);
      delete econfig;
      econfig = vconf;
      bool cleanup_ts;
      common::Value* cleanup_ts_field = config_args->Find(CLEANUP_TS_FIELD);
      if (cleanup_ts_field && cleanup_ts_field->GetAsBoolean(&cleanup_ts)) {
        vconf->SetCleanupTS(cleanup_ts);
      }
    }

    *config = econfig;
    return common::Error();
  } else if (stream_type == fastotv::TIMESHIFT_RECORDER || stream_type == fastotv::CATCHUP) {
    streams::RelayConfig rel(aconf);

    std::string video_parser;
    common::Value* video_parser_field = config_args->Find(VIDEO_PARSER_FIELD);
    if (video_parser_field && video_parser_field->GetAsBasicString(&video_parser)) {
      rel.SetVideoParser(video_parser);
    }

    std::string audio_parser;
    common::Value* audio_parser_field = config_args->Find(AUDIO_PARSER_FIELD);
    if (audio_parser_field && audio_parser_field->GetAsBasicString(&audio_parser)) {
      rel.SetAudioParser(audio_parser);
    }

    streams::TimeshiftConfig* tconf = new streams::TimeshiftConfig(rel);
    int64_t timeshift_chunk_duration;
    common::Value* timeshift_chunk_duration_field = config_args->Find(TIMESHIFT_CHUNK_DURATION_FIELD);
    if (timeshift_chunk_duration_field && timeshift_chunk_duration_field->GetAsInteger64(&timeshift_chunk_duration)) {
      tconf->SetTimeShiftChunkDuration(timeshift_chunk_duration);
    }
    CHECK(tconf->GetTimeShiftChunkDuration()) << "Avoid division by zero";

    *config = tconf;
    return common::Error();
  }

  return common::make_error(common::MemSPrintf("Unhandled stream type: %d", stream_type));
}

Config* make_config_copy(const Config* conf, const link_generator::ILinkGenerator* generator) {
  Config* copy = conf->Clone();
  if (generator) {
    input_t input = copy->GetInput();
    for (size_t i = 0; i < input.size(); ++i) {
      fastotv::InputUri url;
      if (generator->Generate(input[i], &url)) {
        input[i] = url;
        DEBUG_LOG() << "Generated url: " << url.GetUrl().spec();
      }
    }
    copy->SetInput(input);
  }

  return copy;
}

}  // namespace stream
}  // namespace fastocloud
