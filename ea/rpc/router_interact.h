// Copyright 2023 The Turbo Authors.
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
#ifndef EA_RPC_ROUTER_INTERACT_H_
#define EA_RPC_ROUTER_INTERACT_H_

#include <brpc/channel.h>
#include "turbo/base/status.h"
#include "ea/base/tlog.h"
#include <butil/endpoint.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <brpc/controller.h>
#include <google/protobuf/descriptor.h>
#include "ea/cli/option_context.h"
#include "eaproto/router/router.interface.pb.h"

namespace EA::client {

    class RouterInteract {
    public:
        static RouterInteract *get_instance() {
            static RouterInteract ins;
            return &ins;
        }

        template<typename Request, typename Response>
        turbo::Status send_request(const std::string &service_name,
                                   const Request &request,
                                   Response &response) {
            const ::google::protobuf::ServiceDescriptor *service_desc = proto::RouterService::descriptor();
            const ::google::protobuf::MethodDescriptor *method =
                    service_desc->FindMethodByName(service_name);
            auto verbose =  OptionContext::get_instance()->verbose;
            if (method == nullptr) {
                TLOG_ERROR_IF(verbose, "service name not exist, service:{}", service_name);
                return turbo::InvalidArgumentError("service name not exist, service:{}", service_name);
            }
            int retry_time = 0;
            uint64_t log_id = butil::fast_rand();
            do {
                if (retry_time > 0 && OptionContext::get_instance()->max_retry > 0) {
                    bthread_usleep(1000 * OptionContext::get_instance()->time_between_meta_connect_error_ms);
                }
                brpc::Controller cntl;
                cntl.set_log_id(log_id);
                //store has leader address
                brpc::ChannelOptions channel_opt;
                channel_opt.timeout_ms =  OptionContext::get_instance()->timeout_ms;
                channel_opt.connect_timeout_ms = OptionContext::get_instance()->connect_timeout_ms;
                brpc::Channel short_channel;
                if (short_channel.Init(OptionContext::get_instance()->server.c_str(), &channel_opt) != 0) {
                    TLOG_WARN_IF(verbose, "connect with router server fail. channel Init fail, leader_addr:{}", OptionContext::get_instance()->server);
                    ++retry_time;
                    continue;
                }
                short_channel.CallMethod(method, &cntl, &request, &response, nullptr);

                TLOG_TRACE_IF(verbose, "router_req[{}], router_resp[{}]", request.ShortDebugString(), response.ShortDebugString());
                if (cntl.Failed()) {
                    TLOG_WARN_IF(verbose, "connect with router server fail. send request fail, error:{}, log_id:{}",
                              cntl.ErrorText(), cntl.log_id());
                    ++retry_time;
                    continue;
                }

                if (response.errcode() != proto::SUCCESS) {
                    TLOG_WARN_IF(verbose, "send meta server fail, log_id:{}, response:{}", cntl.log_id(),
                              response.ShortDebugString());
                    //return turbo::UnavailableError("send meta server fail, log_id:{}, response:{}", cntl.log_id(),
                    //                               response.ShortDebugString());
                    return turbo::OkStatus();
                } else {
                    return turbo::OkStatus();
                }
            } while (retry_time < OptionContext::get_instance()->max_retry);
            return turbo::DeadlineExceededError("try times {} and can not get response.", retry_time);

        }

    };
}  // namespace EA::client

#endif  // EA_RPC_ROUTER_INTERACT_H_
