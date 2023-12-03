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



#ifndef EA_DISCOVERY_CONFIG_MANAGER_H_
#define EA_DISCOVERY_CONFIG_MANAGER_H_

#include "turbo/container/flat_hash_map.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include "turbo/container/flat_hash_map.h"
#include "ea/discovery/discovery_state_machine.h"
#include "turbo/module/module_version.h"
#include "ea/discovery/discovery_server.h"
#include <braft/raft.h>
#include <bthread/mutex.h>

namespace EA::discovery {

    class ConfigManager {
    public:
        static turbo::ModuleVersion kDefaultVersion;
        static ConfigManager *get_instance() {
            static ConfigManager ins;
            return &ins;
        }

        ~ConfigManager();

        ///
        /// \brief preprocess for raft machine, check
        ///        parameter
        /// \param controller
        /// \param request
        /// \param response
        /// \param done
        void process_schema_info(google::protobuf::RpcController *controller,
                                                const EA::discovery::DiscoveryManagerRequest *request,
                                                EA::discovery::DiscoveryManagerResponse *response,
                                                google::protobuf::Closure *done);
        ///
        /// \param request
        /// \param done
        void create_config(const ::EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \param request
        /// \param done
        void remove_config(const ::EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \return
        int load_snapshot();

        ///
        /// \param name
        /// \param version
        /// \return
        static std::string make_config_key(const std::string &name, const turbo::ModuleVersion &version);

        ///
        /// \param machine
        void set_discovery_state_machine(DiscoveryStateMachine *machine);
    private:
        ConfigManager();

        ///
        friend class QueryConfigManager;

        ///
        /// \param value
        /// \return
        int load_config_snapshot(const std::string &value);

        ///
        /// \param request
        /// \param done
        void remove_config_all(const ::EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

    private:
        DiscoveryStateMachine *_discovery_state_machine;
        bthread_mutex_t _config_mutex;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::discovery::ConfigInfo>> _configs;

    };

    ///
    /// inlines
    ///

    inline ConfigManager::ConfigManager() {
        bthread_mutex_init(&_config_mutex, nullptr);
    }

    inline ConfigManager::~ConfigManager() {
        bthread_mutex_destroy(&_config_mutex);
    }

    inline void ConfigManager::set_discovery_state_machine(DiscoveryStateMachine *machine) {
        _discovery_state_machine = machine;
    }
}  // namespace EA::discovery
#endif  // EA_DISCOVERY_CONFIG_MANAGER_H_
