// Copyright 2023 The Elastic Architecture Infrastructure Authors.
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

//
// Created by jeff on 23-11-29.
//

#ifndef EA_DISCOVERY_ROUTER_SERVICE_H_
#define EA_DISCOVERY_ROUTER_SERVICE_H_

#include "ea/client/discovery_sender.h"

namespace EA::discovery {

    class RouterServiceImpl : public EA::discovery::DiscoveryRouterService {
    public:

        static RouterServiceImpl* get_instance() {
            static RouterServiceImpl ins;
            return &ins;
        }

        turbo::Status init(const std::string &discovery_peers);

        ~RouterServiceImpl()  = default;

        void discovery_manager(::google::protobuf::RpcController *controller,
                          const ::EA::discovery::DiscoveryManagerRequest *request,
                          ::EA::discovery::DiscoveryManagerResponse *response,
                          ::google::protobuf::Closure *done) override;

        void discovery_query(::google::protobuf::RpcController *controller,
                        const ::EA::discovery::DiscoveryQueryRequest *request,
                        ::EA::discovery::DiscoveryQueryResponse *response,
                        ::google::protobuf::Closure *done) override;

    private:
        bool _is_init;
        EA::client::DiscoverySender _manager_sender;
        EA::client::DiscoverySender _query_sender;
    };

}  // namespace EA::discovery
#endif  // EA_DISCOVERY_ROUTER_SERVICE_H_
