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
    class ServletManager {
    public:
        friend class QueryServletManager;

        ~ServletManager() {
            bthread_mutex_destroy(&_servlet_mutex);
        }

        static ServletManager *get_instance() {
            static ServletManager instance;
            return &instance;
        }
    public:

        ///
        /// \brief create servlet call by schema manager,
        ///        fail on db exists and namespace not spec
        ///        servlet name  = namespace_name + "\001" + servlet_info.zone()+ "\001" + servlet_info.servlet_name();
        /// \param request
        /// \param done
        void create_servlet(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief remove servlet call by schema manager,
        ///        fail on db not exists and namespace not spec
        ///        servlet name  = namespace_name + "\001" + servlet_info.zone()+ "\001" + servlet_info.servlet_name();
        ///
        /// \param request
        /// \param done
        void drop_servlet(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief modify servlet call by schema manager,
        ///        fail on db not exists and namespace not spec
        ///        servlet name  = namespace_name + "\001" + servlet_info.zone()+ "\001" + servlet_info.servlet_name();
        ///
        /// \param request
        /// \param done
        void modify_servlet(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief load servlet info by a pb serialized string,
        ///        call by state machine snapshot load, so do not check legal,
        ///        set it to memory directly
        ///
        /// \param request
        /// \param done
        int load_servlet_snapshot(const std::string &value);

        ///
        /// \brief clear data in memory
        void clear();

        ///
        /// \brief set max servlet id
        /// \param max_servlet_id
        void set_max_servlet_id(int64_t max_servlet_id);

        ///
        /// \brief get max servlet id
        /// \param max_servlet_id
        int64_t get_max_servlet_id();

        ///
        /// \brief get servlet id by db name
        /// \param servlet_name must be format as namespace + "." + zone_name + "." + servlet_name
        /// \return
        int64_t get_servlet_id(const std::string &servlet_name);

        ///
        /// \brief get db info by servlet id
        /// \param servlet_id
        /// \param servlet_info
        /// \return -1 db not exists
        int get_servlet_info(const int64_t &servlet_id, EA::discovery::ServletInfo &servlet_info);

        ///
        /// \param namespace_name
        /// \param zone_name
        /// \param servlet_name
        /// \return
        static std::string make_servlet_key(const std::string &namespace_name, const std::string & zone_name, const std::string &servlet_name);
        ///
        /// \param zone_key
        /// \param servlet_name
        /// \return
        static std::string make_servlet_key(const std::string &zone_key,const std::string &servlet_name);

    private:
        ServletManager();

        ///
        /// \param servlet_name
        void erase_servlet_info(const std::string &servlet_name);

        ///
        /// \param servlet_info
        void set_servlet_info(const EA::discovery::ServletInfo &servlet_info);

        ///
        /// \param servlet_id
        /// \return
        std::string construct_servlet_key(int64_t servlet_id);

        ///
        /// \return
        std::string construct_max_servlet_id_key();

    private:
        //! std::mutex                                          _servlet_mutex;
        bthread_mutex_t _servlet_mutex;
        int64_t _max_servlet_id{0};
        //! servlet name --> servlet id，name: namespace\001zone\001servlet
        std::unordered_map<std::string, int64_t> _servlet_id_map;
        std::unordered_map<int64_t, EA::discovery::ServletInfo> _servlet_info_map;
    };

    ///
    /// inlines
    ///

    inline void ServletManager::set_max_servlet_id(int64_t max_servlet_id) {
        BAIDU_SCOPED_LOCK(_servlet_mutex);
        _max_servlet_id = max_servlet_id;
    }

    inline int64_t ServletManager::get_max_servlet_id() {
        BAIDU_SCOPED_LOCK(_servlet_mutex);
        return _max_servlet_id;
    }

    inline void ServletManager::set_servlet_info(const EA::discovery::ServletInfo &servlet_info) {
        BAIDU_SCOPED_LOCK(_servlet_mutex);
        std::string servlet_name = make_servlet_key(servlet_info.namespace_name(), servlet_info.zone(), servlet_info.servlet_name());
        _servlet_id_map[servlet_name] = servlet_info.servlet_id();
        _servlet_info_map[servlet_info.servlet_id()] = servlet_info;
    }

    inline void ServletManager::erase_servlet_info(const std::string &servlet_name) {
        BAIDU_SCOPED_LOCK(_servlet_mutex);
        int64_t servlet_id = _servlet_id_map[servlet_name];
        _servlet_id_map.erase(servlet_name);
        _servlet_info_map.erase(servlet_id);
    }

    inline int64_t ServletManager::get_servlet_id(const std::string &servlet_name) {
        BAIDU_SCOPED_LOCK(_servlet_mutex);
        if (_servlet_id_map.find(servlet_name) != _servlet_id_map.end()) {
            return _servlet_id_map[servlet_name];
        }
        return 0;
    }
    inline int ServletManager::get_servlet_info(const int64_t &servlet_id, EA::discovery::ServletInfo &servlet_info) {
        BAIDU_SCOPED_LOCK(_servlet_mutex);
        if (_servlet_info_map.find(servlet_id) == _servlet_info_map.end()) {
            return -1;
        }
        servlet_info = _servlet_info_map[servlet_id];
        return 0;
    }

    inline void ServletManager::clear() {
        _servlet_id_map.clear();
        _servlet_info_map.clear();
    }

    inline ServletManager::ServletManager() : _max_servlet_id(0) {
        bthread_mutex_init(&_servlet_mutex, nullptr);
    }

    inline std::string ServletManager::construct_servlet_key(int64_t servlet_id) {
        std::string servlet_key = DiscoveryConstants::SCHEMA_IDENTIFY
                                   + DiscoveryConstants::SERVLET_SCHEMA_IDENTIFY;
        servlet_key.append((char *) &servlet_id, sizeof(int64_t));
        return servlet_key;
    }

    inline std::string ServletManager::construct_max_servlet_id_key() {
        std::string max_servlet_id_key = DiscoveryConstants::SCHEMA_IDENTIFY
                                          + DiscoveryConstants::MAX_ID_SCHEMA_IDENTIFY
                                          + DiscoveryConstants::MAX_SERVLET_ID_KEY;
        return max_servlet_id_key;
    }

    inline std::string ServletManager::make_servlet_key(const std::string &namespace_name, const std::string & zone_name, const std::string &servlet_name) {
        return namespace_name + "\001" + zone_name + "\001" + servlet_name;
    }

    inline std::string ServletManager::make_servlet_key(const std::string &zone_key,const std::string &servlet_name) {
        return zone_key + "\001" + servlet_name;
    }
}  // namespace EA::discovery

