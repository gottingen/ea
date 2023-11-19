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

#ifndef EA_OPS_QUERY_PLUGIN_MANAGER_H_
#define EA_OPS_QUERY_PLUGIN_MANAGER_H_

#include "eaproto/ops/ops.interface.pb.h"
#include "turbo/container/flat_hash_map.h"
#include "ea/base/lru_cache.h"
#include "turbo/files/filesystem.h"
#include <bthread/mutex.h>
#include <memory>

namespace EA::plugin {

    struct CachePlugin {
        int fd{-1};

        ~CachePlugin();

        std::string file_path;
    };
    typedef std::shared_ptr<CachePlugin> CachePluginPtr;

    class QueryPluginManager {
    public:
        static QueryPluginManager *get_instance() {
            static QueryPluginManager ins;
            return &ins;
        }

        ~QueryPluginManager();

        std::string kReadLinkDir;

        void init();

        void
        download_plugin(const ::EA::proto::QueryOpsServiceRequest *request,
                        ::EA::proto::QueryOpsServiceResponse *response);

        void
        list_plugin(const ::EA::proto::QueryOpsServiceRequest *request, ::EA::proto::QueryOpsServiceResponse *response);

        void
        tombstone_list_plugin(const ::EA::proto::QueryOpsServiceRequest *request,
                              ::EA::proto::QueryOpsServiceResponse *response);

        void
        list_plugin_version(const ::EA::proto::QueryOpsServiceRequest *request,
                            ::EA::proto::QueryOpsServiceResponse *response);

        void
        tombstone_list_plugin_version(const ::EA::proto::QueryOpsServiceRequest *request,
                                      ::EA::proto::QueryOpsServiceResponse *response);


        void
        plugin_info(const ::EA::proto::QueryOpsServiceRequest *request,
                    ::EA::proto::QueryOpsServiceResponse *response);

        void
        tombstone_plugin_info(const ::EA::proto::QueryOpsServiceRequest *request,
                              ::EA::proto::QueryOpsServiceResponse *response);

    private:
        QueryPluginManager();

    private:
        friend class CachePlugin;
        Cache<std::string, CachePluginPtr> _cache;
        bthread_mutex_t _plugin_mutex;
    };

    inline QueryPluginManager::QueryPluginManager() {
        bthread_mutex_init(&_plugin_mutex, nullptr);
    }

    inline QueryPluginManager::~QueryPluginManager() {
        bthread_mutex_destroy(&_plugin_mutex);
    }
}  // namespace EA::plugin

#endif  // EA_OPS_QUERY_PLUGIN_MANAGER_H_
