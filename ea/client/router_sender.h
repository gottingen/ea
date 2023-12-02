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
#include "eapi/discovery/discovery.interface.pb.h"
#include "ea/client/base_message_sender.h"

namespace EA::client {

    /**
     * @ingroup ea_rpc
     * @brief RouterSender is used to send messages to the meta server.
     *        It is used by the DiscoveryClient to send messages to the router server.
     *        It do not need to be judge the leader, because the router server will
     *        do it.
     * @code
     *       auto router_sender = RouterSender::get_instance();
     *       auto rs = router_sender->init("127.0.0.1:8888");
     *       if(!rs.ok()) {
     *          TLOG_ERROR("init router sender fail, error:{}", rs.message());
     *          return -1;
     *       }
     *       EA::discovery::DiscoveryManagerRequest request;
     *       EA::discovery::DiscoveryManagerResponse response;
     *       auto status = router_sender->discovery_manager(request, response);
     *       if(!status.ok()) {
     *          TLOG_ERROR("send meta manager request fail, error:{}", status.message());
     *          return -1;
     *       }
     *       ...
     *       return 0;
     *@endcode
     *
     */
    class RouterSender : public BaseMessageSender {
    public:

        /**
         * @brief get_instance is used to get the singleton instance of the RouterSender.
         * @return the singleton instance of the RouterSender.
         */
        static RouterSender *get_instance() {
            static RouterSender ins;
            return &ins;
        }

    

        static const int kRetryTimes = 3;

        /**
         * @brief init is used to initialize the RouterSender. It must be called before using the RouterSender.
         * @param server [input] are the addresses of the meta servers, separated by commas.
         * @return Status::OK if the RouterSender was initialized successfully. Otherwise, an error status is returned. 
         */
        turbo::Status init(const std::string &server);

        /**
         * @brief set_server is used to set the addresses of the meta servers, must be called before using the RouterSender.
         * @param server [input] are the addresses of the meta servers, separated by commas.
         * @return the RouterSender.
         */
        RouterSender &set_server(const std::string &server);

        /**
         * @brief set_verbose is used to set the verbose flag of the RouterSender.
         * @param verbose [input] is the verbose flag of the RouterSender.
         * @return the RouterSender.
         */
        RouterSender &set_verbose(bool verbose);

        /**
         * @brief set_time_out is used to set the timeout of the RouterSender.
         * @param time_ms [input] is the timeout of the RouterSender.
         * @return the RouterSender.
         */
        RouterSender &set_time_out(int time_ms);

        /**
         * @brief set_connect_time_out is used to set the connect timeout of the RouterSender.
         * @param time_ms [input] is the connect timeout of the RouterSender.
         * @return the RouterSender.
         */
        RouterSender &set_connect_time_out(int time_ms);

        /**
         * @brief set_interval_time is used to set the interval time of the RouterSender.
         * @param time_ms [input] is the interval time of the RouterSender.
         * @return the RouterSender.
         */
        RouterSender &set_interval_time(int time_ms);

        /**
         * @brief set_retry_time is used to set the retry time of the RouterSender.
         * @param retry [input] is the retry time of the RouterSender.
         * @return the RouterSender.
         */
        RouterSender &set_retry_time(int retry);

        /**
         * @brief send_request is used to send a request to the meta server.
         * @param service_name [input] is the name of the service to send the request to.
         * @param request [input] is the request to send.
         * @param response [output] is the response received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        template<typename Request, typename Response>
        turbo::Status send_request(const std::string &service_name,
                                   const Request &request,
                                   Response &response, int retry_times);

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @param retry_time [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                   EA::discovery::DiscoveryManagerResponse &response, int retry_time) override;

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                   EA::discovery::DiscoveryManagerResponse &response) override;

        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @param retry_time [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                 EA::discovery::DiscoveryQueryResponse &response, int retry_time) override;

        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                 EA::discovery::DiscoveryQueryResponse &response) override;

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
        const ::google::protobuf::ServiceDescriptor *service_desc = EA::discovery::DiscoveryRouterService::descriptor();
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
            if (response.errcode() != EA::discovery::SUCCESS) {
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
