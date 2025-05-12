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

#include <fastotv/commands_info/machine_info.h>
#include <fastotv/commands_info/project_info.h>

#include <common/uri/gurl.h>

#include <string>

namespace fastocloud {
namespace server {
namespace service {

class OnlineUsers : public common::serializer::JsonSerializer<OnlineUsers> {
 public:
  typedef JsonSerializer<OnlineUsers> base_class;
  OnlineUsers();
  explicit OnlineUsers(size_t daemon, size_t http, size_t vods, size_t cods);

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* out) const override;

 private:
  size_t daemon_;
  size_t http_;
  size_t vods_;
  size_t cods_;
};

class ServerInfo : public fastotv::commands_info::MachineInfo {
 public:
  typedef fastotv::commands_info::MachineInfo base_class;
  ServerInfo();
  ServerInfo(cpu_load_t cpu_load,
             gpu_load_t gpu_load,
             const std::string& load_average,
             size_t ram_bytes_total,
             size_t ram_bytes_free,
             size_t hdd_bytes_total,
             size_t hdd_bytes_free,
             fastotv::bandwidth_t net_bytes_recv,
             fastotv::bandwidth_t net_bytes_send,
             time_t uptime,
             fastotv::timestamp_t timestamp,
             const OnlineUsers& online_users,
             size_t net_total_bytes_recv,
             size_t net_total_bytes_send,
             cost_t cost);

  OnlineUsers GetOnlineUsers() const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* out) const override;

 private:
  OnlineUsers online_users_;
};

class BalanceInfo : public common::serializer::JsonSerializer<BalanceInfo> {
 public:
  typedef JsonSerializer<BalanceInfo> base_class;
  typedef ServerInfo::cost_t balance_t;
  typedef ServerInfo::cost_t speed_t;

  BalanceInfo();
  BalanceInfo(balance_t balance, speed_t speed);

  balance_t GetBalance() const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* out) const override;

 private:
  balance_t balance_;
  speed_t speed_;
};

class FullServiceInfo : public ServerInfo {
 public:
  typedef ServerInfo base_class;
  typedef common::uri::GURL host_t;

  FullServiceInfo();
  explicit FullServiceInfo(const host_t& http_host,
                           const host_t& vods_host,
                           const host_t& cods_host,
                           common::time64_t exp_time,
                           const std::string& hls_dir,
                           const std::string& vods_dir,
                           const std::string& cods_dir,
                           const std::string& timeshifts_dir,
                           const std::string& feedback_dir,
                           const std::string& proxy_dir,
                           const std::string& data_dir,
                           const base_class& base);

  host_t GetHttpHost() const;
  host_t GetVodsHost() const;
  host_t GetCodsHost() const;

  fastotv::commands_info::LicenseProjectInfo GetProject() const;

  std::string GetVsystem() const;
  std::string GetVrole() const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* out) const override;

 private:
  host_t http_host_;
  host_t vods_host_;
  host_t cods_host_;

  std::string hls_dir_;
  std::string vods_dir_;
  std::string cods_dir_;
  std::string timeshifts_dir_;
  std::string feedback_dir_;
  std::string proxy_dir_;
  std::string data_dir_;

  fastotv::commands_info::LicenseProjectInfo project_;
  fastotv::commands_info::OperationSystemInfo os_;

  std::string vsystem_;
  std::string vrole_;
};

}  // namespace service
}  // namespace server
}  // namespace fastocloud
