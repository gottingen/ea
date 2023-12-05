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


#include "ea/discovery/discovery_manager.h"
#include "ea/discovery/zone_manager.h"
#include "ea/discovery/servlet_manager.h"
#include "ea/discovery/instance_manager.h"
#include "ea/discovery/namespace_manager.h"
#include "ea/storage/rocks_storage.h"
#include "ea/base/scope_exit.h"

namespace EA::discovery {

    void DiscoveryManager::process_schema_info(google::protobuf::RpcController *controller,
                                            const EA::discovery::DiscoveryManagerRequest *request,
                                            EA::discovery::DiscoveryManagerResponse *response,
                                            google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        if (!_discovery_state_machine->is_leader()) {
            if (response) {
                response->set_errcode(EA::NOT_LEADER);
                response->set_errmsg("not leader");
                response->set_leader(butil::endpoint2str(_discovery_state_machine->get_leader()).c_str());
            }
            TLOG_WARN("discovery state machine is not leader, request: {}", request->ShortDebugString());
            return;
        }
        uint64_t log_id = 0;
        brpc::Controller *cntl = nullptr;
        if (controller != nullptr) {
            cntl = static_cast<brpc::Controller *>(controller);
            if (cntl->has_log_id()) {
                log_id = cntl->log_id();
            }
        }
        ON_SCOPE_EXIT(([cntl, log_id, response]() {
            if (response != nullptr && response->errcode() != EA::SUCCESS) {
                const auto &remote_side_tmp = butil::endpoint2str(cntl->remote_side());
                const char *remote_side = remote_side_tmp.c_str();
                TLOG_WARN("response error, remote_side:{}, log_id:{}", remote_side, log_id);
            }
        }));
        switch (request->op_type()) {
            case EA::discovery::OP_CREATE_NAMESPACE:
            case EA::discovery::OP_MODIFY_NAMESPACE:
            case EA::discovery::OP_DROP_NAMESPACE: {
                if (!request->has_namespace_info()) {
                    ERROR_SET_RESPONSE(response, EA::INPUT_PARAM_ERROR,
                                       "no namespace_info", request->op_type(), log_id);
                    return;

                }
                _discovery_state_machine->process(controller, request, response, done_guard.release());
                return;
                case EA::discovery::OP_CREATE_ZONE:
                case EA::discovery::OP_MODIFY_ZONE:
                case EA::discovery::OP_DROP_ZONE: {
                    if (!request->has_zone_info()) {
                        ERROR_SET_RESPONSE(response, EA::INPUT_PARAM_ERROR,
                                           "no zone_info", request->op_type(), log_id);
                        return;
                    }
                    _discovery_state_machine->process(controller, request, response, done_guard.release());
                    return;
                }
                case EA::discovery::OP_CREATE_SERVLET:
                case EA::discovery::OP_MODIFY_SERVLET:
                case EA::discovery::OP_DROP_SERVLET: {
                    if (!request->has_servlet_info()) {
                        ERROR_SET_RESPONSE(response, EA::INPUT_PARAM_ERROR,
                                           "no servlet info", request->op_type(), log_id);
                        return;
                    }
                    _discovery_state_machine->process(controller, request, response, done_guard.release());
                    return;
                }
                case EA::discovery::OP_ADD_INSTANCE:
                case EA::discovery::OP_DROP_INSTANCE:
                case EA::discovery::OP_UPDATE_INSTANCE: {
                    if (!request->has_instance_info()) {
                        ERROR_SET_RESPONSE(response, EA::INPUT_PARAM_ERROR,
                                           "no instance info", request->op_type(), log_id);
                        return;
                    }
                    auto &instance = request->instance_info();
                    if (!instance.has_namespace_name()
                        || !instance.has_zone_name()
                        || !instance.has_servlet_name()
                        || !instance.has_address()
                        || !instance.has_env()) {
                        ERROR_SET_RESPONSE(response, EA::INPUT_PARAM_ERROR,
                                           "no required namespace zone or servlet info", request->op_type(), log_id);
                        return;
                    }
                    _discovery_state_machine->process(controller, request, response, done_guard.release());
                    return;
                }
            }

            default:
                ERROR_SET_RESPONSE(response, EA::INPUT_PARAM_ERROR,
                                   "invalid op_type", request->op_type(), log_id);
                return;
        }
    }


    int DiscoveryManager::check_and_get_for_privilege(EA::discovery::UserPrivilege &user_privilege) {
        std::string namespace_name = user_privilege.namespace_name();
        int64_t namespace_id = NamespaceManager::get_instance()->get_namespace_id(namespace_name);
        if (namespace_id == 0) {
            TLOG_ERROR("namespace not exist, namespace:{}, request：{}",
                       namespace_name,
                       user_privilege.ShortDebugString());
            return -1;
        }
        user_privilege.set_namespace_id(namespace_id);

        for (auto &pri_zone: *user_privilege.mutable_privilege_zone()) {
            std::string base_name = ZoneManager::make_zone_key(namespace_name, pri_zone.zone());
            int64_t zone_id = ZoneManager::get_instance()->get_zone_id(base_name);
            if (zone_id == 0) {
                TLOG_ERROR("zone:{} not exist, namespace:{}, request：{}",
                           base_name, namespace_name,
                           user_privilege.ShortDebugString());
                return -1;
            }
            pri_zone.set_zone_id(zone_id);
        }
        for (auto &pri_servlet: *user_privilege.mutable_privilege_servlet()) {
            std::string base_name = ZoneManager::make_zone_key(namespace_name, pri_servlet.zone());
            std::string servlet_name = ServletManager::make_servlet_key(base_name, pri_servlet.servlet_name());
            int64_t zone_id = ZoneManager::get_instance()->get_zone_id(base_name);
            if (zone_id == 0) {
                TLOG_ERROR("zone:{} not exist, namespace:{}, request：{}",
                           base_name, namespace_name,
                           user_privilege.ShortDebugString());
                return -1;
            }
            int64_t servlet_id = ServletManager::get_instance()->get_servlet_id(servlet_name);
            if (servlet_id == 0) {
                TLOG_ERROR("table_name:{} not exist, zone:{} namespace:{}, request：{}",
                           servlet_name, base_name,
                           namespace_name, user_privilege.ShortDebugString());
                return -1;
            }
            pri_servlet.set_zone_id(zone_id);
            pri_servlet.set_servlet_id(servlet_id);
        }
        return 0;
    }

    int DiscoveryManager::load_snapshot() {
        TLOG_INFO("DiscoveryManager start load_snapshot");
        NamespaceManager::get_instance()->clear();
        ZoneManager::get_instance()->clear();
        ServletManager::get_instance()->clear();
        InstanceManager::get_instance()->clear();
        //创建一个snapshot
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<rocksdb::Iterator> iter(
                RocksStorage::get_instance()->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(DiscoveryConstants::DISCOVERY_TREE_IDENTIFY);
        std::string max_id_prefix = DiscoveryConstants::DISCOVERY_TREE_IDENTIFY;
        max_id_prefix += DiscoveryConstants::DISCOVERY_TREE_MAX_ID_IDENTIFY;

        std::string namespace_prefix = DiscoveryConstants::DISCOVERY_TREE_IDENTIFY;
        namespace_prefix += DiscoveryConstants::DISCOVERY_TREE_NAMESPACE_IDENTIFY;


        std::string zone_prefix = DiscoveryConstants::DISCOVERY_TREE_IDENTIFY;
        zone_prefix += DiscoveryConstants::DISCOVERY_TREE_ZONE_IDENTIFY;

        std::string servlet_prefix = DiscoveryConstants::DISCOVERY_TREE_IDENTIFY;
        servlet_prefix += DiscoveryConstants::DISCOVERY_TREE_SERVLET_IDENTIFY;


        for (; iter->Valid(); iter->Next()) {
            int ret = 0;
            if (iter->key().starts_with(zone_prefix)) {
                ret = ZoneManager::get_instance()->load_zone_snapshot(iter->value().ToString());
            } else if (iter->key().starts_with(servlet_prefix)) {
                ret = ServletManager::get_instance()->load_servlet_snapshot(iter->value().ToString());
            } else if (iter->key().starts_with(namespace_prefix)) {
                ret = NamespaceManager::get_instance()->load_namespace_snapshot(iter->value().ToString());
            } else if (iter->key().starts_with(max_id_prefix)) {
                ret = load_max_id_snapshot(max_id_prefix, iter->key().ToString(), iter->value().ToString());
            } else {
                TLOG_ERROR("unknown schema info when load snapshot, key:{}", iter->key().data());
            }
            if (ret != 0) {
                TLOG_ERROR("load snapshot fail, key:{}, value:{}",
                           iter->key().data(),
                           iter->value().data());
                return -1;
            }
        }
        TLOG_INFO("DiscoveryManager load_snapshot done...");
        return 0;
    }

    int DiscoveryManager::check_and_get_for_instance(EA::discovery::ServletInstance &instance) {
        std::string namespace_name = instance.namespace_name();
        int64_t namespace_id = NamespaceManager::get_instance()->get_namespace_id(namespace_name);
        if (namespace_id == 0) {
            TLOG_ERROR("namespace not exist, namespace:{}, request：{}",
                       namespace_name,
                       instance.ShortDebugString());
            return -1;
        }
        instance.set_namespace_id(namespace_id);

        std::string base_name = ZoneManager::make_zone_key(namespace_name, instance.zone_name());
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(base_name);
        if (zone_id == 0) {
            TLOG_ERROR("zone:{} not exist, namespace:{}, request：{}",
                       base_name, namespace_name,
                       instance.ShortDebugString());
            return -1;
        }
        instance.set_zone_id(zone_id);

        std::string servlet_name = ServletManager::make_servlet_key(base_name, instance.servlet_name());
        int64_t servlet_id = ServletManager::get_instance()->get_servlet_id(servlet_name);
        if (servlet_id == 0) {
            TLOG_ERROR("servlet:{} not exist, zone:{} namespace:{}, request：{}",
                       servlet_name, base_name,
                       namespace_name, instance.ShortDebugString());
            return -1;
        }
        instance.set_zone_id(zone_id);
        instance.set_servlet_id(servlet_id);

        return 0;
    }


    int DiscoveryManager::load_max_id_snapshot(const std::string &max_id_prefix,
                                            const std::string &key,
                                            const std::string &value) {
        std::string max_key(key, max_id_prefix.size());
        int64_t *max_id = (int64_t *) (value.c_str());
        if (max_key == DiscoveryConstants::MAX_NAMESPACE_ID_KEY) {
            NamespaceManager::get_instance()->set_max_namespace_id(*max_id);
            TLOG_WARN("max_namespace_id:{}", *max_id);
            return 0;
        }
        if (max_key == DiscoveryConstants::MAX_ZONE_ID_KEY) {
            ZoneManager::get_instance()->set_max_zone_id(*max_id);
            TLOG_WARN("max_zone_id:{}", *max_id);
            return 0;
        }
        if (max_key == DiscoveryConstants::MAX_SERVLET_ID_KEY) {
            ServletManager::get_instance()->set_max_servlet_id(*max_id);
            TLOG_WARN("max_servlet_id:{}", *max_id);
            return 0;
        }
        return 0;
    }

}  // namespace EA::discovery
