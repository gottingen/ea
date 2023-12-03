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


#include "ea/discovery/discovery_server.h"
#include "ea/discovery/auto_incr_state_machine.h"
#include "ea/discovery/tso_state_machine.h"
#include "ea/discovery/discovery_state_machine.h"
#include "ea/discovery/privilege_manager.h"
#include "ea/discovery/schema_manager.h"
#include "ea/discovery/config_manager.h"
#include "ea/discovery/query_config_manager.h"
#include "ea/discovery/query_privilege_manager.h"
#include "ea/discovery/query_namespace_manager.h"
#include "ea/discovery/query_instance_manager.h"
#include "ea/discovery/query_zone_manager.h"
#include "ea/discovery/query_servlet_manager.h"
#include "ea/discovery/discovery_rocksdb.h"

namespace EA::discovery {

    DiscoveryServer::~DiscoveryServer() {}

    int DiscoveryServer::init(const std::vector<braft::PeerId> &peers) {
        auto ret = DiscoveryRocksdb::get_instance()->init();
        if (ret < 0) {
            TLOG_ERROR("rocksdb init fail");
            return -1;
        }
        butil::EndPoint addr;
        butil::str2endpoint(FLAGS_discovery_listen.c_str(), &addr);
        //addr.ip = butil::my_ip();
        //addr.port = FLAGS_discovery_port;
        braft::PeerId peer_id(addr, 0);
        _discovery_state_machine = new(std::nothrow)DiscoveryStateMachine(peer_id);
        if (_discovery_state_machine == nullptr) {
            TLOG_ERROR("new discovery_state_machine fail");
            return -1;
        }
        //state_machine初始化
        ret = _discovery_state_machine->init(peers);
        if (ret != 0) {
            TLOG_ERROR("discovery state machine init fail");
            return -1;
        }
        TLOG_WARN("discovery state machine init success");

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

        SchemaManager::get_instance()->set_discovery_state_machine(_discovery_state_machine);
        ConfigManager::get_instance()->set_discovery_state_machine(_discovery_state_machine);
        PrivilegeManager::get_instance()->set_discovery_state_machine(_discovery_state_machine);
        _flush_bth.run([this]() { flush_memtable_thread(); });
        _init_success = true;
        return 0;
    }

    void DiscoveryServer::flush_memtable_thread() {
        while (!_shutdown) {
            bthread_usleep_fast_shutdown(FLAGS_flush_memtable_interval_us, _shutdown);
            if (_shutdown) {
                return;
            }
            auto rocksdb = RocksStorage::get_instance();
            rocksdb::FlushOptions flush_options;
            auto status = rocksdb->flush(flush_options, rocksdb->get_meta_info_handle());
            if (!status.ok()) {
                TLOG_WARN("flush discovery info to rocksdb fail, err_msg:{}", status.ToString());
            }
            status = rocksdb->flush(flush_options, rocksdb->get_raft_log_handle());
            if (!status.ok()) {
                TLOG_WARN("flush log_cf to rocksdb fail, err_msg:{}", status.ToString());
            }
        }
    }


    void DiscoveryServer::discovery_manager(google::protobuf::RpcController *controller,
                                  const EA::discovery::DiscoveryManagerRequest *request,
                                  EA::discovery::DiscoveryManagerResponse *response,
                                  google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        brpc::Controller *cntl =
                static_cast<brpc::Controller *>(controller);
        uint64_t log_id = 0;
        if (cntl->has_log_id()) {
            log_id = cntl->log_id();
        }
        RETURN_IF_NOT_INIT(_init_success, response, log_id);
        if (request->op_type() == EA::discovery::OP_CREATE_USER
            || request->op_type() == EA::discovery::OP_DROP_USER
            || request->op_type() == EA::discovery::OP_ADD_PRIVILEGE
            || request->op_type() == EA::discovery::OP_DROP_PRIVILEGE) {
            PrivilegeManager::get_instance()->process_user_privilege(controller,
                                                                     request,
                                                                     response,
                                                                     done_guard.release());
            return;
        }
        if (request->op_type() == EA::discovery::OP_CREATE_NAMESPACE
            || request->op_type() == EA::discovery::OP_DROP_NAMESPACE
            || request->op_type() == EA::discovery::OP_MODIFY_NAMESPACE
            || request->op_type() == EA::discovery::OP_CREATE_ZONE
            || request->op_type() == EA::discovery::OP_DROP_ZONE
            || request->op_type() == EA::discovery::OP_MODIFY_ZONE
            || request->op_type() == EA::discovery::OP_CREATE_SERVLET
            || request->op_type() == EA::discovery::OP_DROP_SERVLET
            || request->op_type() == EA::discovery::OP_MODIFY_SERVLET
            || request->op_type() == EA::discovery::OP_ADD_INSTANCE
            || request->op_type() == EA::discovery::OP_DROP_INSTANCE
            || request->op_type() == EA::discovery::OP_UPDATE_INSTANCE
            || request->op_type() == EA::discovery::OP_MODIFY_RESOURCE_TAG
            || request->op_type() == EA::discovery::OP_UPDATE_MAIN_LOGICAL_ROOM) {
            SchemaManager::get_instance()->process_schema_info(controller,
                                                               request,
                                                               response,
                                                               done_guard.release());
            return;
        }
        if(request->op_type() == EA::discovery::OP_CREATE_CONFIG
            ||request->op_type() == EA::discovery::OP_REMOVE_CONFIG) {
            ConfigManager::get_instance()->process_schema_info(controller,
                                                               request,
                                                               response,
                                                               done_guard.release());
            return;
        }
        if (request->op_type() == EA::discovery::OP_GEN_ID_FOR_AUTO_INCREMENT
            || request->op_type() == EA::discovery::OP_UPDATE_FOR_AUTO_INCREMENT
            || request->op_type() == EA::discovery::OP_ADD_ID_FOR_AUTO_INCREMENT
            || request->op_type() == EA::discovery::OP_DROP_ID_FOR_AUTO_INCREMENT) {
            _auto_incr_state_machine->process(controller,
                                              request,
                                              response,
                                              done_guard.release());
            return;
        }


        TLOG_ERROR("request has wrong op_type:{} , log_id:{}",
                 request->op_type(), log_id);
        response->set_errcode(EA::INPUT_PARAM_ERROR);
        response->set_errmsg("invalid op_type");
        response->set_op_type(request->op_type());
    }

    void DiscoveryServer::discovery_query(google::protobuf::RpcController *controller,
                           const EA::discovery::DiscoveryQueryRequest *request,
                           EA::discovery::DiscoveryQueryResponse *response,
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
        response->set_errcode(EA::SUCCESS);
        response->set_errmsg("success");
        switch (request->op_type()) {
            case EA::discovery::QUERY_USER_PRIVILEGE: {
                QueryPrivilegeManager::get_instance()->get_user_info(request, response);
                break;
            }
            case EA::discovery::QUERY_NAMESPACE: {
                QueryNamespaceManager::get_instance()->get_namespace_info(request, response);
                break;
            }
            case EA::discovery::QUERY_ZONE: {
                QueryZoneManager::get_instance()->get_zone_info(request, response);
                break;
            }
            case EA::discovery::QUERY_SERVLET: {
                QueryServletManager::get_instance()->get_servlet_info(request, response);
                break;
            }
            case EA::discovery::QUERY_GET_CONFIG: {
                QueryConfigManager::get_instance()->get_config(request, response);
                break;
            }
            case EA::discovery::QUERY_LIST_CONFIG: {
                QueryConfigManager::get_instance()->list_config(request, response);
                break;
            }
            case EA::discovery::QUERY_LIST_CONFIG_VERSION: {
                QueryConfigManager::get_instance()->list_config_version(request, response);
                break;
            }

            case EA::discovery::QUERY_PRIVILEGE_FLATTEN: {
                QueryPrivilegeManager::get_instance()->get_flatten_servlet_privilege(request, response);
                break;
            }
            case EA::discovery::QUERY_INSTANCE: {
                QueryInstanceManager::get_instance()->query_instance(request, response);
                break;
            }
            case EA::discovery::QUERY_INSTANCE_FLATTEN: {
                QueryInstanceManager::get_instance()->query_instance_flatten(request, response);
                break;
            }

            default: {
                TLOG_WARN("invalid op_type, request:{} logid:{}",
                           request->ShortDebugString(), log_id);
                response->set_errcode(EA::INPUT_PARAM_ERROR);
                response->set_errmsg("invalid op_type");
            }
        }
        TLOG_INFO("query op_type_name:{}, time_cost:{}, log_id:{}, ip:{}, request: {}",
                  EA::discovery::QueryOpType_Name(request->op_type()),
                  time_cost.get_time(), log_id, remote_side, request->ShortDebugString());
    }

    void DiscoveryServer::raft_control(google::protobuf::RpcController *controller,
                                  const EA::RaftControlRequest *request,
                                  EA::RaftControlResponse *response,
                                  google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        if (request->region_id() == 0) {
            _discovery_state_machine->raft_control(controller, request, response, done_guard.release());
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
        response->set_errcode(EA::INPUT_PARAM_ERROR);
        response->set_errmsg("unmatch region id");
        TLOG_ERROR("unmatch region_id in discovery server, request: {}", request->ShortDebugString());
    }


    void DiscoveryServer::tso_service(google::protobuf::RpcController *controller,
                                 const EA::discovery::TsoRequest *request,
                                 EA::discovery::TsoResponse *response,
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

    void DiscoveryServer::shutdown_raft() {
        _shutdown = true;
        if (_discovery_state_machine != nullptr) {
            _discovery_state_machine->shutdown_raft();
        }
        if (_auto_incr_state_machine != nullptr) {
            _auto_incr_state_machine->shutdown_raft();
        }
        if (_tso_state_machine != nullptr) {
            _tso_state_machine->shutdown_raft();
        }
    }

    bool DiscoveryServer::have_data() {
        return _discovery_state_machine->have_data()
               && _auto_incr_state_machine->have_data()
               && _tso_state_machine->have_data();
    }

    void DiscoveryServer::close() {
        _flush_bth.join();
        TLOG_INFO("DiscoveryServer flush joined");
    }

}  // namespace EA::discovery
