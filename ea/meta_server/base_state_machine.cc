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


#include "ea/meta_server/base_state_machine.h"
#include "ea/flags/meta.h"

namespace EA::servlet {

    void MetaServerClosure::Run() {
        if (!status().ok()) {
            if (response) {
                response->set_errcode(EA::servlet::NOT_LEADER);
                response->set_leader(butil::endpoint2str(common_state_machine->get_leader()).c_str());
            }
            TLOG_ERROR("meta server closure fail, error_code:{}, error_mas:{}",
                     status().error_code(), status().error_cstr());
        }
        total_time_cost = time_cost.get_time();
        std::string remote_side;
        if (cntl != nullptr) {
            remote_side = butil::endpoint2str(cntl->remote_side()).c_str();
        }

        if (response != nullptr && response->op_type() != EA::servlet::OP_GEN_ID_FOR_AUTO_INCREMENT) {
            TLOG_INFO("request:{}, response:{}, raft_time_cost:[{}], total_time_cost:[{}], remote_side:[{}]",
                      request,
                      response->ShortDebugString(),
                      raft_time_cost,
                      total_time_cost,
                      remote_side);
        }
        if (done != nullptr) {
            done->Run();
        }
        delete this;
    }

    void TsoClosure::Run() {
        if (!status().ok()) {
            if (response) {
                response->set_errcode(EA::servlet::NOT_LEADER);
                response->set_leader(butil::endpoint2str(common_state_machine->get_leader()).c_str());
            }
            TLOG_ERROR("meta server closure fail, error_code:{}, error_mas:{}",
                     status().error_code(), status().error_cstr());
        }
        if (sync_cond) {
            sync_cond->decrease_signal();
        }
        if (done != nullptr) {
            done->Run();
        }
        delete this;
    }

    int BaseStateMachine::init(const std::vector<braft::PeerId> &peers) {
        braft::NodeOptions options;
        options.election_timeout_ms = FLAGS_meta_election_timeout_ms;
        options.fsm = this;
        options.initial_conf = braft::Configuration(peers);
        options.snapshot_interval_s = FLAGS_meta_snapshot_interval_s;
        options.log_uri = FLAGS_meta_log_uri + std::to_string(_dummy_region_id);
        //options.stable_uri = FLAGS_meta_stable_uri + "/meta_server";
        options.raft_meta_uri = FLAGS_meta_stable_uri + _file_path;
        options.snapshot_uri = FLAGS_meta_snapshot_uri + _file_path;
        int ret = _node.init(options);
        if (ret < 0) {
            TLOG_ERROR("raft node init fail");
            return ret;
        }
        TLOG_INFO("raft init success, meat state machine init success");
        return 0;
    }

    void BaseStateMachine::process(google::protobuf::RpcController *controller,
                                     const EA::servlet::MetaManagerRequest *request,
                                     EA::servlet::MetaManagerResponse *response,
                                     google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        if (!_is_leader) {
            if (response) {
                response->set_errcode(EA::servlet::NOT_LEADER);
                response->set_errmsg("not leader");
                response->set_leader(butil::endpoint2str(_node.leader_id().addr).c_str());
            }
            TLOG_WARN("state machine not leader, request: {}", request->ShortDebugString());
            return;
        }
        brpc::Controller *cntl =
                static_cast<brpc::Controller *>(controller);
        butil::IOBuf data;
        butil::IOBufAsZeroCopyOutputStream wrapper(&data);
        if (!request->SerializeToZeroCopyStream(&wrapper) && cntl) {
            cntl->SetFailed(brpc::EREQUEST, "Fail to serialize request");
            return;
        }
        MetaServerClosure *closure = new MetaServerClosure;
        closure->request = request->ShortDebugString();
        closure->cntl = cntl;
        closure->response = response;
        closure->done = done_guard.release();
        closure->common_state_machine = this;
        braft::Task task;
        task.data = &data;
        task.done = closure;
        _node.apply(task);
    }

    void BaseStateMachine::on_leader_start() {
        _is_leader.store(true);
    }

    void BaseStateMachine::on_leader_start(int64_t term) {
        TLOG_INFO("leader start at term: {}", term);
        on_leader_start();
    }

    void BaseStateMachine::on_leader_stop() {
        _is_leader.store(false);
        TLOG_INFO("leader stop");
    }

    void BaseStateMachine::on_leader_stop(const butil::Status &status) {
        TLOG_INFO("leader stop, error_code:%d, error_des:{}",
                   status.error_code(), status.error_cstr());
        on_leader_stop();
    }

    void BaseStateMachine::on_error(const ::braft::Error &e) {
        TLOG_ERROR("meta state machine error, error_type:{}, error_code:{}, error_des:{}",
                 static_cast<int>(e.type()), e.status().error_code(), e.status().error_cstr());
    }

    void BaseStateMachine::on_configuration_committed(const ::braft::Configuration &conf) {
        std::string new_peer;
        for (auto iter = conf.begin(); iter != conf.end(); ++iter) {
            new_peer += iter->to_string() + ",";
        }
        TLOG_INFO("new conf committed, new peer: {}", new_peer.c_str());
    }

}  // namespace EA::servlet
