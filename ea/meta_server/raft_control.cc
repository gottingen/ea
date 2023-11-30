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


#include "ea/meta_server/raft_control.h"
#include <algorithm>
#include "ea/base/tlog.h"

namespace braft {
    DECLARE_int32(raft_election_heartbeat_factor);
}
namespace EA {
    class RaftControlDone : public braft::Closure {
    public:
        RaftControlDone(google::protobuf::RpcController *controller,
                        const EA::servlet::RaftControlRequest *request,
                        EA::servlet::RaftControlResponse *response,
                        google::protobuf::Closure *done,
                        braft::Node *node)
                : _controller(controller),
                  _request(request),
                  _response(response),
                  _done(done),
                  _node(node) {}

        ~RaftControlDone() override {}

        void Run() override{
            auto cntl = static_cast<brpc::Controller *>(_controller);
            uint64_t log_id = 0;
            if (cntl->has_log_id()) {
                log_id = cntl->log_id();
            }
            if (status().ok()) {
                TLOG_INFO("raft control success, type:{}, region_id:{}, remote_side: {}, log_id:{}",
                          _request->op_type(), _request->region_id(),
                          butil::endpoint2str(cntl->remote_side()).c_str(), log_id);
                _response->set_errcode(EA::servlet::SUCCESS);
            } else {
                TLOG_WARN("raft control failed, type:{}, region_id:{},"
                           " status:{}:{}, remote_side: {}, log_id:{}",
                           _request->op_type(), _request->region_id(),
                           status().error_code(), status().error_cstr(),
                           butil::endpoint2str(cntl->remote_side()).c_str(),
                           log_id);
                _response->set_errcode(EA::servlet::INTERNAL_ERROR);
                _response->set_errmsg(status().error_cstr());
                _response->set_leader(butil::endpoint2str(_node->leader_id().addr).c_str());
            }
            //UpdateRegionStatus::get_instance()->reset_region_status(_request->region_id());
            _done->Run();
            delete this;
        }

    private:
        google::protobuf::RpcController *_controller;
        const EA::servlet::RaftControlRequest *_request;
        EA::servlet::RaftControlResponse *_response;
        google::protobuf::Closure *_done;
        braft::Node *_node;
    };

    void _set_peer(google::protobuf::RpcController *controller,
                   const EA::servlet::RaftControlRequest *request,
                   EA::servlet::RaftControlResponse *response,
                   google::protobuf::Closure *done,
                   braft::Node *node);

    int _diff_peers(const std::vector<braft::PeerId> &old_peers,
                    const std::vector<braft::PeerId> &new_peers, braft::PeerId *peer);

    void _trans_leader(google::protobuf::RpcController *controller,
                       const EA::servlet::RaftControlRequest *request,
                       EA::servlet::RaftControlResponse *response,
                       google::protobuf::Closure *done,
                       braft::Node *node);

    void common_raft_control(google::protobuf::RpcController *controller,
                             const EA::servlet::RaftControlRequest *request,
                             EA::servlet::RaftControlResponse *response,
                             google::protobuf::Closure *done,
                             braft::Node *node) {
        auto cntl = static_cast<brpc::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        response->set_region_id(request->region_id());
        brpc::ClosureGuard done_guard(done);


        switch (request->op_type()) {
            case EA::servlet::SetPeer : {
                _set_peer(controller, request, response, done_guard.release(), node);
                return;
            }
            case EA::servlet::SnapShot : {
                RaftControlDone *snapshot_done =
                        new RaftControlDone(cntl, request, response, done_guard.release(), node);
                node->snapshot(snapshot_done);
                return;
            }
            case EA::servlet::ShutDown : {
                RaftControlDone *shutdown_done =
                        new RaftControlDone(cntl, request, response, done_guard.release(), node);
                node->shutdown(shutdown_done);
                return;
            }
            case EA::servlet::TransLeader : {
                _trans_leader(controller, request, response, done_guard.release(), node);
                return;
            }
            case EA::servlet::GetLeader : {
                butil::EndPoint leader_addr = node->leader_id().addr;
                if (leader_addr != butil::EndPoint()) {
                    response->set_errcode(EA::servlet::SUCCESS);
                    response->set_leader(butil::endpoint2str(leader_addr).c_str());
                } else {
                    TLOG_ERROR("node:{} {} get leader fail, log_id:{}",
                             node->node_id().group_id.c_str(),
                             node->node_id().peer_id.to_string().c_str(),
                             log_id);
                    response->set_errcode(EA::servlet::INTERNAL_ERROR);
                    response->set_errmsg("get leader fail");
                }
                return;
            }
            case EA::servlet::ListPeer: {
                butil::EndPoint leader_addr = node->leader_id().addr;
                if (leader_addr != butil::EndPoint()) {
                    response->set_leader(butil::endpoint2str(leader_addr).c_str());
                    std::vector<braft::PeerId> peers;
                    auto s = node->list_peers(&peers);
                    if(!s.ok()) {
                        TLOG_ERROR("node:{} {} list peers fail, log_id:{}",
                                   node->node_id().group_id.c_str(),
                                   node->node_id().peer_id.to_string().c_str(),
                                   log_id);
                        response->set_errcode(EA::servlet::INTERNAL_ERROR);
                        response->set_errmsg("list peers fail");
                        return;
                    }
                    for(auto &peer : peers) {
                        response->add_peers(butil::endpoint2str(peer.addr).c_str());
                    }
                    response->set_errcode(EA::servlet::SUCCESS);
                    return;
                } else {
                    TLOG_ERROR("node:{} {} get leader fail, log_id:{}",
                               node->node_id().group_id.c_str(),
                               node->node_id().peer_id.to_string().c_str(),
                               log_id);
                    response->set_errcode(EA::servlet::INTERNAL_ERROR);
                    response->set_errmsg("get leader fail");
                }
                return;
            }
            case EA::servlet::ResetVoteTime : {
                node->reset_election_timeout_ms(request->election_time());
                response->set_errcode(EA::servlet::SUCCESS);
                return;
            }
            default:
                TLOG_ERROR("node:{} {} unsupport request type:{}, log_id:{}",
                         node->node_id().group_id.c_str(),
                         node->node_id().peer_id.to_string().c_str(),
                         request->op_type(), log_id);
                return;
        }
    }

    void _set_peer(google::protobuf::RpcController *controller,
                   const EA::servlet::RaftControlRequest *request,
                   EA::servlet::RaftControlResponse *response,
                   google::protobuf::Closure *done,
                   braft::Node *node) {
        auto cntl =
                static_cast<brpc::Controller *>(controller);
        brpc::ClosureGuard done_guard(done);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        bool is_force = request->has_force() && request->force();
        std::vector<braft::PeerId> old_peers;
        std::vector<braft::PeerId> new_peers;
        for (int i = 0; i < request->old_peers_size(); i++) {
            braft::PeerId peer;
            if (peer.parse(request->old_peers(i)) != 0) {
                response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                response->set_errmsg("old peer parse fail");
                return;
            }
            old_peers.push_back(peer);
        }
        for (int i = 0; i < request->new_peers_size(); i++) {
            braft::PeerId peer;
            if (peer.parse(request->new_peers(i)) != 0) {
                response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                response->set_errmsg("new peer parse fail");
                return;
            }
            new_peers.push_back(peer);
        }
        if (is_force) {
            braft::Configuration new_conf(new_peers);
            auto status = node->reset_peers(new_conf);
            if (!status.ok()) {
                TLOG_ERROR("node:{} {} set peer fail, status:{} {}, ret:{}",
                         node->node_id().group_id.c_str(),
                         node->node_id().peer_id.to_string().c_str(),
                         status.error_code(), status.error_cstr(),
                         status.error_cstr());
                response->set_errcode(EA::servlet::INTERNAL_ERROR);
                response->set_errmsg("force set peer fail");
            } else {
                response->set_errcode(EA::servlet::SUCCESS);
                response->set_errmsg("force set peer success");
            }
            return;
        }
        std::vector<braft::PeerId> inner_peers;
        auto status = node->list_peers(&inner_peers);
        if (!status.ok() && status.error_code() == 1) {
            response->set_errcode(EA::servlet::NOT_LEADER);
            response->set_leader(butil::endpoint2str(node->leader_id().addr).c_str());
            TLOG_WARN("node:{} {} list peers fail, not leader, status:{} {}, log_id: {}",
                       node->node_id().group_id.c_str(),
                       node->node_id().peer_id.to_string().c_str(),
                       status.error_code(), status.error_cstr(),
                       log_id);
            return;
        }
        if (!status.ok()) {
            response->set_errcode(EA::servlet::PEER_NOT_EQUAL);
            response->set_errmsg("node list peer fail");
            TLOG_WARN("node:{} {} list peers fail, status:{} {}, log_id: {}",
                       node->node_id().group_id.c_str(),
                       node->node_id().peer_id.to_string().c_str(),
                       status.error_code(), status.error_cstr(),
                       log_id);
            return;
        }
        if (inner_peers.size() != old_peers.size()) {
            TLOG_WARN("peer size is not equal when set peer, node:{} {},"
                       " inner_peer.size: {}, old_peer.size: {}, remote_side: {}, log_id: {}",
                       node->node_id().group_id.c_str(),
                       node->node_id().peer_id.to_string().c_str(),
                       inner_peers.size(),
                       old_peers.size(),
                       butil::endpoint2str(cntl->remote_side()).c_str(),
                       log_id);
            response->set_errcode(EA::servlet::PEER_NOT_EQUAL);
            response->set_errmsg("peer size not equal");
            return;
        }
        for (auto &inner_peer: inner_peers) {
            auto iter = std::find(old_peers.begin(), old_peers.end(), inner_peer);
            if (iter == old_peers.end()) {
                TLOG_WARN("old_peer not equal to list peers, node:{} {}, inner_peer: {}, log_id: {}",
                           node->node_id().group_id.c_str(),
                           node->node_id().peer_id.to_string().c_str(),
                           butil::endpoint2str(inner_peer.addr).c_str(),
                           log_id);
                response->set_errcode(EA::servlet::PEER_NOT_EQUAL);
                response->set_errmsg("peer not equal");
                return;
            }
        }
        braft::PeerId peer;
        // add peer
        if (new_peers.size() == old_peers.size() + 1) {
            if (0 == _diff_peers(old_peers, new_peers, &peer)) {
                auto set_peer_done =
                        new RaftControlDone(cntl, request, response, done_guard.release(), node);
                node->add_peer(peer, set_peer_done);
            } else {
                response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                response->set_errmsg("diff peer fail when add peer");
                TLOG_ERROR("node:{} {} set peer fail, log_id:{}",
                         node->node_id().group_id.c_str(),
                         node->node_id().peer_id.to_string().c_str(),
                         log_id);
                return;
            }
        } else if (old_peers.size() == new_peers.size() + 1) {
            if (0 == _diff_peers(old_peers, new_peers, &peer)) {
                bool self_faulty = false;
                bool other_faulty = false;
                braft::NodeStatus status;
                node->get_status(&status);
                for (auto iter: status.stable_followers) {
                    if (iter.first == peer) {
                        if (iter.second.consecutive_error_times > braft::FLAGS_raft_election_heartbeat_factor) {
                            self_faulty = true;
                            break;
                        }
                    } else {
                        if (iter.second.consecutive_error_times > braft::FLAGS_raft_election_heartbeat_factor) {
                            TLOG_WARN("node:{} {} peer:{} is faulty,log_id:{}",
                                       node->node_id().group_id.c_str(),
                                       node->node_id().peer_id.to_string().c_str(),
                                       iter.first.to_string().c_str(),
                                       log_id);
                            other_faulty = true;
                        }
                    }
                }
                if (self_faulty || (!self_faulty && !other_faulty)) {
                    auto  set_peer_done =
                            new RaftControlDone(cntl, request, response, done_guard.release(), node);
                    node->remove_peer(peer, set_peer_done);
                } else {
                    response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                    response->set_errmsg("other peer is faulty");
                    TLOG_ERROR("node:{} {} set peer fail,log_id:{}",
                             node->node_id().group_id.c_str(),
                             node->node_id().peer_id.to_string().c_str(),
                             log_id);
                    return;
                }
            } else {
                response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                response->set_errmsg("diff peer fail when remove peer");
                TLOG_ERROR("node:{} {} set peer fail,log_id:{}",
                         node->node_id().group_id.c_str(),
                         node->node_id().peer_id.to_string().c_str(),
                         log_id);
                return;
            }
        } else {
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            response->set_errmsg("set peer fail");
            TLOG_INFO("node:{} {}, set_peer argument failed, log_id:{}",
                      node->node_id().group_id.c_str(),
                      node->node_id().peer_id.to_string().c_str(),
                      log_id);
        }
    }

    void _trans_leader(google::protobuf::RpcController *controller,
                       const EA::servlet::RaftControlRequest *request,
                       EA::servlet::RaftControlResponse *response,
                       google::protobuf::Closure *done,
                       braft::Node *node) {
        brpc::ClosureGuard done_guard(done);
        auto cntl =
                static_cast<brpc::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        braft::PeerId peer;
        if (peer.parse(request->new_leader()) != 0) {
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            response->set_errmsg("new leader parse fail");
            return;
        }
        // return 0 or -1
        int ret = node->transfer_leadership_to(peer);
        if (ret != 0) {
            response->set_errcode(EA::servlet::NOT_LEADER);
            response->set_leader(butil::endpoint2str(node->leader_id().addr).c_str());
            TLOG_WARN("node:{} {} transfer leader fail, log_id:{}",
                       node->node_id().group_id.c_str(),
                       node->node_id().peer_id.to_string().c_str(),
                       log_id);
            return;
        }
        std::vector<braft::PeerId> peers;
        auto s = node->list_peers(&peers);
        if(!s.ok()) {
            TLOG_ERROR("node:{} {} list peers fail, log_id:{}",
                       node->node_id().group_id.c_str(),
                       node->node_id().peer_id.to_string().c_str(),
                       log_id);
            response->set_errcode(EA::servlet::INTERNAL_ERROR);
            response->set_errmsg("list peers fail");
            return;
        }
        for(auto &peer : peers) {
            response->add_peers(butil::endpoint2str(peer.addr).c_str());
        }
        response->set_errcode(EA::servlet::SUCCESS);
        response->set_leader(request->new_leader());
    }

    int _diff_peers(const std::vector<braft::PeerId> &old_peers,
                    const std::vector<braft::PeerId> &new_peers, braft::PeerId *peer) {
        braft::Configuration old_conf(old_peers);
        braft::Configuration new_conf(new_peers);
        if (old_peers.size() == new_peers.size() - 1 && new_conf.contains(old_peers)) {
            // add peer
            for (size_t i = 0; i < old_peers.size(); i++) {
                new_conf.remove_peer(old_peers[i]);
            }
            std::vector<braft::PeerId> peers;
            new_conf.list_peers(&peers);
            //CHECK(1 == peers.size());
            if (1 != peers.size()) {
                return -1;
            }
            *peer = peers[0];
            return 0;
        } else if (old_peers.size() == new_peers.size() + 1 && old_conf.contains(new_peers)) {
            // remove peer
            for (size_t i = 0; i < new_peers.size(); i++) {
                old_conf.remove_peer(new_peers[i]);
            }
            std::vector<braft::PeerId> peers;
            old_conf.list_peers(&peers);
            if (1 != peers.size()) {
                return -1;
            }
            *peer = peers[0];
            return 0;
        } else {
            return -1;
        }
    }

}//namespace

