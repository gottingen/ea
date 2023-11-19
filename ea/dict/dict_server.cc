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
#include "ea/dict/dict_server.h"
#include "ea/dict/query_dict_manager.h"
#include "ea/dict/dict_state_machine.h"

namespace EA::dict {
    void DictServer::dict_manage(::google::protobuf::RpcController* controller,
                 const ::EA::proto::OpsServiceRequest* request,
                 ::EA::proto::OpsServiceResponse* response,
                 ::google::protobuf::Closure* done)  {
        brpc::ClosureGuard done_guard(done);
        auto op_type = request->op_type();
        switch (op_type) {
            case EA::proto::OP_CREATE_DICT:{
                _machine->process(controller, request, response, done_guard.release());
                break;
            }
            case EA::proto::OP_UPLOAD_DICT:{
                _machine->process(controller, request, response, done_guard.release());
                break;
            }
            case EA::proto::OP_REMOVE_DICT:{
                _machine->process(controller, request, response, done_guard.release());
                break;
            }
            case EA::proto::OP_RESTORE_TOMBSTONE_DICT:{
                _machine->process(controller, request, response, done_guard.release());
                break;
            }
            case EA::proto::OP_REMOVE_TOMBSTONE_DICT:{
                _machine->process(controller, request, response, done_guard.release());
                break;
            }
            default:{
                response->set_errcode(proto::INPUT_PARAM_ERROR);
                response->set_errmsg("invalid op_type");
            }
        }
    }

    void DictServer::dict_query(::google::protobuf::RpcController *controller,
                   const ::EA::proto::QueryOpsServiceRequest *request,
                   ::EA::proto::QueryOpsServiceResponse *response,
                   ::google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        auto op_type = request->op_type();
        switch (op_type) {
            case EA::proto::QUERY_DOWNLOAD_DICT:{
                QueryDictManager::get_instance()->download_dict(request, response);
                break;
            }
            case EA::proto::QUERY_INFO_DICT:{
                QueryDictManager::get_instance()->dict_info(request, response);
                break;
            }
            case EA::proto::QUERY_TOMBSTONE_DICT_INFO:{
                QueryDictManager::get_instance()->tombstone_dict_info(request, response);
                break;
            }
            case EA::proto::QUERY_LIST_DICT:{
                QueryDictManager::get_instance()->list_dict(request, response);
                break;
            }
            case EA::proto::QUERY_LIST_DICT_VERSION:{
                QueryDictManager::get_instance()->list_dict_version(request, response);
                break;
            }
            case EA::proto::QUERY_TOMBSTONE_LIST_DICT:{
                QueryDictManager::get_instance()->tombstone_list_dict(request, response);
                break;
            }
            case EA::proto::QUERY_TOMBSTONE_LIST_DICT_VERSION:{
                QueryDictManager::get_instance()->tombstone_list_dict_version(request, response);
                break;
            }
            default:{
                response->set_errcode(proto::INPUT_PARAM_ERROR);
                response->set_errmsg("invalid op_type");
            }
        }
    }

    int DictServer::init(const std::vector<braft::PeerId> &peers) {
        butil::EndPoint addr;
        butil::str2endpoint(FLAGS_dict_listen.c_str(), &addr);
        braft::PeerId peer_id(addr, 0);
        _machine = new(std::nothrow)DictStateMachine("dict_raft", peer_id);
        if (_machine == nullptr) {
            TLOG_ERROR("new meta_state_machine fail");
            return -1;
        }
        auto ret = _machine->init(peers);
        if (ret != 0) {
            TLOG_ERROR("service state machine init fail");
            return -1;
        }
        TLOG_INFO("service state machine init success");
        /// clean read links
        //QueryDictManager::get_instance()->init();
        return 0;
    }

    bool DictServer::have_data() {
        if(!_machine) {
            return true;
        }
        return _machine->have_data();
    }

    void DictServer::shutdown_raft() {
        if(_machine) {
            _machine->shutdown_raft();
        }
    }

    void DictServer::close() {

    }
}  // EA::dict
