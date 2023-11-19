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


#ifndef EA_OPS_PLUGIN_PLUGIN_MANAGER_H_
#define EA_OPS_PLUGIN_PLUGIN_MANAGER_H_

#include "turbo/container/flat_hash_map.h"
#include "eaproto/ops/ops.interface.pb.h"
#include "turbo/container/flat_hash_map.h"
#include "turbo/module/module_version.h"
#include "turbo/files/filesystem.h"
#include "turbo/base/status.h"
#include <braft/raft.h>
#include <bthread/mutex.h>
#include "ea/gflags/plugin.h"

namespace EA::plugin {

    class PluginManager {
    public:
        static PluginManager *get_instance() {
            static PluginManager ins;
            return &ins;
        }

        ~PluginManager();

        void create_plugin(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void upload_plugin(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void remove_plugin(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void restore_plugin(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void remove_tombstone_plugin(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        int load_snapshot();

        int load_snapshot_file(const std::string& file_path);

        int save_snapshot(const std::string &base_dir, const std::string &prefix, std::vector<std::string> &files);

        static std::string make_plugin_key(const std::string &name, const turbo::ModuleVersion &version);

        static std::string make_plugin_filename(const std::string &name, const turbo::ModuleVersion &version, EA::proto::Platform platform);
        static std::string make_plugin_store_path(const std::string &name, const turbo::ModuleVersion &version, EA::proto::Platform platform);

    private:
        PluginManager();

        friend class QueryPluginManager;

        static bool load_plugin_snapshot(const std::string &key, const std::string &value);

        void remove_plugin_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);
        void remove_tombstone_plugin_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);
        void restore_plugin_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        static turbo::Status transfer_info_to_entity(const EA::proto::PluginInfo *info, EA::proto::PluginEntity*entity);
        static void transfer_entity_to_info(const EA::proto::PluginEntity *info, EA::proto::PluginInfo*entity);
    private:
        bthread_mutex_t _plugin_mutex;
        bthread_mutex_t _tombstone_plugin_mutex;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::proto::PluginEntity>> _plugins;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::proto::PluginEntity>> _tombstone_plugins;
    };

    ///
    /// inlines
    ///

    inline PluginManager::PluginManager() {
        bthread_mutex_init(&_plugin_mutex, nullptr);
        bthread_mutex_init(&_tombstone_plugin_mutex, nullptr);
        std::error_code ec;
        if(!turbo::filesystem::exists(FLAGS_plugin_data_root, ec)) {
            turbo::filesystem::create_directories(FLAGS_plugin_data_root, ec);
        }
    }

    inline PluginManager::~PluginManager() {
        bthread_mutex_destroy(&_plugin_mutex);
        bthread_mutex_destroy(&_tombstone_plugin_mutex);
    }
}  // namespace EA::plugin

#endif  // EA_OPS_PLUGIN_PLUGIN_MANAGER_H_
