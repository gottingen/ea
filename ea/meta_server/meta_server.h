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

#include <braft/raft.h>
#include "eapi/servlet/servlet.interface.pb.h"
#include "ea/base/bthread.h"

namespace EA::servlet {
    class MetaStateMachine;

    class AutoIncrStateMachine;

    class TSOStateMachine;

    class MetaServer : public EA::servlet::MetaService {
    public:
        ~MetaServer() override;

        static MetaServer *get_instance() {
            static MetaServer _instance;
            return &_instance;
        }

        int init(const std::vector<braft::PeerId> &peers);

        //schema control method
        void meta_manager(google::protobuf::RpcController *controller,
                                  const EA::servlet::MetaManagerRequest *request,
                                  EA::servlet::MetaManagerResponse *response,
                                  google::protobuf::Closure *done) override;

        void meta_query(google::protobuf::RpcController *controller,
                           const EA::servlet::QueryRequest *request,
                           EA::servlet::QueryResponse *response,
                           google::protobuf::Closure *done) override;

        //raft control method
        void raft_control(google::protobuf::RpcController *controller,
                                  const EA::servlet::RaftControlRequest *request,
                                  EA::servlet::RaftControlResponse *response,
                                  google::protobuf::Closure *done) override;


        void tso_service(google::protobuf::RpcController *controller,
                                 const EA::servlet::TsoRequest *request,
                                 EA::servlet::TsoResponse *response,
                                 google::protobuf::Closure *done) override;


        void flush_memtable_thread();

        void shutdown_raft();

        bool have_data();

        void close();

    private:
        MetaServer() {}

        bthread::Mutex _meta_interact_mutex;
        MetaStateMachine *_meta_state_machine = nullptr;
        AutoIncrStateMachine *_auto_incr_state_machine = nullptr;
        TSOStateMachine *_tso_state_machine = nullptr;
        Bthread _flush_bth;
        bool _init_success = false;
        bool _shutdown = false;
    }; //class

}  // namespace EA::servlet
