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
#include "ea/config/config_state_machine.h"
#include "ea/rpc/config_server_interact.h"
#include "ea/config/config_manager.h"
#include <braft/util.h>
#include <braft/storage.h>
#include "ea/gflags/config.h"
#include "ea/rdb/storage.h"
#include "ea/raft/parse_path.h"


namespace EA::config {

    void ConfigServiceClosure::Run() {
        if (!status().ok()) {
            if (response) {
                response->set_errcode(proto::NOT_LEADER);
                response->set_leader(butil::endpoint2str(state_machine->get_leader()).c_str());
            }
            TLOG_ERROR("service server closure fail, error_code:{}, error_mas:{}",
                       status().error_code(), status().error_cstr());
        }
        total_time_cost = time_cost.get_time();
        if (done != nullptr) {
            done->Run();
        }
        delete this;
    }

    int ConfigStateMachine::init(const std::vector<braft::PeerId> &peers) {
        braft::NodeOptions options;
        options.election_timeout_ms = FLAGS_config_election_timeout_ms;
        options.fsm = this;
        options.initial_conf = braft::Configuration(peers);
        options.snapshot_interval_s = FLAGS_config_snapshot_interval_s;
        options.log_uri = FLAGS_config_log_uri + "0";
        //options.stable_uri = FLAGS_service_stable_uri + "/meta_server";
        options.raft_meta_uri = FLAGS_config_stable_uri;// + _file_path;
        options.snapshot_uri = FLAGS_config_snapshot_uri;// + _file_path;
        int ret = _node.init(options);
        if (ret < 0) {
            TLOG_ERROR("raft node init fail");
            return ret;
        }
        TLOG_INFO("raft init success, meat state machine init success");
        return 0;
    }

    void ConfigStateMachine::process(google::protobuf::RpcController *controller,
                                   const proto::OpsServiceRequest *request,
                                   proto::OpsServiceResponse *response,
                                   google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        if (!_is_leader) {
            if (response) {
                response->set_errcode(proto::NOT_LEADER);
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
        ConfigServiceClosure *closure = new ConfigServiceClosure;
        closure->request = request->ShortDebugString();
        closure->cntl = cntl;
        closure->response = response;
        closure->done = done_guard.release();
        closure->state_machine = this;
        braft::Task task;
        task.data = &data;
        task.done = closure;
        _node.apply(task);
    }

    void ConfigStateMachine::start_check_bns() {
        //bns ，自动探测是否迁移
        if (FLAGS_config_server_bns.find(":") == std::string::npos) {
            if (!_check_start) {
                auto fun = [this]() {
                    start_check_migrate();
                };
                _check_migrate.run(fun);
                _check_start = true;
            }
        }
    }

    void ConfigStateMachine::on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) {
        TLOG_WARN("start on snapshot save");
        Bthread bth(&BTHREAD_ATTR_SMALL);
        std::function<void()> save_snapshot_function = [this, done, writer]() {
            save_snapshot(done, writer);
        };
        bth.run(save_snapshot_function);
    }

    void ConfigStateMachine::save_snapshot(braft::Closure *done, braft::SnapshotWriter *writer) {
        brpc::ClosureGuard done_guard(done);

        std::string snapshot_path = writer->get_path();
        std::string sst_file_path = snapshot_path + FLAGS_config_snapshot_sst;

        auto rs = EA::rdb::Storage::get_instance()->dump_rkv(sst_file_path);
        if (!rs.ok()) {
            done->status().set_error(EINVAL, "Fail to finish SstFileWriter");
            return;
        }
        if (writer->add_file(FLAGS_config_snapshot_sst) != 0) {
            done->status().set_error(EINVAL, "Fail to add file");
            TLOG_WARN("Error while adding file to writer");
            return;
        }
    }

    void ConfigStateMachine::raft_control(google::protobuf::RpcController *controller,
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

    int ConfigStateMachine::on_snapshot_load(braft::SnapshotReader *reader) {
        TLOG_WARN("start on snapshot load");
        // clean local data
        auto rs = EA::rdb::Storage::get_instance()->clean_rkv();
        if (!rs.ok()) {
            return -1;
        }
        TLOG_WARN("clear data success");
        std::vector<std::string> files;
        reader->list_files(&files);
        for (auto &file: files) {
            TLOG_WARN("snapshot load file:{}", file);
            if (file == FLAGS_config_snapshot_sst) {
                std::string snapshot_path = reader->get_path();
                _applied_index = parse_snapshot_index_from_path(snapshot_path, false);
                TLOG_WARN("_applied_index:{} path:{}", _applied_index, snapshot_path);
                snapshot_path.append(FLAGS_config_snapshot_sst);

                // restore from file
                auto res = EA::rdb::Storage::get_instance()->load_rkv(snapshot_path);
                if (!res.ok()) {
                    TLOG_WARN("Error while ingest file {}, Error {}",
                              snapshot_path, res.ToString());
                    return -1;

                }
                // restore memory store
                int ret = 0;
                ret = ConfigManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    TLOG_ERROR("ClusterManager load snapshot fail");
                    return -1;
                }
            }
        }
        set_have_data(true);
        return 0;
    }
    void ConfigStateMachine::on_leader_start(int64_t term) {
        TLOG_INFO("leader start at term: {}", term);
        start_check_bns();
        _is_leader.store(true);
    }

    void ConfigStateMachine::on_leader_stop(const butil::Status &status) {
        TLOG_INFO("leader stop, error_code:%d, error_des:{}",
                  status.error_code(), status.error_cstr());
        _is_leader.store(false);
        if (_check_start) {
            _check_migrate.join();
            _check_start = false;
            TLOG_INFO("check migrate thread join");
        }
        TLOG_INFO("leader stop");
    }

    void ConfigStateMachine::on_error(const ::braft::Error &e) {
        TLOG_ERROR("service state machine error, error_type:{}, error_code:{}, error_des:{}",
                   static_cast<int>(e.type()), e.status().error_code(), e.status().error_cstr());
    }

    void ConfigStateMachine::on_configuration_committed(const ::braft::Configuration &conf) {
        std::string new_peer;
        for (auto iter = conf.begin(); iter != conf.end(); ++iter) {
            new_peer += iter->to_string() + ",";
        }
        TLOG_INFO("new conf committed, new peer: {}", new_peer.c_str());
    }

    void ConfigStateMachine::start_check_migrate() {
        TLOG_INFO("start check migrate");
        static int64_t count = 0;
        int64_t sleep_time_count = FLAGS_config_check_migrate_interval_us / (1000 * 1000LL); //以S为单位
        while (_node.is_leader()) {
            int time = 0;
            while (time < sleep_time_count) {
                if (!_node.is_leader()) {
                    return;
                }
                bthread_usleep(1000 * 1000LL);
                ++time;
            }
            TLOG_TRACE("start check migrate, count: {}", count);
            ++count;
            check_migrate();
        }
    }

    void ConfigStateMachine::check_migrate() {
        //判断service server是否需要做迁移
        /*
        std::vector<std::string> instances;
        std::string remove_peer;
        std::string add_peer;
        int ret = 0;
        if (get_instance_from_bns(&ret, FLAGS_config_server_bns, instances, false) != 0 ||
            (int32_t) instances.size() != FLAGS_config_replica_number) {
            TLOG_WARN("get instance from bns fail, bns:%s, ret:{}, instance.size:{}",
                      FLAGS_config_server_bns.c_str(), ret, instances.size());
            return;
        }
        std::set<std::string> instance_set;
        for (auto &instance: instances) {
            instance_set.insert(instance);
        }
        std::set<std::string> peers_in_server;
        std::vector<braft::PeerId> peers;
        if (!_node.list_peers(&peers).ok()) {
            TLOG_WARN("node list peer fail");
            return;
        }
        for (auto &peer: peers) {
            peers_in_server.insert(butil::endpoint2str(peer.addr).c_str());
        }
        if (peers_in_server == instance_set) {
            return;
        }
        for (auto &peer: peers_in_server) {
            if (instance_set.find(peer) == instance_set.end()) {
                remove_peer = peer;
                TLOG_INFO("remove peer: {}", remove_peer);
                break;
            }
        }
        for (auto &peer: instance_set) {
            if (peers_in_server.find(peer) == peers_in_server.end()) {
                add_peer = peer;
                TLOG_INFO("add peer: {}", add_peer);
                break;
            }
        }
        ret = 0;
        if (remove_peer.size() != 0) {
            //发送remove_peer的请求
            ret = send_set_peer_request(true, remove_peer);
        }
        if (add_peer.size() != 0 && ret == 0) {
            send_set_peer_request(false, add_peer);
        }
         */
    }

    void ConfigStateMachine::on_apply(braft::Iterator &iter) {
        for (; iter.valid(); iter.next()) {
            braft::Closure *done = iter.done();
            brpc::ClosureGuard done_guard(done);
            if (done) {
                ((ConfigServiceClosure *) done)->raft_time_cost = ((ConfigServiceClosure *) done)->time_cost.get_time();
            }
            butil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            proto::OpsServiceRequest request;
            if (!request.ParseFromZeroCopyStream(&wrapper)) {
                TLOG_ERROR("parse from protobuf fail when on_apply");
                if (done) {
                    if (((ConfigServiceClosure *) done)->response) {
                        ((ConfigServiceClosure *) done)->response->set_errcode(proto::PARSE_FROM_PB_FAIL);
                        ((ConfigServiceClosure *) done)->response->set_errmsg("parse from protobuf fail");
                    }
                    braft::run_closure_in_bthread(done_guard.release());
                }
                continue;
            }
            if (done && ((ConfigServiceClosure *) done)->response) {
                ((ConfigServiceClosure *) done)->response->set_op_type(request.op_type());
            }
            TLOG_INFO("on apply, term:{}, index:{}, request op_type:{}",
                      iter.term(), iter.index(),
                      proto::OpType_Name(request.op_type()));
            switch (request.op_type()) {
                case proto::OP_CREATE_CONFIG: {
                    ConfigManager::get_instance()->create_config(request, done);
                    break;
                }
                case proto::OP_REMOVE_CONFIG: {
                    ConfigManager::get_instance()->remove_config(request, done);
                    break;
                }
                default: {
                    TLOG_ERROR("unsupport request type, type:{}", request.op_type());
                    CONFIG_SERVICE_SET_DONE_AND_RESPONSE(done, proto::UNSUPPORT_REQ_TYPE, "unsupport request type");
                }
            }
            _applied_index = iter.index();
            if (done) {
                braft::run_closure_in_bthread(done_guard.release());
            }
        }
    }

    int ConfigStateMachine::send_set_peer_request(bool remove_peer, const std::string &change_peer) {
        EA::rpc::ConfigServerInteract config_server_interact;
        if (config_server_interact.init() != 0) {
            TLOG_ERROR("service server interact init fail when set peer");
            return -1;
        }
        proto::RaftControlRequest request;
        request.set_op_type(proto::SetPeer);
        std::set<std::string> peers_in_server;
        std::vector<braft::PeerId> peers;
        if (!_node.list_peers(&peers).ok()) {
            TLOG_WARN("node list peer fail");
            return -1;
        }
        for (auto &peer: peers) {
            request.add_old_peers(butil::endpoint2str(peer.addr).c_str());
            if (!remove_peer || (remove_peer && peer != change_peer)) {
                request.add_new_peers(butil::endpoint2str(peer.addr).c_str());
            }
        }
        if (!remove_peer) {
            request.add_new_peers(change_peer);
        }
        proto::RaftControlResponse response;
        int ret = config_server_interact.send_request("raft_control", request, response);
        if (ret != 0) {
            TLOG_WARN("set peer when service server migrate fail, request:{}, response:{}",
                      request.ShortDebugString(), response.ShortDebugString());
        }
        return ret;
    }

}  // namespace EA::config
