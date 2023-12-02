// Copyright (c) 2020 Baidu, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "eapi/discovery/discovery.interface.pb.h"
#include "ea/discovery/router_service.h"
#include "ea/base/bthread.h"

namespace EA::discovery {

    turbo::Status RouterServiceImpl::init(const std::string &discovery_peers) {
        if(_is_init) {
            return  turbo::OkStatus();
        }
        auto rs = _manager_sender.init(discovery_peers);
        if(!rs.ok()) {
            return rs;
        }
        rs = _query_sender.init(discovery_peers);
        if(!rs.ok()) {
            return rs;
        }
        _is_init = true;
        return turbo::OkStatus();
    }
    void RouterServiceImpl::discovery_manager(::google::protobuf::RpcController* controller,
                      const ::EA::discovery::DiscoveryManagerRequest* request,
                      ::EA::discovery::DiscoveryManagerResponse* response,
                      ::google::protobuf::Closure* done) {

        auto rpc_discovery_func = [controller, request,response, done,this](){
            brpc::ClosureGuard done_guard(done);
            auto ret = _manager_sender.discovery_manager(*request, *response, 2);
            if(!ret.ok()) {
                TLOG_ERROR("rpc to discovery server:discovery_manager error:{}", controller->ErrorText());
            }
        };
        EA::Bthread bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

    void RouterServiceImpl::discovery_query(::google::protobuf::RpcController* controller,
               const ::EA::discovery::DiscoveryQueryRequest* request,
               ::EA::discovery::DiscoveryQueryResponse* response,
               ::google::protobuf::Closure* done) {
        auto rpc_discovery_func = [controller, request,response, done, this](){
            brpc::ClosureGuard done_guard(done);
            auto ret = _query_sender.discovery_query(*request, *response, 2);
            if(!ret.ok()) {
                TLOG_ERROR("rpc to discovery server:discovery_manager error:{}", controller->ErrorText());
            }
        };
        EA::Bthread bth;
        bth.run(rpc_discovery_func);
        bth.join();
    }

}  // namespace EA::discovery
