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

#include <unordered_map>
#include <set>
#include <mutex>
#include "eapi/servlet/servlet.interface.pb.h"
#include "ea/meta_server/meta_constants.h"
#include "braft/raft.h"
#include "bthread/mutex.h"

namespace EA::servlet {
    class NamespaceManager {
    public:
        friend class QueryNamespaceManager;

        ~NamespaceManager() {
            bthread_mutex_destroy(&_namespace_mutex);
        }

        static NamespaceManager *get_instance() {
            static NamespaceManager instance;
            return &instance;
        }

        ///
        /// \brief create namespace
        ///        failed when exists
        /// \param request
        /// \param done
        void create_namespace(const EA::servlet::MetaManagerRequest &request, braft::Closure *done);

        ///
        /// \brief remove namespace
        ///        failed when exists db in the namespace
        /// \param request
        /// \param done
        void drop_namespace(const EA::servlet::MetaManagerRequest &request, braft::Closure *done);

        ///
        /// \brief modify namespace
        ///        name and quota can be modify
        /// \param request
        /// \param done
        void modify_namespace(const EA::servlet::MetaManagerRequest &request, braft::Closure *done);

        ///
        /// \brief load namespace snapshot called by meta state machine
        /// \param value
        /// \return
        int load_namespace_snapshot(const std::string &value);

        ///
        /// \param max_namespace_id
        void set_max_namespace_id(int64_t max_namespace_id);

        ///
        /// \brief get max id for namespace
        /// \return

        int64_t get_max_namespace_id();

        ///
        /// \brief add a zone under a namespace
        /// \param namespace_id
        /// \param zone_id
        void add_zone_id(int64_t namespace_id, int64_t zone_id);

        ///
        /// \brief remove a zone under a namespace
        /// \param namespace_id
        /// \param zone_id
        void delete_zone_id(int64_t namespace_id, int64_t zone_id);

        ///
        /// \brief get namespace id by namespace name, and the name
        /// \param namespace_name
        /// \return
        int64_t get_namespace_id(const std::string &namespace_name);

        ///
        /// \brief get namespace resouce tag
        /// \param namespace_name
        /// \return
        const std::string get_resource_tag(const int64_t &namespace_id);

        ///
        /// \brief get namespace info by namespace id
        /// \param namespace_id
        /// \param namespace_info
        /// \return
        int get_namespace_info(const int64_t &namespace_id, EA::servlet::NameSpaceInfo &namespace_info);

        ///
        /// \brief clear memory values.
        void clear();

    private:
        NamespaceManager();

        ///
        /// \brief set namespace info for space.
        /// \param namespace_info
        void set_namespace_info(const EA::servlet::NameSpaceInfo &namespace_info);

        ///
        /// \brief erase info for namespace
        /// \param namespace_name
        void erase_namespace_info(const std::string &namespace_name);

        ///
        /// \brief construct namespace key
        /// \param namespace_id
        /// \return
        std::string construct_namespace_key(int64_t namespace_id);

        std::string construct_max_namespace_id_key();

    private:
        //std::mutex                                          _namespace_mutex;
        bthread_mutex_t _namespace_mutex;

        int64_t _max_namespace_id{0};
        // namespace层级name与id的映射关系
        std::unordered_map<std::string, int64_t> _namespace_id_map;
        // namespace层级，id与info的映射关系
        std::unordered_map<int64_t, EA::servlet::NameSpaceInfo> _namespace_info_map;
        std::unordered_map<int64_t, std::set<int64_t>> _zone_ids; //only in memory, not in rocksdb
    };

    ///
    /// inlines
    ///

    inline void NamespaceManager::set_max_namespace_id(int64_t max_namespace_id) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        _max_namespace_id = max_namespace_id;
    }

    inline int64_t NamespaceManager::get_max_namespace_id() {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        return _max_namespace_id;
    }

    inline void NamespaceManager::set_namespace_info(const EA::servlet::NameSpaceInfo &namespace_info) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        _namespace_id_map[namespace_info.namespace_name()] = namespace_info.namespace_id();
        _namespace_info_map[namespace_info.namespace_id()] = namespace_info;
    }

    inline void NamespaceManager::erase_namespace_info(const std::string &namespace_name) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        int64_t namespace_id = _namespace_id_map[namespace_name];
        _namespace_id_map.erase(namespace_name);
        _namespace_info_map.erase(namespace_id);
        _zone_ids.erase(namespace_id);
    }


    inline void NamespaceManager::add_zone_id(int64_t namespace_id, int64_t zone_id) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        _zone_ids[namespace_id].insert(zone_id);
    }

    inline void NamespaceManager::delete_zone_id(int64_t namespace_id, int64_t zone_id) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        if (_zone_ids.find(namespace_id) != _zone_ids.end()) {
            _zone_ids[namespace_id].erase(zone_id);
        }
    }

    inline int64_t NamespaceManager::get_namespace_id(const std::string &namespace_name) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        if (_namespace_id_map.find(namespace_name) == _namespace_id_map.end()) {
            return 0;
        }
        return _namespace_id_map[namespace_name];
    }

    inline const std::string NamespaceManager::get_resource_tag(const int64_t &namespace_id) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        if (_namespace_info_map.find(namespace_id) == _namespace_info_map.end()) {
            return "";
        }
        return _namespace_info_map[namespace_id].resource_tag();
    }

    inline int NamespaceManager::get_namespace_info(const int64_t &namespace_id, EA::servlet::NameSpaceInfo &namespace_info) {
        BAIDU_SCOPED_LOCK(_namespace_mutex);
        if (_namespace_info_map.find(namespace_id) == _namespace_info_map.end()) {
            return -1;
        }
        namespace_info = _namespace_info_map[namespace_id];
        return 0;
    }

    inline void NamespaceManager::clear() {
        _namespace_id_map.clear();
        _namespace_info_map.clear();
        _zone_ids.clear();
    }
    inline NamespaceManager::NamespaceManager() : _max_namespace_id(0) {
        bthread_mutex_init(&_namespace_mutex, nullptr);
    }

    inline std::string NamespaceManager::construct_namespace_key(int64_t namespace_id) {
        std::string namespace_key = MetaConstants::SCHEMA_IDENTIFY
                                    + MetaConstants::NAMESPACE_SCHEMA_IDENTIFY;
        namespace_key.append((char *) &namespace_id, sizeof(int64_t));
        return namespace_key;
    }

    inline std::string NamespaceManager::construct_max_namespace_id_key() {
        std::string max_namespace_id_key = MetaConstants::SCHEMA_IDENTIFY
                                           + MetaConstants::MAX_ID_SCHEMA_IDENTIFY
                                           + MetaConstants::MAX_NAMESPACE_ID_KEY;
        return max_namespace_id_key;
    }

}  // namespace EA::servlet
