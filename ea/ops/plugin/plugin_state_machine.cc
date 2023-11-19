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
#include "ea/ops/plugin/plugin_state_machine.h"
#include "ea/rpc/plugin_server_interact.h"
#include "ea/ops/plugin/plugin_manager.h"
#include "ea/ops/plugin/plugin_meta.h"
#include <braft/util.h>
#include <braft/storage.h>
#include "ea/raft/parse_path.h"

namespace EA::plugin {

    void PluginServiceClosure::Run() {
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

    int PluginStateMachine::init(const std::vector<braft::PeerId> &peers) {
        braft::NodeOptions options;
        options.election_timeout_ms = FLAGS_plugin_election_timeout_ms;
        options.fsm = this;
        options.initial_conf = braft::Configuration(peers);
        options.snapshot_interval_s = FLAGS_plugin_snapshot_interval_s;
        options.log_uri = FLAGS_plugin_log_uri + "0";
        //options.stable_uri = FLAGS_service_stable_uri + "/meta_server";
        options.raft_meta_uri = FLAGS_plugin_stable_uri;// + _file_path;
        options.snapshot_uri = FLAGS_plugin_snapshot_uri;// + _file_path;
        int ret = _node.init(options);
        if (ret < 0) {
            TLOG_ERROR("raft node init fail");
            return ret;
        }
        TLOG_INFO("raft init success, meat state machine init success");
        return 0;
    }

    void PluginStateMachine::process(google::protobuf::RpcController *controller,
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
        PluginServiceClosure *closure = new PluginServiceClosure;
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

    void PluginStateMachine::start_check_bns() {
        //bns ，自动探测是否迁移
        if (FLAGS_plugin_server_bns.find(":") == std::string::npos) {
            if (!_check_start) {
                auto fun = [this]() {
                    start_check_migrate();
                };
                _check_migrate.run(fun);
                _check_start = true;
            }
        }
    }

    void PluginStateMachine::on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) {
        TLOG_WARN("start on snapshot save");
        //create a snapshot
        Bthread bth(&BTHREAD_ATTR_SMALL);
        std::function<void()> save_snapshot_function = [this, done, writer]() {
            save_snapshot(done, writer);
        };
        bth.run(save_snapshot_function);
    }

    void PluginStateMachine::save_snapshot(braft::Closure *done, braft::SnapshotWriter *writer) {
        brpc::ClosureGuard done_guard(done);

        std::string snapshot_path = writer->get_path();
        std::string sst_file_path = snapshot_path + FLAGS_plugin_snapshot_sst;

        auto rs = PluginMeta::get_rkv()->dump(sst_file_path);
        if (!rs.ok()) {
            done->status().set_error(EINVAL, "Fail to finish SstFileWriter");
            return;
        }
        if (writer->add_file(FLAGS_plugin_snapshot_sst) != 0) {
            done->status().set_error(EINVAL, "Fail to add file");
            TLOG_WARN("Error while adding file to writer");
            return;
        }
        /// plugin files;
        std::string plugin_base_path = snapshot_path + "/plugins";
        std::error_code ec;
        turbo::filesystem::create_directories(plugin_base_path, ec);
        if(ec) {
            TLOG_WARN("Error while create plugin file snapshot path:{}", plugin_base_path);
            return;
        }
        std::vector<std::string> files;
        if(PluginManager::get_instance()->save_snapshot(snapshot_path, "/plugins",files) != 0) {
            done->status().set_error(EINVAL, "Fail to snapshot plugin");
            TLOG_WARN("Fail to snapshot plugin");
            return;
        }
        for(auto & f: files) {
            if (writer->add_file(f) != 0) {
                done->status().set_error(EINVAL, "Fail to add file");
                TLOG_WARN("Error while adding file to writer: {}", "/plugins/" + f);
                return;
            }
        }
    }

    int PluginStateMachine::on_snapshot_load(braft::SnapshotReader *reader) {
        TLOG_WARN("start on snapshot load");
        // clean local data
        auto rs = PluginMeta::get_rkv()->clean();
        if (!rs.ok()) {
            return -1;
        }
        std::vector<std::string> files;
        reader->list_files(&files);
        for (auto &file: files) {
            TLOG_WARN("snapshot load file:{}", file);
            if (file == FLAGS_plugin_snapshot_sst) {
                std::string snapshot_path = reader->get_path();
                _applied_index = parse_snapshot_index_from_path(snapshot_path, false);
                TLOG_WARN("_applied_index:{} path:{}", _applied_index, snapshot_path);
                snapshot_path.append(FLAGS_plugin_snapshot_sst);

                // restore from file
                auto res = PluginMeta::get_rkv()->load(snapshot_path);
                if (!res.ok()) {
                    TLOG_WARN("Error while ingest file {}, Error {}",
                              snapshot_path, res.ToString());
                    return -1;

                }
                // restore memory store
                int ret = 0;
                ret =PluginManager::get_instance()->load_snapshot();
                if (ret != 0) {
                    TLOG_ERROR("PluginManager load snapshot fail");
                    return -1;
                }
            }
            if(turbo::StartsWith(file, "/plugins")) {
                int ret =PluginManager::get_instance()->load_snapshot_file(reader->get_path() + file);
                if (ret != 0) {
                    TLOG_ERROR("PluginManager load snapshot file fail");
                    return -1;
                }
            }
        }
        set_have_data(true);
        return 0;
    }
    void PluginStateMachine::on_leader_start(int64_t term) {
        TLOG_INFO("leader start at term: {}", term);
        start_check_bns();
        _is_leader.store(true);
    }

    void PluginStateMachine::on_leader_stop(const butil::Status &status) {
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

    void PluginStateMachine::on_error(const ::braft::Error &e) {
        TLOG_ERROR("service state machine error, error_type:{}, error_code:{}, error_des:{}",
                   static_cast<int>(e.type()), e.status().error_code(), e.status().error_cstr());
    }

    void PluginStateMachine::on_configuration_committed(const ::braft::Configuration &conf) {
        std::string new_peer;
        for (auto iter = conf.begin(); iter != conf.end(); ++iter) {
            new_peer += iter->to_string() + ",";
        }
        TLOG_INFO("new conf committed, new peer: {}", new_peer.c_str());
    }

    void PluginStateMachine::start_check_migrate() {
        TLOG_INFO("start check migrate");
        static int64_t count = 0;
        int64_t sleep_time_count = FLAGS_plugin_check_migrate_interval_us / (1000 * 1000LL); //以S为单位
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

    void PluginStateMachine::check_migrate() {
        //判断service server是否需要做迁移
        std::vector<std::string> instances;
        std::string remove_peer;
        std::string add_peer;
        int ret = 0;
        /*
        if (get_instance_from_bns(&ret, FLAGS_plugin_server_bns, instances, false) != 0 ||
            (int32_t) instances.size() != FLAGS_plugin_replica_number) {
            TLOG_WARN("get instance from bns fail, bns:%s, ret:{}, instance.size:{}",
                      FLAGS_plugin_server_bns.c_str(), ret, instances.size());
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
        }*/
    }

    void PluginStateMachine::on_apply(braft::Iterator &iter) {
        for (; iter.valid(); iter.next()) {
            braft::Closure *done = iter.done();
            brpc::ClosureGuard done_guard(done);
            if (done) {
                ((PluginServiceClosure *) done)->raft_time_cost = ((PluginServiceClosure *) done)->time_cost.get_time();
            }
            butil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            proto::OpsServiceRequest request;
            if (!request.ParseFromZeroCopyStream(&wrapper)) {
                TLOG_ERROR("parse from protobuf fail when on_apply");
                if (done) {
                    if (((PluginServiceClosure *) done)->response) {
                        ((PluginServiceClosure *) done)->response->set_errcode(proto::PARSE_FROM_PB_FAIL);
                        ((PluginServiceClosure *) done)->response->set_errmsg("parse from protobuf fail");
                    }
                    braft::run_closure_in_bthread(done_guard.release());
                }
                continue;
            }
            if (done && ((PluginServiceClosure *) done)->response) {
                ((PluginServiceClosure *) done)->response->set_op_type(request.op_type());
            }
            TLOG_INFO("on apply, term:{}, index:{}, request op_type:{}",
                      iter.term(), iter.index(),
                      proto::OpType_Name(request.op_type()));
            switch (request.op_type()) {
                case proto::OP_CREATE_PLUGIN: {
                    PluginManager::get_instance()->create_plugin(request, done);
                    break;
                }
                case proto::OP_REMOVE_PLUGIN: {
                    PluginManager::get_instance()->remove_plugin(request, done);
                    break;
                }
                case proto::OP_RESTORE_TOMBSTONE_PLUGIN: {
                    PluginManager::get_instance()->restore_plugin(request, done);
                    break;
                }
                case proto::OP_REMOVE_TOMBSTONE_PLUGIN: {
                    PluginManager::get_instance()->remove_tombstone_plugin(request, done);
                    break;
                }
                case proto::OP_UPLOAD_PLUGIN: {
                    PluginManager::get_instance()->upload_plugin(request, done);
                    break;
                }
                default: {
                    TLOG_ERROR("unsupport request type, type:{}", request.op_type());
                    PLUGIN_SERVICE_SET_DONE_AND_RESPONSE(done, proto::UNSUPPORT_REQ_TYPE, "unsupport request type");
                }
            }
            _applied_index = iter.index();
            if (done) {
                braft::run_closure_in_bthread(done_guard.release());
            }
        }
    }

    int PluginStateMachine::send_set_peer_request(bool remove_peer, const std::string &change_peer) {
        EA::rpc::PluginServerInteract plugin_server_interact;
        if (plugin_server_interact.init() != 0) {
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
        int ret = plugin_server_interact.send_request("raft_control", request, response);
        if (ret != 0) {
            TLOG_WARN("set peer when service server migrate fail, request:{}, response:{}",
                      request.ShortDebugString(), response.ShortDebugString());
        }
        return ret;
    }

}  // namespace EA::plugin
