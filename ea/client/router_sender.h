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
#ifndef EA_CLIENT_ROUTER_SENDER_H_
#define EA_CLIENT_ROUTER_SENDER_H_

#include <brpc/channel.h>
#include "turbo/base/status.h"
#include "ea/base/tlog.h"
#include <butil/endpoint.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <brpc/controller.h>
#include <google/protobuf/descriptor.h>
#include "ea/cli/option_context.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include "ea/client/base_message_sender.h"

namespace EA::client {

    class RouterSender : public BaseMessageSender {
    public:
        ///
        /// \return
        static RouterSender *get_instance() {
            static RouterSender ins;
            return &ins;
        }

        static const int kRetryTimes = 3;
        ///
        /// \param server
        /// \param verbose
        /// \param retry_times
        /// \return
        turbo::Status init(const std::string &server);

        ///
        /// \param server
        /// \return
        RouterSender &set_server(const std::string &server);

        ///
        /// \param verbose
        /// \return
        RouterSender &set_verbose(bool verbose);

        ///
        /// \param time_ms
        /// \return
        RouterSender &set_time_out(int time_ms);

        ///
        /// \param time_ms
        /// \return
        RouterSender &set_connect_time_out(int time_ms);

        ///
        /// \param time_ms
        /// \return
        RouterSender &set_interval_time(int time_ms);

        ///
        /// \param retry
        /// \return
        RouterSender &set_retry_time(int retry);

        ///
        /// \tparam Request
        /// \tparam Response
        /// \param service_name
        /// \param request
        /// \param response
        /// \param retry_times
        /// \return
        template<typename Request, typename Response>
        turbo::Status send_request(const std::string &service_name,
                                   const Request &request,
                                   Response &response, int retry_times);

        ///
        /// \param request
        /// \param response
        /// \param retry_times
        /// \return
        turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                   EA::servlet::MetaManagerResponse &response, int retry_time) override;

        ///
        /// \param request
        /// \param response
        /// \return
        turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                   EA::servlet::MetaManagerResponse &response) override;

        ///
        /// \param request
        /// \param response
        /// \param retry_times
        /// \return
        turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                 EA::servlet::QueryResponse &response, int retry_time) override;

        ///
        /// \param request
        /// \param response
        /// \return
        turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                 EA::servlet::QueryResponse &response) override;

    private:
        std::mutex _server_mutex;
        bool _verbose{false};
        int _retry_times{kRetryTimes};
        std::string _server;
        int _timeout_ms{300};
        int _connect_timeout_ms{500};
        int _between_meta_connect_error_ms{1000};
    };

    template<typename Request, typename Response>
    turbo::Status RouterSender::send_request(const std::string &service_name,
                                             const Request &request,
                                             Response &response, int retry_times) {
        const ::google::protobuf::ServiceDescriptor *service_desc = EA::servlet::RouterService::descriptor();
        const ::google::protobuf::MethodDescriptor *method =
                service_desc->FindMethodByName(service_name);
        if (method == nullptr) {
            TLOG_ERROR_IF(_verbose, "service name not exist, service:{}", service_name);
            return turbo::InvalidArgumentError("service name not exist, service:{}", service_name);
        }
        int retry_time = 0;
        uint64_t log_id = butil::fast_rand();
        do {
            if (retry_time > 0 && retry_times > 0) {
                bthread_usleep(1000 * _between_meta_connect_error_ms);
            }
            brpc::Controller cntl;
            cntl.set_log_id(log_id);
            //store has leader address
            brpc::ChannelOptions channel_opt;
            channel_opt.timeout_ms = _timeout_ms;
            channel_opt.connect_timeout_ms = _connect_timeout_ms;
            brpc::Channel short_channel;
            if (short_channel.Init(_server.c_str(), &channel_opt) != 0) {
                TLOG_WARN_IF(_verbose, "connect with router server fail. channel Init fail, leader_addr:{}", _server);
                ++retry_time;
                continue;
            }
            short_channel.CallMethod(method, &cntl, &request, &response, nullptr);

            TLOG_TRACE_IF(_verbose, "router_req[{}], router_resp[{}]", request.ShortDebugString(),
                          response.ShortDebugString());
            if (cntl.Failed()) {
                TLOG_WARN_IF(_verbose, "connect with router server fail. send request fail, error:{}, log_id:{}",
                             cntl.ErrorText(), cntl.log_id());
                ++retry_time;
                continue;
            }
            return turbo::OkStatus();
            /*
            if (response.errcode() != EA::servlet::SUCCESS) {
                TLOG_WARN_IF(_verbose, "send meta server fail, log_id:{}, response:{}", cntl.log_id(),
                             response.ShortDebugString());
                //return turbo::UnavailableError("send meta server fail, log_id:{}, response:{}", cntl.log_id(),
                //                               response.ShortDebugString());
                return turbo::OkStatus();
            } else {
                return turbo::OkStatus();
            }*/
        } while (retry_time < retry_times);
        return turbo::DeadlineExceededError("try times {} reach max_try {} and can not get response.", retry_time,
                                            retry_times);

    }

}  // namespace EA::client

#endif  // EA_CLIENT_ROUTER_SENDER_H_