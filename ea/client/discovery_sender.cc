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


#include "ea/client/discovery_sender.h"
#include <braft/route_table.h>
#include <braft/raft.h>
#include <braft/util.h>

namespace EA::client {


    turbo::Status DiscoverySender::init(const std::string & raft_nodes) {
        _master_leader_address.ip = butil::IP_ANY;
        std::vector<std::string> peers = turbo::str_split(raft_nodes, turbo::by_any_char(",;\t\n "));
        for (auto &peer : peers) {
            butil::EndPoint end_point;
            if(butil::str2endpoint(peer.c_str(), &end_point) != 0) {
                return turbo::invalid_argument_error("invalid address {}", peer);
            }
            _discovery_nodes.push_back(end_point);
        }
        return turbo::ok_status();
    }


    std::string DiscoverySender::get_leader() const {
        TLOG_INFO_IF(_verbose, "get master address:{}", butil::endpoint2str(_master_leader_address).c_str());
        return butil::endpoint2str(_master_leader_address).c_str();
    }

    void DiscoverySender::set_leader_address(const butil::EndPoint &addr) {
        std::unique_lock<std::mutex> lock(_master_leader_mutex);
        _master_leader_address = addr;
        TLOG_INFO_IF(_verbose, "set master address:{}", butil::endpoint2str(_master_leader_address).c_str());
    }

    turbo::Status DiscoverySender::discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                           EA::discovery::DiscoveryManagerResponse &response, int retry_times) {
        return send_request("discovery_manager", request, response, retry_times);
    }

    turbo::Status DiscoverySender::discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                           EA::discovery::DiscoveryManagerResponse &response) {
        return send_request("discovery_manager", request, response, _retry_times);
    }

    turbo::Status DiscoverySender::discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                         EA::discovery::DiscoveryQueryResponse &response, int retry_times) {
        return send_request("discovery_query", request, response, retry_times);
    }

    turbo::Status DiscoverySender::discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                         EA::discovery::DiscoveryQueryResponse &response) {
        return send_request("discovery_query", request, response, _retry_times);
    }


    DiscoverySender &DiscoverySender::set_verbose(bool verbose) {
        _verbose = verbose;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_time_out(int time_ms) {
        _request_timeout = time_ms;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_connect_time_out(int time_ms) {
        _connect_timeout = time_ms;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_interval_time(int time_ms) {
        _between_discovery_connect_error_ms = time_ms;
        return *this;
    }

    DiscoverySender &DiscoverySender::set_retry_time(int retry) {
        _retry_times = retry;
        return *this;
    }

}  // EA::client

