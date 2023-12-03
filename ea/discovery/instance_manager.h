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

//
// Created by jeff on 23-11-29.
//

#ifndef EA_DISCOVERY_INSTANCE_MANAGER_H_
#define EA_DISCOVERY_INSTANCE_MANAGER_H_

#include "turbo/container/flat_hash_map.h"
#include "turbo/container/flat_hash_set.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include "braft/raft.h"
#include "bthread/mutex.h"
#include "ea/discovery/discovery_constants.h"
#include "turbo/times/stop_watcher.h"
#include "ea/base/time_cast.h"
#include "ea/discovery/zone_manager.h"
#include "ea/discovery/servlet_manager.h"

namespace EA::discovery {

    class InstanceManager {
    public:
        static InstanceManager *get_instance() {
            static InstanceManager ins;
            return &ins;
        }

        ~InstanceManager();

        ///
        /// \param request
        /// \param done
        void add_instance(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \param request
        /// \param done
        void drop_instance(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \param request
        /// \param done
        void update_instance(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \param value
        /// \return
        int load_instance_snapshot(const std::string &value);

        void clear();

        ///
        /// \return
        int load_snapshot();

    private:
        InstanceManager();

        std::string construct_instance_key(const std::string &address);

        void set_instance_info(const EA::discovery::ServletInstance &instance_info);

        void remove_instance_info(const std::string &address);

    private:
        friend class QueryInstanceManager;
        bthread_mutex_t _instance_mutex;
        turbo::flat_hash_map<std::string, EA::discovery::ServletInstance>       _instance_info;
        turbo::flat_hash_map<std::string, TimeCost>                           _removed_instance;

        /// namespace --> instance
        turbo::flat_hash_map<std::string, turbo::flat_hash_set<std::string>>  _namespace_instance;
        /// key zone[namespace + 0x01+zone] value:instance address
        turbo::flat_hash_map<std::string, turbo::flat_hash_set<std::string>>  _zone_instance;
        /// key zone[namespace + 0x01+zone + 0x01 + servlet] value:instance address
        turbo::flat_hash_map<std::string, turbo::flat_hash_set<std::string>>  _servlet_instance;
    };

    /// inlines

    inline InstanceManager::InstanceManager() {
        bthread_mutex_init(&_instance_mutex, nullptr);
    }

    inline InstanceManager::~InstanceManager() {
        bthread_mutex_destroy(&_instance_mutex);
    }

    inline std::string InstanceManager::construct_instance_key(const std::string &address) {
        std::string instance_key = DiscoveryConstants::DISCOVERY_IDENTIFY
                                   + DiscoveryConstants::DISCOVERY_INSTANCE_IDENTIFY;
        instance_key.append(address);
        return instance_key;
    }

    inline void InstanceManager::clear() {
        _instance_info.clear();
        _removed_instance.clear();
        _namespace_instance.clear();
        _zone_instance.clear();
        _servlet_instance.clear();
    }

}  // namespace EA::discovery

#endif  // EA_DISCOVERY_INSTANCE_MANAGER_H_
