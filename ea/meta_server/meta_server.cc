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


#include "ea/meta_server/meta_server.h"
#include "ea/meta_server/auto_incr_state_machine.h"
#include "ea/meta_server/tso_state_machine.h"
#include "ea/meta_server/meta_state_machine.h"
#include "ea/meta_server/privilege_manager.h"
#include "ea/meta_server/schema_manager.h"
#include "ea/meta_server/config_manager.h"
#include "ea/meta_server/query_config_manager.h"
#include "ea/meta_server/query_privilege_manager.h"
#include "ea/meta_server/query_namespace_manager.h"
#include "ea/meta_server/query_instance_manager.h"
#include "ea/meta_server/query_zone_manager.h"
#include "ea/meta_server/query_servlet_manager.h"
#include "ea/meta_server/meta_rocksdb.h"

namespace EA::servlet {

    MetaServer::~MetaServer() {}

    int MetaServer::init(const std::vector<braft::PeerId> &peers) {
        auto ret = MetaRocksdb::get_instance()->init();
        if (ret < 0) {
            TLOG_ERROR("rocksdb init fail");
            return -1;
        }
        butil::EndPoint addr;
        butil::str2endpoint(FLAGS_meta_listen.c_str(), &addr);
        //addr.ip = butil::my_ip();
        //addr.port = FLAGS_meta_port;
        braft::PeerId peer_id(addr, 0);
        _meta_state_machine = new(std::nothrow)MetaStateMachine(peer_id);
        if (_meta_state_machine == nullptr) {
            TLOG_ERROR("new meta_state_machine fail");
            return -1;
        }
        //state_machine初始化
        ret = _meta_state_machine->init(peers);
        if (ret != 0) {
            TLOG_ERROR("meta state machine init fail");
            return -1;
        }
        TLOG_WARN("meta state machine init success");

        _auto_incr_state_machine = new(std::nothrow)AutoIncrStateMachine(peer_id);
        if (_auto_incr_state_machine == nullptr) {
            TLOG_ERROR("new auot_incr_state_machine fail");
            return -1;
        }
        ret = _auto_incr_state_machine->init(peers);
        if (ret != 0) {
            TLOG_ERROR(" auot_incr_state_machine init fail");
            return -1;
        }
        TLOG_WARN("auot_incr_state_machine init success");

        _tso_state_machine = new(std::nothrow)TSOStateMachine(peer_id);
        if (_tso_state_machine == nullptr) {
            TLOG_ERROR("new _tso_state_machine fail");
            return -1;
        }
        ret = _tso_state_machine->init(peers);
        if (ret != 0) {
            TLOG_ERROR(" _tso_state_machine init fail");
            return -1;
        }
        TLOG_WARN("_tso_state_machine init success");

        SchemaManager::get_instance()->set_meta_state_machine(_meta_state_machine);
        ConfigManager::get_instance()->set_meta_state_machine(_meta_state_machine);
        PrivilegeManager::get_instance()->set_meta_state_machine(_meta_state_machine);
        _flush_bth.run([this]() { flush_memtable_thread(); });
        _init_success = true;
        return 0;
    }

    void MetaServer::flush_memtable_thread() {
        while (!_shutdown) {
            bthread_usleep_fast_shutdown(FLAGS_flush_memtable_interval_us, _shutdown);
            if (_shutdown) {
                return;
            }
            auto rocksdb = RocksStorage::get_instance();
            rocksdb::FlushOptions flush_options;
            auto status = rocksdb->flush(flush_options, rocksdb->get_meta_info_handle());
            if (!status.ok()) {
                TLOG_WARN("flush meta info to rocksdb fail, err_msg:{}", status.ToString());
            }
            status = rocksdb->flush(flush_options, rocksdb->get_raft_log_handle());
            if (!status.ok()) {
                TLOG_WARN("flush log_cf to rocksdb fail, err_msg:{}", status.ToString());
            }
        }
    }


    void MetaServer::meta_manager(google::protobuf::RpcController *controller,
                                  const EA::servlet::MetaManagerRequest *request,
                                  EA::servlet::MetaManagerResponse *response,
                                  google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        brpc::Controller *cntl =
                static_cast<brpc::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        if (request->op_type() == EA::servlet::OP_CREATE_USER
            || request->op_type() == EA::servlet::OP_DROP_USER
            || request->op_type() == EA::servlet::OP_ADD_PRIVILEGE
            || request->op_type() == EA::servlet::OP_DROP_PRIVILEGE) {
            PrivilegeManager::get_instance()->process_user_privilege(controller,
                                                                     request,
                                                                     response,
                                                                     done_guard.release());
            return;
        }
        if (request->op_type() == EA::servlet::OP_CREATE_NAMESPACE
            || request->op_type() == EA::servlet::OP_DROP_NAMESPACE
            || request->op_type() == EA::servlet::OP_MODIFY_NAMESPACE
            || request->op_type() == EA::servlet::OP_CREATE_ZONE
            || request->op_type() == EA::servlet::OP_DROP_ZONE
            || request->op_type() == EA::servlet::OP_MODIFY_ZONE
            || request->op_type() == EA::servlet::OP_CREATE_SERVLET
            || request->op_type() == EA::servlet::OP_DROP_SERVLET
            || request->op_type() == EA::servlet::OP_MODIFY_SERVLET
            || request->op_type() == EA::servlet::OP_ADD_INSTANCE
            || request->op_type() == EA::servlet::OP_DROP_INSTANCE
            || request->op_type() == EA::servlet::OP_UPDATE_INSTANCE
            || request->op_type() == EA::servlet::OP_MODIFY_RESOURCE_TAG
            || request->op_type() == EA::servlet::OP_UPDATE_MAIN_LOGICAL_ROOM) {
            SchemaManager::get_instance()->process_schema_info(controller,
                                                               request,
                                                               response,
                                                               done_guard.release());
            return;
        }
        if(request->op_type() == EA::servlet::OP_CREATE_CONFIG
            ||request->op_type() == EA::servlet::OP_REMOVE_CONFIG) {
            ConfigManager::get_instance()->process_schema_info(controller,
                                                               request,
                                                               response,
                                                               done_guard.release());
            return;
        }
        if (request->op_type() == EA::servlet::OP_GEN_ID_FOR_AUTO_INCREMENT
            || request->op_type() == EA::servlet::OP_UPDATE_FOR_AUTO_INCREMENT
            || request->op_type() == EA::servlet::OP_ADD_ID_FOR_AUTO_INCREMENT
            || request->op_type() == EA::servlet::OP_DROP_ID_FOR_AUTO_INCREMENT) {
            _auto_incr_state_machine->process(controller,
                                              request,
                                              response,
                                              done_guard.release());
            return;
        }


        TLOG_ERROR("request has wrong op_type:{} , log_id:{}",
                 request->op_type(), log_id);
        response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
        response->set_errmsg("invalid op_type");
        response->set_op_type(request->op_type());
    }

    void MetaServer::meta_query(google::protobuf::RpcController *controller,
                           const EA::servlet::QueryRequest *request,
                           EA::servlet::QueryResponse *response,
                           google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        brpc::Controller *cntl =
                static_cast<brpc::Controller *>(controller);
        const auto &remote_side_tmp = butil::endpoint2str(cntl->remote_side());
        const char *remote_side = remote_side_tmp.c_str();
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        TimeCost time_cost;
        response->set_errcode(EA::servlet::SUCCESS);
        response->set_errmsg("success");
        switch (request->op_type()) {
            case EA::servlet::QUERY_USER_PRIVILEGE: {
                QueryPrivilegeManager::get_instance()->get_user_info(request, response);
                break;
            }
            case EA::servlet::QUERY_NAMESPACE: {
                QueryNamespaceManager::get_instance()->get_namespace_info(request, response);
                break;
            }
            case EA::servlet::QUERY_ZONE: {
                QueryZoneManager::get_instance()->get_zone_info(request, response);
                break;
            }
            case EA::servlet::QUERY_SERVLET: {
                QueryServletManager::get_instance()->get_servlet_info(request, response);
                break;
            }
            case EA::servlet::QUERY_GET_CONFIG: {
                QueryConfigManager::get_instance()->get_config(request, response);
                break;
            }
            case EA::servlet::QUERY_LIST_CONFIG: {
                QueryConfigManager::get_instance()->list_config(request, response);
                break;
            }
            case EA::servlet::QUERY_LIST_CONFIG_VERSION: {
                QueryConfigManager::get_instance()->list_config_version(request, response);
                break;
            }

            case EA::servlet::QUERY_PRIVILEGE_FLATTEN: {
                QueryPrivilegeManager::get_instance()->get_flatten_servlet_privilege(request, response);
                break;
            }
            case EA::servlet::QUERY_INSTANCE: {
                QueryInstanceManager::get_instance()->query_instance(request, response);
                break;
            }
            case EA::servlet::QUERY_INSTANCE_FLATTEN: {
                QueryInstanceManager::get_instance()->query_instance_flatten(request, response);
                break;
            }

            default: {
                TLOG_WARN("invalid op_type, request:{} logid:{}",
                           request->ShortDebugString(), log_id);
                response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                response->set_errmsg("invalid op_type");
            }
        }
        TLOG_INFO("query op_type_name:{}, time_cost:{}, log_id:{}, ip:{}, request: {}",
                  EA::servlet::QueryOpType_Name(request->op_type()),
                  time_cost.get_time(), log_id, remote_side, request->ShortDebugString());
    }

    void MetaServer::raft_control(google::protobuf::RpcController *controller,
                                  const EA::servlet::RaftControlRequest *request,
                                  EA::servlet::RaftControlResponse *response,
                                  google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        if (request->region_id() == 0) {
            _meta_state_machine->raft_control(controller, request, response, done_guard.release());
            return;
        }
        if (request->region_id() == 1) {
            _auto_incr_state_machine->raft_control(controller, request, response, done_guard.release());
            return;
        }
        if (request->region_id() == 2) {
            _tso_state_machine->raft_control(controller, request, response, done_guard.release());
            return;
        }
        response->set_region_id(request->region_id());
        response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
        response->set_errmsg("unmatch region id");
        TLOG_ERROR("unmatch region_id in meta server, request: {}", request->ShortDebugString());
    }


    void MetaServer::tso_service(google::protobuf::RpcController *controller,
                                 const EA::servlet::TsoRequest *request,
                                 EA::servlet::TsoResponse *response,
                                 google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        brpc::Controller *cntl =
                static_cast<brpc::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        if (_tso_state_machine != nullptr) {
            _tso_state_machine->process(controller, request, response, done_guard.release());
        }
    }

    void MetaServer::shutdown_raft() {
        _shutdown = true;
        if (_meta_state_machine != nullptr) {
            _meta_state_machine->shutdown_raft();
        }
        if (_auto_incr_state_machine != nullptr) {
            _auto_incr_state_machine->shutdown_raft();
        }
        if (_tso_state_machine != nullptr) {
            _tso_state_machine->shutdown_raft();
        }
    }

    bool MetaServer::have_data() {
        return _meta_state_machine->have_data()
               && _auto_incr_state_machine->have_data()
               && _tso_state_machine->have_data();
    }

    void MetaServer::close() {
        _flush_bth.join();
        TLOG_INFO("MetaServer flush joined");
    }

}  // namespace EA::servlet
