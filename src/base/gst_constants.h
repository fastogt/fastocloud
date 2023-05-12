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

#include <array>

#define DECODEBIN "decodebin"
#define FAKE_SINK "fakesink"
#define TEST_SINK "testsink"
#define VIDEO_TEST_SRC "videotestsrc"
#define AUDIO_TEST_SRC "audiotestsrc"
#define DISPLAY_SRC "ximagesrc"
#define VIDEO_SCREEN_SINK "autovideosink"
#define AUDIO_SCREEN_SINK "autoaudiosink"
#define QUEUE "queue"
#define QUEUE2 "queue2"
#define H264_PARSE "h264parse"
#define H265_PARSE "h265parse"
#define MPEG_VIDEO_PARSE "mpegvideoparse"
#define AAC_PARSE "aacparse"
#define AC3_PARSE "ac3parse"
#define MPEG_AUDIO_PARSE "mpegaudioparse"
#define RAW_AUDIO_PARSE "rawaudioparse"
#define TEE "tee"
#define MP4_MUX "mp4mux"
#define QT_MUX "qtmux"
#define FLV_MUX "flvmux"
#define MPEGTS_MUX "mpegtsmux"
#define FILE_SINK "filesink"
#define RTP_MUX "rtpmux"
#define RTP_MPEG2_PAY "rtpmp2tpay"
#define RTP_H264_PAY "rtph264pay"
#define RTP_H265_PAY "rtph265pay"
#define RTP_AAC_PAY "rtpmp4apay"
#define RTP_AC3_PAY "rtpac3pay"
#define RTP_MPEG2_DEPAY "rtpmp2tdepay"
#define RTP_H264_DEPAY "rtph264depay"
#define RTP_H265_DEPAY "rtph265depay"
#define RTP_AAC_DEPAY "rtpmp4adepay"
#define RTP_AC3_DEPAY "rtpac3depay"
#define V4L2_SRC "v4l2src"
#define SPLIT_MUX_SINK "splitmuxsink"
#define ALSA_SRC "alsasrc"
#define MULTIFILE_SRC "multifilesrc"
#define APP_SRC "appsrc"
#define FILE_SRC "filesrc"
#define IMAGE_FREEZE "imagefreeze"
#define CAPS_FILTER "capsfilter"
#define AUDIO_CONVERT "audioconvert"
#define RG_VOLUME "rgvolume"
#define VOLUME "volume"
#define FAAC "faac"
#define VOAAC_ENC "voaacenc"
#define AUDIO_RESAMPLE "audioresample"
#define LAME_MP3_ENC "lamemp3enc"
#define VIDEO_CONVERT "videoconvert"
#define AV_DEINTERLACE "avdeinterlace"
#define DEINTERLACE "deinterlace"
#define ASPECT_RATIO "aspectratiocrop"
#define UDP_SINK "udpsink"
#define TCP_SERVER_SINK "tcpserversink"
#define RTMP_SINK "rtmpsink"
#define HLS_SINK "hlssink"
#define SOUP_HTTP_SRC "souphttpsrc"
#define DVB_SRC "dvbsrc"
#define VIDEO_SCALE "videoscale"
#define VIDEO_RATE "videorate"
#define MULTIFILE_SINK "multifilesink"
#define KVS_SINK "kvssink"

#define NV_H264_ENC "nvh264enc"

#define NV_H265_ENC "nvh265enc"

#define MSDK_H264_ENC "msdkh264enc"

#define X264_ENC "x264enc"

#define X265_ENC "x265enc"
#define MPEG2_ENC "mpeg2enc"

#define EAVC_ENC "eavcenc"

#define OPEN_H264_ENC "openh264enc"

#define UDP_SRC "udpsrc"
#define RTMP_SRC "rtmpsrc"
#define RTSP_SRC "rtspsrc"
#define TCP_SERVER_SRC "tcpserversrc"

#define VAAPI_H264_ENC "vaapih264enc"

#define VAAPI_MPEG2_ENC "vaapimpeg2enc"
#define VAAPI_DECODEBIN "vaapidecodebin"
#define VAAPI_POST_PROC "vaapipostproc"

#define GDK_PIXBUF_OVERLAY "gdkpixbufoverlay"
#define RSVG_OVERLAY "rsvgoverlay"
#define VIDEO_BOX "videobox"
#define VIDEO_MIXER "videomixer"
#define AUDIO_MIXER "audiomixer"
#define INTERLEAVE "interleave"
#define DEINTERLEAVE "deinterleave"
#define TEXT_OVERLAY "textoverlay"
#define VIDEO_CROP "videocrop"
#define SPECTRUM "spectrum"
#define LEVEL "level"
#define HLS_DEMUX "hlsdemux"
#define VIDEO_DECK_SINK "decklinkvideosink"
#define AUDIO_DECK_SINK "decklinkaudiosink"
#define INTERLACE "interlace"
#define AUTO_VIDEO_CONVERT "autovideoconvert"
#define TS_PARSE "tsparse"
#define AVDEC_H264 "avdec_h264"
#define TS_DEMUX "tsdemux"

#define AVDEC_AC3 "avdec_ac3"
#define AVDEC_AC3_FIXED "avdec_ac3_fixed"
#define AVDEC_AAC "avdec_aac"
#define AVDEC_AAC_FIXED "avdec_aac_fixed"
#define SOUP_HTTP_CLIENT_SINK "souphttpclientsink"

#define MFX_H264_ENC "mfxh264enc"
#define MFX_VPP "mfxvpp"
#define MFX_H264_DEC "mfxh264dec"

#define SRT_SRC "srtsrc"
#define SRT_SINK "srtsink"

// deep learning
#define TINY_YOLOV2 "tinyyolov2"
#define TINY_YOLOV3 "tinyyolov3"
#define DETECTION_OVERLAY "detectionoverlay"

#define SUPPORTED_VIDEO_PARSERS_COUNT 3
#define SUPPORTED_AUDIO_PARSERS_COUNT 4

extern const std::array<const char*, SUPPORTED_VIDEO_PARSERS_COUNT> kSupportedVideoParsers;
extern const std::array<const char*, SUPPORTED_AUDIO_PARSERS_COUNT> kSupportedAudioParsers;

#define SUPPORTED_VIDEO_ENCODERS_COUNT 10
#define SUPPORTED_AUDIO_ENCODERS_COUNT 3

extern const std::array<const char*, SUPPORTED_VIDEO_ENCODERS_COUNT> kSupportedVideoEncoders;
extern const std::array<const char*, SUPPORTED_AUDIO_ENCODERS_COUNT> kSupportedAudioEncoders;
