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


#ifndef EA_OPS_PLUGIN_PLUGIN_STATE_MACHINE_H_
#define EA_OPS_PLUGIN_PLUGIN_STATE_MACHINE_H_

#include <braft/raft.h>
#include "ea/raft/raft_control.h"
#include "eaproto/ops/ops.interface.pb.h"
#include "ea/base/tlog.h"
#include "ea/base/time_cost.h"
#include "ea/base/bthread.h"

namespace EA::plugin {
    class PluginStateMachine;

    struct PluginServiceClosure : public braft::Closure {
        void Run() override;

        brpc::Controller *cntl;
        PluginStateMachine *state_machine;
        google::protobuf::Closure *done;
        proto::OpsServiceResponse *response;
        std::string request;
        int64_t raft_time_cost;
        int64_t total_time_cost;
        TimeCost time_cost;
    };

    class PluginStateMachine : public braft::StateMachine {
    public:

        PluginStateMachine(const std::string &identify,
                         const braft::PeerId &peerId) :
                _node(identify, peerId),
                _is_leader(false),
                _check_migrate(&BTHREAD_ATTR_SMALL) {}

        ~PluginStateMachine() override = default;

        int init(const std::vector<braft::PeerId> &peers);

        void raft_control(google::protobuf::RpcController *controller,
                                  const proto::RaftControlRequest *request,
                                  proto::RaftControlResponse *response,
                                  google::protobuf::Closure *done) {
            brpc::ClosureGuard done_guard(done);
            if (!is_leader() && !request->force()) {
                TLOG_INFO("node is not leader when raft control, region_id: {}", request->region_id());
                response->set_errcode(proto::NOT_LEADER);
                response->set_region_id(request->region_id());
                response->set_leader(butil::endpoint2str(_node.leader_id().addr).c_str());
                response->set_errmsg("not leader");
                return;
            }
            common_raft_control(controller, request, response, done_guard.release(), &_node);
        }

        void process(google::protobuf::RpcController *controller,
                             const proto::OpsServiceRequest *request,
                             proto::OpsServiceResponse *response,
                             google::protobuf::Closure *done);

        void start_check_migrate();

        void check_migrate();

        // state machine method
        void on_apply(braft::Iterator &iter) override;

        void on_shutdown() override{
            TLOG_INFO("raft is shut down");
        };

        void on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) override;

        int on_snapshot_load(braft::SnapshotReader *reader) override;

        void on_leader_start(int64_t term) override;

        void on_leader_stop(const butil::Status &status) override;

        void on_error(const ::braft::Error &e) override;

        void on_configuration_committed(const ::braft::Configuration &conf) override;

        butil::EndPoint get_leader() {
            return _node.leader_id().addr;
        }

        void shutdown_raft() {
            _node.shutdown(nullptr);
            TLOG_INFO("raft node was shutdown");
            _node.join();
            TLOG_INFO("raft node join completely");
        }

        void start_check_bns();

        bool is_leader() const {
            return _is_leader;
        }

        bool have_data() {
            return _have_data;
        }

        void set_have_data(bool flag) {
            _have_data = flag;
        }

    private:
        virtual int send_set_peer_request(bool remove_peer, const std::string &change_peer);

        void save_snapshot(braft::Closure *done, braft::SnapshotWriter *writer);

    protected:
        braft::Node _node;
        std::atomic<bool> _is_leader;
    private:
        Bthread _check_migrate;
        bool _check_start = false;
        bool _have_data = false;
        int64_t _applied_index{0};
    };

}  // namespace EA::plugin

#define PLUGIN_SERVICE_SET_DONE_AND_RESPONSE(done, errcode, err_message) \
    do {\
        if (done && ((PluginServiceClosure*)done)->response) {\
            ((PluginServiceClosure*)done)->response->set_errcode(errcode);\
            ((PluginServiceClosure*)done)->response->set_errmsg(err_message);\
        }\
    }while (0);

#endif  // EA_OPS_PLUGIN_PLUGIN_STATE_MACHINE_H_
