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

#ifndef EA_DICT_DICT_SERVER_H_
#define EA_DICT_DICT_SERVER_H_

#include "eaproto/ops/ops.interface.pb.h"
#include "brpc/closure_guard.h"
#include <braft/raft.h>

namespace EA::dict {

    class DictStateMachine;

    class DictServer : public EA::proto::DictService {
    public:
        static DictServer* get_instance() {
            static DictServer ins;
            return &ins;
        }

        void dict_manage(::google::protobuf::RpcController *controller,
                     const ::EA::proto::OpsServiceRequest *request,
                     ::EA::proto::OpsServiceResponse *response,
                     ::google::protobuf::Closure *done) override;

        void dict_query(::google::protobuf::RpcController *controller,
                        const ::EA::proto::QueryOpsServiceRequest *request,
                        ::EA::proto::QueryOpsServiceResponse *response,
                        ::google::protobuf::Closure *done) override;

        int init(const std::vector<braft::PeerId> &peers);

        bool have_data();

        void shutdown_raft();

        void close();

    private:
        DictStateMachine *_machine;
    };

}  // namespace EA::dict

#endif  // EA_DICT_DICT_SERVER_H_
