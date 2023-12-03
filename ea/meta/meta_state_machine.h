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
#include "ea/raft/raft_control.h"
#include "ea/proto/meta.interface.pb.h"
#include "ea/base/bthread.h"
#include "ea/base/time_cast.h"

namespace EA::db {

    class MetaStateMachine;

    struct MetaServerClosure : public braft::Closure {
        void Run() override;

        brpc::Controller *cntl;
        MetaStateMachine *meta_state_machine;
        google::protobuf::Closure *done;
        EA::db::MetaManageResponse *response;
        std::string request;
        int64_t raft_time_cost;
        int64_t total_time_cost;
        TimeCost time_cost;
    };


    class MetaStateMachine : public braft::StateMachine {
    public:

        MetaStateMachine(int64_t dummy_region_id,
                         const std::string &identify,
                         const std::string &file_path,
                         const braft::PeerId &peerId) :
                _node(identify, peerId),
                _is_leader(false),
                _dummy_region_id(dummy_region_id),
                _file_path(file_path) {}

        ~MetaStateMachine() = default;

        virtual int init(const std::vector<braft::PeerId> &peers);

        virtual void raft_control(google::protobuf::RpcController *controller,
                                  const EA::RaftControlRequest *request,
                                  EA::RaftControlResponse *response,
                                  google::protobuf::Closure *done) {
            brpc::ClosureGuard done_guard(done);
            if (!is_leader() && !request->force()) {
                TLOG_INFO("node is not leader when raft control, region_id: {}", request->region_id());
                response->set_errcode(EA::NOT_LEADER);
                response->set_region_id(request->region_id());
                response->set_leader(butil::endpoint2str(_node.leader_id().addr).c_str());
                response->set_errmsg("not leader");
                return;
            }
            common_raft_control(controller, request, response, done_guard.release(), &_node);
        }

        virtual void process(google::protobuf::RpcController *controller,
                             const EA::discovery::DiscoveryManagerRequest *request,
                             EA::discovery::DiscoveryManagerResponse *response,
                             google::protobuf::Closure *done);

        // state machine method
        virtual void on_apply(braft::Iterator &iter) = 0;

        virtual void on_shutdown() {
            TLOG_INFO("raft is shut down");
        };

        virtual void on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done) = 0;

        virtual int on_snapshot_load(braft::SnapshotReader *reader) = 0;

        virtual void on_leader_start();

        virtual void on_leader_start(int64_t term);

        virtual void on_leader_stop();

        virtual void on_leader_stop(const butil::Status &status);

        virtual void on_error(const ::braft::Error &e);

        virtual void on_configuration_committed(const ::braft::Configuration &conf);

        virtual butil::EndPoint get_leader() {
            return _node.leader_id().addr;
        }

        virtual void shutdown_raft() {
            _node.shutdown(nullptr);
            TLOG_INFO("raft node was shutdown");
            _node.join();
            TLOG_INFO("raft node join completely");
        }

        virtual bool is_leader() const{
            return _is_leader;
        }

        bool have_data() {
            return _have_data;
        }

        void set_have_data(bool flag) {
            _have_data = flag;
        }

    protected:
        braft::Node _node;
        std::atomic<bool> _is_leader;
        int64_t _dummy_region_id;
        std::string _file_path;
    private:
        bool _have_data = false;
    };

#define META_ERROR_SET_RESPONSE(response, errcode, err_message, op_type, log_id) \
    do {\
        TLOG_ERROR("request op_type:{}, {} ,log_id:{}",\
                op_type, err_message, log_id);\
        if (response != nullptr) {\
            response->set_errcode(errcode);\
            response->set_errmsg(err_message);\
            response->set_op_type(op_type);\
        }\
    }while (0);

#define ERROR_SET_RESPONSE_WARN(response, errcode, err_message, op_type, log_id) \
    do {\
        TLOG_WARN("request op_type:{}, {} ,log_id:{}",\
                op_type, err_message, log_id);\
        if (response != nullptr) {\
            response->set_errcode(errcode);\
            response->set_errmsg(err_message);\
            response->set_op_type(op_type);\
        }\
    }while (0);

#define META_IF_DONE_SET_RESPONSE(done, errcode, err_message) \
    do {\
        if (done && ((MetaServerClosure*)done)->response) {\
            ((MetaServerClosure*)done)->response->set_errcode(errcode);\
            ((MetaServerClosure*)done)->response->set_errmsg(err_message);\
        }\
    }while (0);

#define META_SET_RESPONSE(response, errcode, err_message) \
    do {\
        if (response) {\
            response->set_errcode(errcode);\
            response->set_errmsg(err_message);\
        }\
    }while (0);

#define META_RETURN_IF_NOT_INIT(init, response, log_id) \
    do {\
        if (!init) {\
            TLOG_WARN("have not init, log_id:{}", log_id);\
            response->set_errcode(EA::HAVE_NOT_INIT);\
            response->set_errmsg("have not init");\
            return;\
        }\
    } while (0);

}  // namespace EA::db

