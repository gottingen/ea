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
#include "ea/discovery/discovery_constants.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include "bthread/mutex.h"
#include "braft/raft.h"

namespace EA::discovery {
    class ZoneManager {
    public:
        friend class QueryZoneManager;

        ~ZoneManager() {
            bthread_mutex_destroy(&_zone_mutex);
        }

        static ZoneManager *get_instance() {
            static ZoneManager instance;
            return &instance;
        }

    public:

        ///
        /// \brief create zone call by schema manager,
        ///        fail on db exists and namespace not spec
        ///        dbname = namespace_name + "\001" + zone_info.zone();
        /// \param request
        /// \param done
        void create_zone(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief remove zone call by schema manager,
        ///        fail on db not exists and namespace not spec
        ///        dbname = namespace_name + "\001" + zone_info.zone();
        ///
        /// \param request
        /// \param done
        void drop_zone(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief modify zone call by schema manager,
        ///        fail on db not exists and namespace not spec
        ///        dbname = namespace_name + "\001" + zone_info.zone();
        ///
        /// \param request
        /// \param done
        void modify_zone(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief load zone info by a pb serialized string,
        ///        call by state machine snapshot load, so do not check legal,
        ///        set it to memory directly
        ///
        /// \param request
        /// \param done
        int load_zone_snapshot(const std::string &value);

        ///
        /// \brief clear data in memory
        void clear();

        ///
        /// \brief set max zone id
        /// \param max_zone_id
        void set_max_zone_id(int64_t max_zone_id);

        ///
        /// \brief get max zone id
        /// \param max_zone_id
        int64_t get_max_zone_id();

        ///
        /// \brief add the servlet to zone
        ///        fail on zone not exists. this condition
        ///        has been check in TableManager. do not check again.
        /// \param zone_id
        /// \param servlet_id
        void add_servlet_id(int64_t zone_id, int64_t servlet_id);

        ///
        /// \brief remove the servlet from zone
        ///        call any
        /// \param zone_id
        /// \param servlet_id
        void delete_servlet_id(int64_t zone_id, int64_t servlet_id);

        ///
        /// \brief get dabase id by db name
        /// \param zone_name must be format as namespace + "." + zone_name
        /// \return
        int64_t get_zone_id(const std::string &zone_name);

        ///
        /// \brief get db info by zone id
        /// \param zone_id
        /// \param zone_info
        /// \return -1 db not exists
        int get_zone_info(const int64_t &zone_id, EA::discovery::ZoneInfo &zone_info);

        ///
        /// \brief get servlets in zone.
        /// \param zone_id
        /// \param servlet_ids
        /// \return
        int get_servlet_ids(const int64_t &zone_id, std::set<int64_t> &servlet_ids);

        ///
        /// \param namespace_name
        /// \param zone_name
        /// \return
        static std::string make_zone_key(const std::string &namespace_name, const std::string &zone_name);

    private:
        ZoneManager();

        void erase_zone_info(const std::string &zone_name);

        void set_zone_info(const EA::discovery::ZoneInfo &zone_info);

        std::string construct_zone_key(int64_t zone_id);

        std::string construct_max_zone_id_key();

    private:
        //! std::mutex                                          _zone_mutex;
        bthread_mutex_t _zone_mutex;
        int64_t _max_zone_id{0};
        //! zone name --> zone id，name: namespace\001zone
        std::unordered_map<std::string, int64_t> _zone_id_map;
        std::unordered_map<int64_t, EA::discovery::ZoneInfo> _zone_info_map;
        std::unordered_map<int64_t, std::set<int64_t>> _servlet_ids;
    };

    ///
    /// inlines
    ///

    inline void ZoneManager::set_max_zone_id(int64_t max_zone_id) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        _max_zone_id = max_zone_id;
    }

    inline int64_t ZoneManager::get_max_zone_id() {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        return _max_zone_id;
    }

    inline void ZoneManager::set_zone_info(const EA::discovery::ZoneInfo &zone_info) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        std::string zone_name = make_zone_key(zone_info.namespace_name(),zone_info.zone());
        _zone_id_map[zone_name] = zone_info.zone_id();
        _zone_info_map[zone_info.zone_id()] = zone_info;
    }

    inline void ZoneManager::erase_zone_info(const std::string &zone_name) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        int64_t zone_id = _zone_id_map[zone_name];
        _zone_id_map.erase(zone_name);
        _zone_info_map.erase(zone_id);
        _servlet_ids.erase(zone_id);
    }

    inline void ZoneManager::add_servlet_id(int64_t zone_id, int64_t servlet_id) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        _servlet_ids[zone_id].insert(servlet_id);
    }

    inline void ZoneManager::delete_servlet_id(int64_t zone_id, int64_t servlet_id) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        if (_servlet_ids.find(zone_id) != _servlet_ids.end()) {
            _servlet_ids[zone_id].erase(servlet_id);
        }
    }

    inline int64_t ZoneManager::get_zone_id(const std::string &zone_name) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        if (_zone_id_map.find(zone_name) != _zone_id_map.end()) {
            return _zone_id_map[zone_name];
        }
        return 0;
    }

    inline int ZoneManager::get_zone_info(const int64_t &zone_id, EA::discovery::ZoneInfo &zone_info) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        if (_zone_info_map.find(zone_id) == _zone_info_map.end()) {
            return -1;
        }
        zone_info = _zone_info_map[zone_id];
        return 0;
    }

    inline int ZoneManager::get_servlet_ids(const int64_t &zone_id, std::set<int64_t> &servlet_ids) {
        BAIDU_SCOPED_LOCK(_zone_mutex);
        if (_servlet_ids.find(zone_id) == _servlet_ids.end()) {
            return -1;
        }
        servlet_ids = _servlet_ids[zone_id];
        return 0;
    }

    inline void ZoneManager::clear() {
        _zone_id_map.clear();
        _zone_info_map.clear();
        _servlet_ids.clear();
    }

    inline ZoneManager::ZoneManager() : _max_zone_id(0) {
        bthread_mutex_init(&_zone_mutex, nullptr);
    }

    inline std::string ZoneManager::construct_zone_key(int64_t zone_id) {
        std::string zone_key = DiscoveryConstants::DISCOVERY_TREE_IDENTIFY
                               + DiscoveryConstants::DISCOVERY_TREE_ZONE_IDENTIFY;
        zone_key.append((char *) &zone_id, sizeof(int64_t));
        return zone_key;
    }

    inline std::string ZoneManager::construct_max_zone_id_key() {
        std::string max_zone_id_key = DiscoveryConstants::DISCOVERY_TREE_IDENTIFY
                                      + DiscoveryConstants::DISCOVERY_TREE_MAX_ID_IDENTIFY
                                      + DiscoveryConstants::MAX_ZONE_ID_KEY;
        return max_zone_id_key;
    }

    inline std::string ZoneManager::make_zone_key(const std::string &namespace_name, const std::string &zone_name) {
        return namespace_name + "\001" + zone_name;
    }
}  // namespace EA::discovery

