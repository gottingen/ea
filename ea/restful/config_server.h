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


#ifndef EA_RESTFUL_CONFIG_SERVER_H_
#define EA_RESTFUL_CONFIG_SERVER_H_

#include "eaproto/ops/config.restful.pb.h"

namespace EA::restful {

    class ConfigServer : public EA::proto::ConfigRestfulService {
    public:
        void create_config(::google::protobuf::RpcController* controller,
                                   const ::EA::proto::ConfigEntity* request,
                                   ::EA::proto::ConfigRestfulResponse* response,
                                   ::google::protobuf::Closure* done) override;
        void remove_config(::google::protobuf::RpcController* controller,
                                   const ::EA::proto::ConfigEmptyRequest* request,
                                   ::EA::proto::ConfigRestfulResponse* response,
                                   ::google::protobuf::Closure* done) override;
        void get_config(::google::protobuf::RpcController* controller,
                                const ::EA::proto::ConfigEmptyRequest* request,
                                ::EA::proto::ConfigRestfulResponse* response,
                                ::google::protobuf::Closure* done) override;
        void get_config_list(::google::protobuf::RpcController* controller,
                                     const ::EA::proto::ConfigEmptyRequest* request,
                                     ::EA::proto::ConfigRestfulResponse* response,
                                     ::google::protobuf::Closure* done) override;
        void get_config_version_list(::google::protobuf::RpcController* controller,
                                             const ::EA::proto::ConfigEmptyRequest* request,
                                             ::EA::proto::ConfigRestfulResponse* response,
                                             ::google::protobuf::Closure* done) override;
    };

}  // namespace EA::restful
#endif  // EA_RESTFUL_CONFIG_SERVER_H_
