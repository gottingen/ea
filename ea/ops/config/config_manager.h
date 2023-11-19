// Copyright 2023 The Elastic-AI Authors.
// part of Elastic AI Search
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


#ifndef EA_OPS_CONFIG_CONFIG_MANAGER_H_
#define EA_OPS_CONFIG_CONFIG_MANAGER_H_

#include "turbo/container/flat_hash_map.h"
#include "eaproto/ops/ops.interface.pb.h"
#include "turbo/container/flat_hash_map.h"
#include "turbo/module/module_version.h"
#include <braft/raft.h>
#include <bthread/mutex.h>

namespace EA::config {

    class ConfigManager {
    public:
        static ConfigManager *get_instance() {
            static ConfigManager ins;
            return &ins;
        }

        ~ConfigManager();

        void create_config(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void remove_config(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        int load_snapshot();

        static std::string make_config_key(const std::string &name, const turbo::ModuleVersion &version);

    private:
        ConfigManager();

        friend class QueryConfigManager;

        static bool load_config_snapshot(const std::string &key, const std::string &value);

        void remove_config_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

    private:
        bthread_mutex_t _config_mutex;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::proto::ConfigEntity>> _configs;
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
}  // namespace EA::config
#endif  // EA_OPS_CONFIG_CONFIG_MANAGER_H_
