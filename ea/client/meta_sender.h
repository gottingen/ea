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
#include "eapi/servlet/servlet.interface.pb.h"
#include "ea/base/tlog.h"
#include "ea/client/base_message_sender.h"

namespace EA::client {

    /**
     * @ingroup meta_client
     * @brief MetaSender is used to send messages to the meta server.
     */
    class MetaSender : public BaseMessageSender {
    public:
        static const int kRetryTimes = 5;

        /**
         * @brief get_instance is used to get the singleton instance of MetaSender.
         * @return
         */
        static MetaSender *get_instance() {
            static MetaSender _instance;
            return &_instance;
        }

        /**
         * @brief get_backup_instance is used to get the singleton instance of MetaSender.
         * @return
         */
        static MetaSender *get_backup_instance() {
            static MetaSender _instance;
            return &_instance;
        }

        MetaSender() = default;

        /**
         * @brief is_inited is used to check if the MetaSender is initialized.
         * @return
         */
        bool is_inited() {
            return _is_inited;
        }

        /**
         * @brief init is used to initialize the MetaSender. It must be called before using the MetaSender.
         * @param raft_nodes [input] is the raft nodes of the meta server.
         * @return Status::OK if the MetaSender was initialized successfully. Otherwise, an error status is returned. 
         */
        turbo::Status init(const std::string &raft_nodes);

        /**
         * @brief init is used to initialize the MetaSender. It can be called any time.
         * @param verbose [input] is the verbose flag.
         * @param meta_raft_group [input] is the raft group of the meta server.
         * @return MetaSender itself.
         */
        MetaSender &set_verbose(bool verbose);

        /**
         * @brief set_time_out is used to set the timeout for sending a request to the meta server.
         * @param time_ms [input] is the timeout in milliseconds for sending a request to the meta server.
         * @return MetaSender itself.
         */
        MetaSender &set_time_out(int time_ms);


        /**
         * @brief set_connect_time_out is used to set the timeout for connecting to the meta server.
         * @param time_ms [input] is the timeout in milliseconds for connecting to the meta server.
         * @return MetaSender itself.
         */
        MetaSender &set_connect_time_out(int time_ms);

        /**
         * @brief set_interval_time is used to set the interval time for retrying to send a request to the meta server.
         * @param time_ms [input] is the interval time in milliseconds for retrying to send a request to the meta server.
         * @return MetaSender itself.
         */
        MetaSender &set_interval_time(int time_ms);

        /**
         * @brief set_retry_time is used to set the number of times to retry sending a request to the meta server.
         * @param retry [input] is the number of times to retry sending a request to the meta server.
         * @return MetaSender itself.
         */
        MetaSender &set_retry_time(int retry);

        /**
         * @brief get_leader is used to get the leader address of the meta server.
         * @return the leader address of the meta server.
         */
        std::string get_leader() const;

        /**
         * @brief meta_manager is used to send a MetaManagerRequest to the meta server for management.
         * @param request [input] is the MetaManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @param retry_time [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                   EA::servlet::MetaManagerResponse &response, int retry_time) override;

        /**
         * @brief meta_manager is used to send a MetaManagerRequest to the meta server for management.
         * @param request [input] is the MetaManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                   EA::servlet::MetaManagerResponse &response) override;

        /**
         * @brief meta_query is used to send a QueryRequest to the meta server for querying.
         * @param request [input] is the QueryRequest to send.
         * @param response [output] is the QueryResponse received from the meta server.
         * @param retry_time [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                 EA::servlet::QueryResponse &response, int retry_time) override;


        /**
         * @brief meta_query is used to send a QueryRequest to the meta server for querying.
         * @param request [input] is the QueryRequest to send.
         * @param response [output] is the QueryResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                 EA::servlet::QueryResponse &response) override;

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

    private:

        /**
         *
         * @param addr
         */
        void set_leader_address(const butil::EndPoint &addr);

    private:
        std::string _meta_raft_group;
        std::string _meta_nodes;
        std::vector<butil::EndPoint> _servlet_nodes;
        int32_t _request_timeout = 30000;
        int32_t _connect_timeout = 5000;
        bool _is_inited{false};
        std::mutex _master_leader_mutex;
        butil::EndPoint _master_leader_address;
        int _between_meta_connect_error_ms{1000};
        int _retry_times{kRetryTimes};
        bool _verbose{false};
    };

    template<typename Request, typename Response>
    inline turbo::Status MetaSender::send_request(const std::string &service_name,
                                                  const Request &request,
                                                  Response &response, int retry_times) {
        const ::google::protobuf::ServiceDescriptor *service_desc = EA::servlet::MetaService::descriptor();
        const ::google::protobuf::MethodDescriptor *method =
                service_desc->FindMethodByName(service_name);
        if (method == nullptr) {
            TLOG_ERROR_IF(_verbose, "service name not exist, service:{}", service_name);
            return turbo::UnavailableError("service name not exist, service:{}", service_name);
        }
        int retry_time = 0;
        bool is_select_leader{false};
        uint64_t log_id = butil::fast_rand();
        do {
            if (!is_select_leader && retry_time > 0 && _between_meta_connect_error_ms > 0) {
                bthread_usleep(1000 * _between_meta_connect_error_ms);
            }
            brpc::Controller cntl;
            cntl.set_log_id(log_id);
            std::unique_lock<std::mutex> lck(_master_leader_mutex);
            butil::EndPoint leader_address = _master_leader_address;
            lck.unlock();
            brpc::ChannelOptions channel_opt;
            channel_opt.timeout_ms = _request_timeout;
            channel_opt.connect_timeout_ms = _connect_timeout;
            brpc::Channel short_channel;
            is_select_leader = leader_address.ip == butil::IP_ANY;
            //store has leader address
            if (is_select_leader) {
                TLOG_INFO_IF(_verbose, "master address null, select leader first");
                auto seed = butil::fast_rand() % _servlet_nodes.size();
                leader_address = _servlet_nodes[seed];
            }
            TLOG_INFO_IF(_verbose & !is_select_leader, "master address:{}",
                         butil::endpoint2str(_master_leader_address).c_str());
            if (short_channel.Init(leader_address, &channel_opt) != 0) {
                TLOG_ERROR_IF(_verbose, "connect with meta server fail. channel Init fail, leader_addr:{}",
                              butil::endpoint2str(leader_address).c_str());
                set_leader_address(butil::EndPoint());
                ++retry_time;
                continue;
            }
            short_channel.CallMethod(method, &cntl, &request, &response, nullptr);

            TLOG_INFO_IF(_verbose, "meta_req[{}], meta_resp[{}]", request.ShortDebugString(),
                         response.ShortDebugString());
            if (cntl.Failed()) {
                TLOG_WARN_IF(_verbose, "connect with server fail. send request fail, error:{}, log_id:{}",
                             cntl.ErrorText(), cntl.log_id());
                set_leader_address(butil::EndPoint());
                ++retry_time;
                continue;
            }
            if (response.errcode() == EA::servlet::HAVE_NOT_INIT) {
                TLOG_WARN_IF(_verbose, "connect with server fail. HAVE_NOT_INIT  log_id:{}", cntl.log_id());
                set_leader_address(butil::EndPoint());
                ++retry_time;
                continue;
            }
            if (response.errcode() == EA::servlet::NOT_LEADER) {
                TLOG_WARN_IF(_verbose, "connect with meta server:{} fail. not leader, redirect to :{}, log_id:{}",
                             butil::endpoint2str(cntl.remote_side()).c_str(),
                             response.leader(), cntl.log_id());
                butil::EndPoint leader_addr;
                butil::str2endpoint(response.leader().c_str(), &leader_addr);
                set_leader_address(leader_addr);
                // select leader do not cost retry times
                ++retry_time;
                continue;
            }
            /// success, The node being tried happens to be leader
            if (_master_leader_address.ip == butil::IP_ANY && leader_address.ip != butil::IP_ANY) {
                TLOG_INFO_IF(_verbose, "set leader ip:{}, log_id:{}",
                            butil::endpoint2str(leader_address).c_str(),cntl.log_id());
                set_leader_address(leader_address);
            }
            return turbo::OkStatus();
        } while (retry_time < retry_times);
        return turbo::UnavailableError("can not connect server after {} times try", retry_times);
    }


}  // namespace EA::client
