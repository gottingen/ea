// Copyright 2023 The Elastic Architecture Infrastructure Authors.
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

#include "ea/meta_server/config_manager.h"
#include "ea/meta_server/base_state_machine.h"
#include "ea/meta_server/meta_rocksdb.h"
#include "ea/meta_server/meta_constants.h"
#include "ea/base/scope_exit.h"

namespace EA::servlet {

    turbo::ModuleVersion ConfigManager::kDefaultVersion(0,0,1);

    void ConfigManager::process_schema_info(google::protobuf::RpcController *controller,
                             const EA::servlet::MetaManagerRequest *request,
                             EA::servlet::MetaManagerResponse *response,
                             google::protobuf::Closure *done) {
        brpc::ClosureGuard done_guard(done);
        if (!_meta_state_machine->is_leader()) {
            if (response) {
                response->set_errcode(EA::servlet::NOT_LEADER);
                response->set_errmsg("not leader");
                response->set_leader(butil::endpoint2str(_meta_state_machine->get_leader()).c_str());
            }
            TLOG_WARN("meta state machine is not leader, request: {}", request->ShortDebugString());
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
            if (response != nullptr && response->errcode() != EA::servlet::SUCCESS) {
                const auto &remote_side_tmp = butil::endpoint2str(cntl->remote_side());
                const char *remote_side = remote_side_tmp.c_str();
                TLOG_WARN("response error, remote_side:{}, log_id:{}", remote_side, log_id);
            }
        }));

        switch (request->op_type()) {
            case EA::servlet::OP_CREATE_CONFIG:
            case EA::servlet::OP_REMOVE_CONFIG:
                if(!request->has_config_info()) {
                    ERROR_SET_RESPONSE(response, EA::servlet::INPUT_PARAM_ERROR,
                                       "no config_info", request->op_type(), log_id);
                    return;
                }
                _meta_state_machine->process(controller, request, response, done_guard.release());
                return;
            default:
                ERROR_SET_RESPONSE(response, EA::servlet::INPUT_PARAM_ERROR,
                                   "invalid op_type", request->op_type(), log_id);
                return;
        }

    }
    void ConfigManager::create_config(const ::EA::servlet::MetaManagerRequest &request, braft::Closure *done) {
        auto &create_request = request.config_info();
        auto &name = create_request.name();
        turbo::ModuleVersion version = kDefaultVersion;

        if(create_request.has_version()) {
            version = turbo::ModuleVersion(create_request.version().major(), create_request.version().minor(),
                    create_request.version().patch());
        }


        BAIDU_SCOPED_LOCK(_config_mutex);
        if (_configs.find(name) == _configs.end()) {
            _configs[name] = std::map<turbo::ModuleVersion, EA::servlet::ConfigInfo>();
        }
        auto it = _configs.find(name);
        // do not rewrite.
        if (it->second.find(version) != it->second.end()) {
            /// already exists
            TLOG_INFO("config :{} version: {} exist", name, version.to_string());
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "config already exist");
            return;
        }
        if(!it->second.empty() && it->second.rbegin()->first >= version) {
            /// Version numbers must increase monotonically
            TLOG_INFO("config :{} version: {} must be larger than current:{}", name, version.to_string(), it->second.rbegin()->first.to_string());
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "Version numbers must increase monotonically");
            return;
        }
        std::string rocks_key = make_config_key(name, version);
        std::string rocks_value;
        if (!create_request.SerializeToString(&rocks_value)) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        int ret = MetaRocksdb::get_instance()->put_meta_info(rocks_key, rocks_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::INTERNAL_ERROR, "write db fail");
            return;
        }
        it->second[version] = create_request;
        TLOG_INFO("config :{} version: {} create", name, version.to_string());
        IF_DONE_SET_RESPONSE(done, EA::servlet::SUCCESS, "success");
    }


    void ConfigManager::remove_config(const ::EA::servlet::MetaManagerRequest &request, braft::Closure *done) {
        auto &remove_request = request.config_info();
        auto &name = remove_request.name();
        bool remove_signal = remove_request.has_version();
        BAIDU_SCOPED_LOCK(_config_mutex);
        if (!remove_signal) {
            remove_config_all(request, done);
            return;
        }
        auto it = _configs.find(name);
        if (it == _configs.end()) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::PARSE_TO_PB_FAIL, "config not exist");
            return;
        }
        turbo::ModuleVersion version(remove_request.version().major(), remove_request.version().minor(),
                                     remove_request.version().patch());

        if (it->second.find(version) == it->second.end()) {
            /// not exists
            TLOG_INFO("config :{} version: {} not exist", name, version.to_string());
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "config not exist");
        }

        std::string rocks_key = make_config_key(name, version);
        int ret = MetaRocksdb::get_instance()->delete_meta_info(std::vector{rocks_key});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        it->second.erase(version);
        if(it->second.empty()) {
            _configs.erase(name);
        }
        IF_DONE_SET_RESPONSE(done, EA::servlet::SUCCESS, "success");
    }

    void ConfigManager::remove_config_all(const ::EA::servlet::MetaManagerRequest &request, braft::Closure *done) {
        auto &remove_request = request.config_info();
        auto &name = remove_request.name();
        auto it = _configs.find(name);
        if (it == _configs.end()) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::PARSE_TO_PB_FAIL, "config not exist");
            return;
        }
        std::vector<std::string> del_keys;

        for(auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            std::string key = make_config_key(name, vit->first);
            del_keys.push_back(key);
        }

        int ret = MetaRocksdb::get_instance()->delete_meta_info(del_keys);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        _configs.erase(name);
        IF_DONE_SET_RESPONSE(done, EA::servlet::SUCCESS, "success");
    }

    int ConfigManager::load_snapshot() {
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        TLOG_INFO("start to load config snapshot");
        _configs.clear();
        std::string config_prefix = MetaConstants::CONFIG_IDENTIFY;
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<rocksdb::Iterator> iter(
                db->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(config_prefix);
        for (; iter->Valid(); iter->Next()) {
            if(load_config_snapshot(iter->value().ToString()) != 0) {
                return -1;
            }
        }
        TLOG_INFO("load config snapshot done");
        return 0;
    }

    int ConfigManager::load_config_snapshot(const std::string &value) {
        EA::servlet::ConfigInfo config_pb;
        if (!config_pb.ParseFromString(value)) {
            TLOG_ERROR("parse from pb fail when load config snapshot, key:{}", value);
            return -1;
        }
        ///TLOG_INFO("load config:{}", config_pb.name());
        if(_configs.find(config_pb.name()) == _configs.end()) {
            _configs[config_pb.name()] = std::map<turbo::ModuleVersion, EA::servlet::ConfigInfo>();
        }
        auto it = _configs.find(config_pb.name());
        turbo::ModuleVersion version(config_pb.version().major(), config_pb.version().minor(),
                                     config_pb.version().patch());
        it->second[version] = config_pb;
        return 0;
    }

    std::string ConfigManager::make_config_key(const std::string &name, const turbo::ModuleVersion &version) {
        return MetaConstants::CONFIG_IDENTIFY + name + version.to_string();
    }

}  // namespace EA::servlet