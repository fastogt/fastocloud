### FastoCloud Media
[![Join the chat at https://discord.com/invite/cnUXsws](https://img.shields.io/discord/584773460585086977?label=discord)](https://discord.gg/Fxbfx3d5dV)

#### [Technical details](https://fastogt.com/static/projects/all_presentations/media_server.pdf)

**FastoCloud Media** is a reliable server software solution for streaming video content of any type and complexity. It enables users to host live video broadcasts using IP/NDI/Analog cameras, provide on-demand video access, and seamlessly integrate video streaming into various projects such as intercom systems, tv streaming, ott, webinars, recording streams and many other. For managment **FastoCloud** nodes can be used:
- [**UI**](https://gitlab.com/fastogt/wsfastocloud)
- [**API**](https://fastogt.stoplight.io/docs/fastocloud-api/6dbac8c0c0a0f-fasto-cloud-community-pro-ml-versions)

Software can start as [**Linux**](https://fastocloud.com/downloads.html) service or in [**Docker**](https://hub.docker.com/repository/docker/fastogt/fastocloud).

Have 3 types of services:
- [**[COM]**](https://gitlab.com/fastogt/fastocloud) Community version
- [**[PRO]**](https://fastocloud.com/downloads.html) Professional version
- [**[ML]**](https://fastocloud.com/downloads.html) Computer vision/AI/ML version

[About FastoCloud](https://github.com/fastogt/fastocloud_docs/wiki)
===============
### Features:
* Cross-platform (Linux, MacOSX, FreeBSD, Raspbian/Armbian, Windows)
* GPU/CPU Encode/Decode/Post Processing
  * Intel QuickSync
  * Nvidia NVENC
  * AMD
* Restreaming
* Stream/Server statistics
* RTSP/RTMP/HLS/DASH and many other different input/output protocols
* Firebase/Google/Azure cloud storages outputs **[PRO]**
* WEBRTC input to any output **[PRO]** [Demo](http://fastowebrtc.com)
* Any inputs to WEBRTC output **[PRO]**
* WebRTC-HTTP ingestion protocol (WHIP) **[PRO]**
* WebRTC-HTTP egress protocol (WHEP) **[PRO]**
* Probe stream **[PRO]**
* [REST API](https://fastogt.stoplight.io/docs/fastocloud-api/6dbac8c0c0a0f-fasto-cloud-community-pro-ml-versions)
* Adaptive hls streams
* Load balancing **[PRO]**
* Embedded urls
* HLS pull/push
* Audio to text conversions **[ML]**
* AD insertion **[PRO]**
* Logo overlay
* Concat videos **[PRO]**
* NDI Input/Output **[PRO]**
* Merging videos (Green screen, Webpage etc) **[PRO]**
* Video effects
* Relays audio/video part of stream
* Timeshifts
* Catchups
* Playlists
* Restream/Transcode from online streaming services like YouTube, Twitch, etc
* Mosaic
* CDN internal nodes **[PRO]**
* Physical Inputs (IP Camera, DVB-T/C/S, ATSC, USB, Screen, ISDB-T and DTMB)
* Presets
* Changing inputs in realtime **[PRO]**
* Scanning folders for media content **[PRO]**
* Channels on demand
* HTTP Live Streaming (HLS) server-side support
* Deep learning video analysis **[ML]**
* Supported deep learning frameworks: **[ML]**
  * Tensorflow
  * NCSDK 
  * Caffe
* ML Hardware: **[ML]**
  * Intel Movidius Neural Compute Stick
  * NVIDIA Jetson, Tesla T4
* ML notifications (Face detection, YOLO, Plate detection, etc) **[ML]**
* NVIDIA Maxine/Deepstream  **[ML]**
* Blacklist IP
* Tokens protected urls
* DRM **[PRO]**
* [Amazon Kinesis Video Streams integration](https://aws.amazon.com/kinesis/video-streams)
* [Admin panel](https://gitlab.com/fastogt/wsfastocloud)
* [Docker Image](https://hub.docker.com/r/fastogt/fastocloud)

### Use cases:
* Broadcast media content
* Video conferences
* Monetize Media content, own branded apps
* YouTube remove AD
* Insert your logo to the stream, watermark
* Record/Timeshift video
* Play own playlist
* Mosaic, view many streams in same time on single window
* Video effects in streams or files
* Insert AD into stream
* Subtitles to text conversions and vise versa
* Streams find bot, with quality analysis, test m3u file streams
* Record and broadcast video from cameras
* Detect objects in real-time video and send email/push notifications
* Objects classification, open doors, time tracking, lock/unlock devices
* Fire/Smoke detection with notifications
* Plate/Human/Face detection
* CCTV Monitoring service
* Screen recording
* Personal branded video apps
* Own video streaming platform on web, tablet and TV

Visit our site: [fastocloud.com](https://fastocloud.com)

[Admin panel](https://gitlab.com/fastogt/wsfastocloud)
==========
### Dashboard page:
![](https://gitlab.com/fastogt/wsfastocloud/raw/main/docs/images/fastocloud_one_web.png)
![](https://gitlab.com/fastogt/wsfastocloud/raw/main/docs/images/fastocloud_one_web_store_live.png)
![](https://gitlab.com/fastogt/wsfastocloud/raw/main/docs/images/fastocloud_one_web_store_vod.png)

[Demo Admin panel](https://ws.fastocloud.com) **Note: Press connect**

Contribute
==========
Contributions are always welcome! Just try to follow our coding style: [FastoGT Coding Style](https://github.com/fastogt/fastonosql/wiki/Coding-Style)

Build
========
Build documentation: [Build FastoCloud](https://github.com/fastogt/fastocloud_env/wiki/Build-service-from-sources)

Install
========
Install documentation: [Install FastoCloud](https://github.com/fastogt/fastocloud_env/wiki/Install-package)

License
=======

Copyright (C) 2014-2024 [FastoGT](https://fastogt.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.