// Copyright 2023 The Elastic-AI Authors.
// part of Elastic AI Search
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

#ifndef EA_META_ROUTER_SERVICE_H_
#define EA_META_ROUTER_SERVICE_H_

#include "ea/client/meta_sender.h"

namespace EA::servlet {

    class RouterServiceImpl : public EA::servlet::RouterService {
    public:

        static RouterServiceImpl* get_instance() {
            static RouterServiceImpl ins;
            return &ins;
        }

        turbo::Status init(const std::string &meta_peers);

        ~RouterServiceImpl()  = default;

        void meta_manager(::google::protobuf::RpcController *controller,
                          const ::EA::servlet::MetaManagerRequest *request,
                          ::EA::servlet::MetaManagerResponse *response,
                          ::google::protobuf::Closure *done) override;

        void meta_query(::google::protobuf::RpcController *controller,
                        const ::EA::servlet::QueryRequest *request,
                        ::EA::servlet::QueryResponse *response,
                        ::google::protobuf::Closure *done) override;

    private:
        bool _is_init;
        EA::client::MetaSender _manager_sender;
        EA::client::MetaSender _query_sender;
    };

}  // namespace EA::servlet
#endif  // EA_META_ROUTER_SERVICE_H_
