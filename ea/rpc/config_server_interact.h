// Copyright 2023 The Elastic AI Search Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//


#pragma once

#include <butil/endpoint.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <brpc/controller.h>
#include <google/protobuf/descriptor.h>
#include "eaproto/ops/ops.interface.pb.h"
#include "ea/gflags/config.h"
#include "ea/base/tlog.h"

namespace EA::rpc {

    class ConfigServerInteract {
    public:
        static const int RETRY_TIMES = 5;

        static ConfigServerInteract *get_instance() {
            static ConfigServerInteract _instance;
            return &_instance;
        }

        ConfigServerInteract()  = default;

        bool is_inited() {
            return _is_inited;
        }

        int init(bool is_backup = false);

        int init_internal(const std::string &meta_bns);

        template<typename Request, typename Response>
        int send_request(const std::string &service_name,
                         const Request &request,
                         Response &response) {
            const ::google::protobuf::ServiceDescriptor *service_desc = proto::ConfigService::descriptor();
            const ::google::protobuf::MethodDescriptor *method =
                    service_desc->FindMethodByName(service_name);
            if (method == nullptr) {
                TLOG_ERROR("service name not exist, service:{}", service_name);
                return -1;
            }
            int retry_time = 0;
            uint64_t log_id = butil::fast_rand();
            do {
                if (retry_time > 0 && FLAGS_config_time_between_connect_error_ms > 0) {
                    bthread_usleep(1000 * FLAGS_config_time_between_connect_error_ms);
                }
                brpc::Controller cntl;
                cntl.set_log_id(log_id);
                std::unique_lock<std::mutex> lck(_master_leader_mutex);
                butil::EndPoint leader_address = _master_leader_address;
                lck.unlock();
                //store has leader address
                if (leader_address.ip != butil::IP_ANY) {
                    //construct short connection
                    brpc::ChannelOptions channel_opt;
                    channel_opt.timeout_ms = _request_timeout;
                    channel_opt.connect_timeout_ms = _connect_timeout;
                    brpc::Channel short_channel;
                    if (short_channel.Init(leader_address, &channel_opt) != 0) {
                        TLOG_WARN("connect with config server fail. channel Init fail, leader_addr:{}",
                                  butil::endpoint2str(leader_address).c_str());
                        _set_leader_address(butil::EndPoint());
                        ++retry_time;
                        continue;
                    }
                    short_channel.CallMethod(method, &cntl, &request, &response, nullptr);
                } else {
                    _bns_channel.CallMethod(method, &cntl, &request, &response, nullptr);
                    if (!cntl.Failed() && response.errcode() == proto::SUCCESS) {
                        _set_leader_address(cntl.remote_side());
                        TLOG_INFO("connect with config server success by bns name, leader:{}",
                                  butil::endpoint2str(cntl.remote_side()).c_str());
                        return 0;
                    }
                }

                TLOG_TRACE("config_req[{}], config_resp[{}]", request.ShortDebugString(), response.ShortDebugString());
                if (cntl.Failed()) {
                    TLOG_WARN("connect with server fail. send request fail, error:{}, log_id:{}",
                              cntl.ErrorText(), cntl.log_id());
                    _set_leader_address(butil::EndPoint());
                    ++retry_time;
                    continue;
                }
                if (response.errcode() == proto::HAVE_NOT_INIT) {
                    TLOG_WARN("connect with server fail. HAVE_NOT_INIT  log_id:{}", cntl.log_id());
                    _set_leader_address(butil::EndPoint());
                    ++retry_time;
                    continue;
                }
                if (response.errcode() == proto::NOT_LEADER) {
                    TLOG_WARN("connect with config server:{} fail. not leader, redirect to :{}, log_id:{}",
                              butil::endpoint2str(cntl.remote_side()).c_str(),
                              response.leader(), cntl.log_id());
                    butil::EndPoint leader_addr;
                    butil::str2endpoint(response.leader().c_str(), &leader_addr);
                    _set_leader_address(leader_addr);
                    ++retry_time;
                    continue;
                }
                if (response.errcode() != proto::SUCCESS) {
                    TLOG_WARN("send config server fail, log_id:{}, response:{}", cntl.log_id(),
                              response.ShortDebugString());
                    return -1;
                } else {
                    return 0;
                }
            } while (retry_time < RETRY_TIMES);
            return -1;
        }

        void _set_leader_address(const butil::EndPoint &addr) {
            std::unique_lock<std::mutex> lock(_master_leader_mutex);
            _master_leader_address = addr;
        }

    private:
        brpc::Channel _bns_channel;
        int32_t _request_timeout = 30000;
        int32_t _connect_timeout = 5000;
        bool _is_inited = false;
        std::mutex _master_leader_mutex;
        butil::EndPoint _master_leader_address;
    };
}  // namespace EA::rpc
