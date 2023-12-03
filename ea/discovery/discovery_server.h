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
#include "eapi/discovery/discovery.interface.pb.h"
#include "ea/base/bthread.h"

namespace EA::discovery {

    class DiscoveryStateMachine;

    class AutoIncrStateMachine;

    class TSOStateMachine;

    class DiscoveryServer : public EA::discovery::DiscoveryService {
    public:
        ~DiscoveryServer() override;

        static DiscoveryServer *get_instance() {
            static DiscoveryServer _instance;
            return &_instance;
        }

        int init(const std::vector<braft::PeerId> &peers);

        //schema control method
        void discovery_manager(google::protobuf::RpcController *controller,
                                  const EA::discovery::DiscoveryManagerRequest *request,
                                  EA::discovery::DiscoveryManagerResponse *response,
                                  google::protobuf::Closure *done) override;

        void discovery_query(google::protobuf::RpcController *controller,
                           const EA::discovery::DiscoveryQueryRequest *request,
                           EA::discovery::DiscoveryQueryResponse *response,
                           google::protobuf::Closure *done) override;

        //raft control method
        void raft_control(google::protobuf::RpcController *controller,
                                  const EA::RaftControlRequest *request,
                                  EA::RaftControlResponse *response,
                                  google::protobuf::Closure *done) override;


        void tso_service(google::protobuf::RpcController *controller,
                                 const EA::discovery::TsoRequest *request,
                                 EA::discovery::TsoResponse *response,
                                 google::protobuf::Closure *done) override;


        void flush_memtable_thread();

        void shutdown_raft();

        bool have_data();

        void close();

    private:
        DiscoveryServer() {}

        bthread::Mutex discovery_nteract_mutex;
        DiscoveryStateMachine *_discovery_state_machine = nullptr;
        AutoIncrStateMachine *_auto_incr_state_machine = nullptr;
        TSOStateMachine *_tso_state_machine = nullptr;
        Bthread _flush_bth;
        bool _init_success = false;
        bool _shutdown = false;
    }; //class

}  // namespace EA::discovery
