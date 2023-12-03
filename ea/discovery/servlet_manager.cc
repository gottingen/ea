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

#include "ea/discovery/servlet_manager.h"
#include "ea/discovery/zone_manager.h"
#include "ea/discovery/base_state_machine.h"
#include "ea/discovery/discovery_rocksdb.h"
#include "ea/discovery/namespace_manager.h"

namespace EA::discovery {
    void ServletManager::create_servlet(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done) {
        // check legal
        auto &servlet_info = const_cast<EA::discovery::ServletInfo &>(request.servlet_info());
        std::string namespace_name = servlet_info.namespace_name();
        std::string zone_name = namespace_name + "\001" + servlet_info.zone();
        std::string servlet_name = zone_name + "\001" + servlet_info.servlet_name();
        int64_t namespace_id = NamespaceManager::get_instance()->get_namespace_id(namespace_name);
        if (namespace_id == 0) {
            TLOG_WARN("request namespace:{} not exist", namespace_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(zone_name);
        if (zone_id == 0) {
            TLOG_WARN("request zone:{} not exist", zone_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }

        if (_servlet_id_map.find(servlet_name) != _servlet_id_map.end()) {
            TLOG_WARN("request zone:{} already exist", servlet_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "servlet already exist");
            return;
        }

        std::vector<std::string> rocksdb_keys;
        std::vector<std::string> rocksdb_values;

        // prepare zone info
        int64_t tmp_servlet_id = _max_servlet_id + 1;
        servlet_info.set_servlet_id(tmp_servlet_id);
        servlet_info.set_zone_id(zone_id);
        servlet_info.set_namespace_id(namespace_id);

        EA::discovery::NameSpaceInfo namespace_info;
        if (NamespaceManager::get_instance()->get_namespace_info(namespace_id, namespace_info) == 0) {
            if (!servlet_info.has_resource_tag() && namespace_info.resource_tag() != "") {
                servlet_info.set_resource_tag(namespace_info.resource_tag());
            }
        }
        servlet_info.set_version(1);

        std::string servlet_value;
        if (!servlet_info.SerializeToString(&servlet_value)) {
            TLOG_WARN("request serializeToArray fail, request:{}", request.ShortDebugString());
            IF_DONE_SET_RESPONSE(done, EA::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        rocksdb_keys.push_back(construct_servlet_key(tmp_servlet_id));
        rocksdb_values.push_back(servlet_value);

        // persist zone_id
        std::string max_zone_id_value;
        max_zone_id_value.append((char *) &tmp_servlet_id, sizeof(int64_t));
        rocksdb_keys.push_back(construct_max_servlet_id_key());
        rocksdb_values.push_back(max_zone_id_value);

        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(rocksdb_keys, rocksdb_values);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update memory info
        set_servlet_info(servlet_info);
        set_max_servlet_id(tmp_servlet_id);
        ZoneManager::get_instance()->add_servlet_id(namespace_id, tmp_servlet_id);
        IF_DONE_SET_RESPONSE(done, EA::SUCCESS, "success");
        TLOG_INFO("create zone success, request:{}", request.ShortDebugString());
    }

    void ServletManager::drop_servlet(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done) {
        // check
        auto &servlet_info = request.servlet_info();
        std::string namespace_name = servlet_info.namespace_name();
        std::string zone_name = namespace_name + "\001" + servlet_info.zone();
        std::string servlet_name = zone_name + "\001" + servlet_info.servlet_name();
        int64_t namespace_id = NamespaceManager::get_instance()->get_namespace_id(namespace_name);
        if (namespace_id == 0) {
            TLOG_WARN("request namespace: {} not exist", namespace_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(zone_name);
        if (zone_id == 0) {
            TLOG_WARN("request zone:{} not exist", zone_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        if (_servlet_id_map.find(zone_name) == _servlet_id_map.end()) {
            TLOG_WARN("request servlet: {} not exist", zone_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }

        int64_t servlet_id = _servlet_id_map[servlet_name];
        // TODO check no instance of servlet
        // persist to rocksdb
        int ret = DiscoveryRocksdb::get_instance()->remove_discovery_info(
                std::vector<std::string>{construct_servlet_key(zone_id)});
        if (ret < 0) {
            TLOG_WARN("drop zone: {} to rocksdb fail", zone_name);
            IF_DONE_SET_RESPONSE(done, EA::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update zone memory info
        erase_servlet_info(servlet_name);
        // update namespace memory info
        ZoneManager::get_instance()->delete_servlet_id(zone_id, servlet_id);
        IF_DONE_SET_RESPONSE(done, EA::SUCCESS, "success");
        TLOG_INFO("drop zone success, request:{}", request.ShortDebugString());
    }

    void ServletManager::modify_servlet(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done) {
        auto &servlet_info = request.servlet_info();
        std::string namespace_name = servlet_info.namespace_name();
        std::string zone_name = namespace_name + "\001" + servlet_info.zone();
        std::string servlet_name = zone_name + "\001" + servlet_info.servlet_name();
        int64_t namespace_id = NamespaceManager::get_instance()->get_namespace_id(namespace_name);
        if (namespace_id == 0) {
            TLOG_WARN("request namespace:{} not exist", namespace_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }
        int64_t zone_id = ZoneManager::get_instance()->get_zone_id(zone_name);
        if (zone_id == 0) {
            TLOG_WARN("request zone:{} not exist", zone_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "namespace not exist");
            return;
        }

        if (_servlet_id_map.find(servlet_name) == _servlet_id_map.end()) {
            TLOG_WARN("request zone:{} not exist", zone_name);
            IF_DONE_SET_RESPONSE(done, EA::INPUT_PARAM_ERROR, "zone not exist");
            return;
        }
        int64_t servlet_id = _servlet_id_map[servlet_name];

        EA::discovery::ServletInfo tmp_servlet_info = _servlet_info_map[zone_id];
        tmp_servlet_info.set_version(tmp_servlet_info.version() + 1);

        if (servlet_info.has_resource_tag()) {
            tmp_servlet_info.set_resource_tag(servlet_info.resource_tag());
        }

        std::string servlet_value;
        if (!tmp_servlet_info.SerializeToString(&servlet_value)) {
            TLOG_WARN("request serializeToArray fail, request:{}", request.ShortDebugString());
            IF_DONE_SET_RESPONSE(done, EA::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }
        int ret = DiscoveryRocksdb::get_instance()->put_discovery_info(construct_servlet_key(servlet_id), servlet_value);
        if (ret < 0) {
            IF_DONE_SET_RESPONSE(done, EA::INTERNAL_ERROR, "write db fail");
            return;
        }
        // update zone values in memory
        set_servlet_info(tmp_servlet_info);
        IF_DONE_SET_RESPONSE(done, EA::SUCCESS, "success");
        TLOG_INFO("modify zone success, request:{}", request.ShortDebugString());
    }

    int ServletManager::load_servlet_snapshot(const std::string &value) {
        EA::discovery::ServletInfo servlet_pb;
        if (!servlet_pb.ParseFromString(value)) {
            TLOG_ERROR("parse from pb fail when load zone snapshot, key:{}", value);
            return -1;
        }
        TLOG_WARN("servlet snapshot:{}", servlet_pb.ShortDebugString());
        set_servlet_info(servlet_pb);
        // update memory namespace values.
        ZoneManager::get_instance()->add_servlet_id(
                servlet_pb.zone_id(), servlet_pb.servlet_id());
        return 0;
    }
}  //  namespace EA::discovery
