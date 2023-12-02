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


#include "ea/meta_server/instance_manager.h"
#include "ea/meta_server/schema_manager.h"
#include "ea/meta_server/meta_rocksdb.h"

namespace EA::servlet {

    void InstanceManager::add_instance(const EA::servlet::MetaManagerRequest &request, braft::Closure *done) {
        auto &instance_info = const_cast<EA::servlet::ServletInstance &>(request.instance_info());
        const std::string &address = instance_info.address();

        auto ret = SchemaManager::get_instance()->check_and_get_for_instance(instance_info);
        if (ret < 0) {
            TLOG_WARN("request not illegal, request:{}", request.ShortDebugString());
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "request invalid");
            return;
        }
        if (_instance_info.find(address) != _instance_info.end()) {
            TLOG_WARN("request instance:{} has been existed", address);
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "instance already existed");
            return;
        }
        auto it = _removed_instance.find(address);
        if(it != _removed_instance.end()) {
            if(it->second.get_time_s() < int64_t(3600)) {
                TLOG_WARN("request instance:{} has been removed in 1 hour", address);
                IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "removed in 1 hour");
                return;
            }
        }
        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare address info
        instance_info.set_version(1);

        std::string instance_value;
        if (!instance_info.SerializeToString(&instance_value)) {
            TLOG_WARN("request serializeToArray fail, request:{}", request.ShortDebugString());
            IF_DONE_SET_RESPONSE(done, EA::servlet::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_instance_key(address));
        rocksdb_values.push_back(instance_value);

        // save to rocksdb

        ret = MetaRocksdb::get_instance()->put_meta_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update values in memory
        BAIDU_SCOPED_LOCK(_instance_mutex);
        set_instance_info(instance_info);
        IF_DONE_SET_RESPONSE(done, EA::servlet::SUCCESS, "success");
        TLOG_INFO("create instance success, request:{}", request.ShortDebugString());
    }

    void InstanceManager::drop_instance(const EA::servlet::MetaManagerRequest &request, braft::Closure *done) {
        auto &instance_info = request.instance_info();
        std::string address = instance_info.address();
        if (_instance_info.find(address) == _instance_info.end()) {
            TLOG_WARN("request address:{} not exist", address);
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "address not exist");
            return;
        }

        std::string instance_key = construct_instance_key(address);

        int ret = MetaRocksdb::get_instance()->delete_meta_info(std::vector<std::string>{instance_key});
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::INTERNAL_ERROR, "write db fail");
            return;
        }

        remove_instance_info(address);
        IF_DONE_SET_RESPONSE(done, EA::servlet::SUCCESS, "success");
        TLOG_INFO("drop instance success, request:{}", request.ShortDebugString());
    }

    void InstanceManager::update_instance(const EA::servlet::MetaManagerRequest &request, braft::Closure *done) {
        auto &instance_info = request.instance_info();
        std::string address = instance_info.address();
        if (_instance_info.find(address) == _instance_info.end()) {
            TLOG_WARN("request address:{} not exist", address);
            IF_DONE_SET_RESPONSE(done, EA::servlet::INPUT_PARAM_ERROR, "address not exist");
            return;
        }

        auto tmp_instance_pb = _instance_info[address];
        if(instance_info.has_status()) {
            tmp_instance_pb.set_status(instance_info.status());
        }
        if(instance_info.has_color()) {
            tmp_instance_pb.set_color(instance_info.color());
        }

        if(instance_info.has_env()) {
            tmp_instance_pb.set_env(instance_info.env());
        }

        if(instance_info.has_weight()) {
            tmp_instance_pb.set_weight(instance_info.weight());
        }
        tmp_instance_pb.set_version(tmp_instance_pb.version() + 1);
        std::string instance_key = construct_instance_key(address);

        std::string instance_value;
        if (!tmp_instance_pb.SerializeToString(&instance_value)) {
            TLOG_WARN("request serializeToArray fail, request:{}", request.ShortDebugString());
            IF_DONE_SET_RESPONSE(done, EA::servlet::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        int ret = MetaRocksdb::get_instance()->put_meta_info(instance_key, instance_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::servlet::INTERNAL_ERROR, "write db fail");
            return;
        }

        BAIDU_SCOPED_LOCK(_instance_mutex);
        set_instance_info(tmp_instance_pb);
        IF_DONE_SET_RESPONSE(done, EA::servlet::SUCCESS, "success");
        TLOG_INFO("drop instance success, request:{}", request.ShortDebugString());
    }

    int InstanceManager::load_instance_snapshot(const std::string &value) {
        EA::servlet::ServletInstance instance_pb;
        if (!instance_pb.ParseFromString(value)) {
            TLOG_ERROR("parse from pb fail when load instance snapshot, value: {}", value);
            return -1;
        }
        TLOG_WARN("instance snapshot:{}", instance_pb.ShortDebugString());
        set_instance_info(instance_pb);
        return 0;
    }

    void InstanceManager::set_instance_info(const EA::servlet::ServletInstance &instance_info) {
        _instance_info[instance_info.address()] = instance_info;
        _removed_instance.erase(instance_info.address());
        if(_namespace_instance.find(instance_info.namespace_name()) == _namespace_instance.end()) {
            _namespace_instance[instance_info.namespace_name()] = turbo::flat_hash_set<std::string>();
        }
        _namespace_instance[instance_info.namespace_name()].insert(instance_info.address());

        auto zone_key = ZoneManager::make_zone_key(instance_info.namespace_name(), instance_info.zone_name());
        if(_zone_instance.find(zone_key) == _zone_instance.end()) {
            _zone_instance[zone_key] = turbo::flat_hash_set<std::string>();
        }
        _zone_instance[zone_key].insert(instance_info.address());

        auto servlet_key = ServletManager::make_servlet_key(zone_key, instance_info.servlet_name());
        if(_servlet_instance.find(servlet_key) == _servlet_instance.end()) {
            _servlet_instance[servlet_key] = turbo::flat_hash_set<std::string>();
        }
        _servlet_instance[servlet_key].insert(instance_info.address());
    }

    void InstanceManager::remove_instance_info(const std::string &address) {
        BAIDU_SCOPED_LOCK(_instance_mutex);
        auto &info = _instance_info[address];
        _removed_instance[address] = TimeCost();
        _namespace_instance.erase(info.namespace_name());
        auto zone_key = ZoneManager::make_zone_key(info.namespace_name(), info.zone_name());
        _zone_instance.erase(zone_key);
        auto servlet_key = ServletManager::make_servlet_key(zone_key, info.servlet_name());
        _servlet_instance.erase(servlet_key);
        _instance_info.erase(address);

    }

    int InstanceManager::load_snapshot() {
        BAIDU_SCOPED_LOCK( InstanceManager::get_instance()->_instance_mutex);
        TLOG_INFO("start to load instance snapshot");
        clear();
        std::string config_prefix = MetaConstants::DISCOVERY_IDENTIFY;
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        RocksStorage *db = RocksStorage::get_instance();
        std::unique_ptr<rocksdb::Iterator> iter(
                db->new_iterator(read_options, db->get_meta_info_handle()));
        iter->Seek(config_prefix);
        for (; iter->Valid(); iter->Next()) {
            if(load_instance_snapshot(iter->value().ToString()) != 0) {
                return -1;
            }
        }
        TLOG_INFO("load config instance done");
        return 0;
    }

}  // namespace EA::servlet
