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

#ifndef EA_OPS_DICT_QUERY_DICT_MANAGER_H_
#define EA_OPS_DICT_QUERY_DICT_MANAGER_H_

#include "eaproto/ops/ops.interface.pb.h"
#include "turbo/container/flat_hash_map.h"
#include "ea/base/lru_cache.h"
#include "ea/gflags/dict.h"
#include "turbo/files/filesystem.h"
#include <bthread/mutex.h>
#include <memory>

namespace EA::dict {

    struct CacheFile {
        int fd{-1};

        ~CacheFile();

        std::string file_path;
    };
    typedef std::shared_ptr<CacheFile> CacheFilePtr;

    class QueryDictManager {
    public:
        static QueryDictManager *get_instance() {
            static QueryDictManager ins;
            return &ins;
        }

        ~QueryDictManager();

        std::string kReadLinkDir;

        void init();

        void
        download_dict(const ::EA::proto::QueryOpsServiceRequest *request,
                        ::EA::proto::QueryOpsServiceResponse *response);

        void
        list_dict(const ::EA::proto::QueryOpsServiceRequest *request, ::EA::proto::QueryOpsServiceResponse *response);

        void
        tombstone_list_dict(const ::EA::proto::QueryOpsServiceRequest *request,
                              ::EA::proto::QueryOpsServiceResponse *response);

        void
        list_dict_version(const ::EA::proto::QueryOpsServiceRequest *request,
                            ::EA::proto::QueryOpsServiceResponse *response);

        void
        tombstone_list_dict_version(const ::EA::proto::QueryOpsServiceRequest *request,
                                      ::EA::proto::QueryOpsServiceResponse *response);


        void
        dict_info(const ::EA::proto::QueryOpsServiceRequest *request,
                    ::EA::proto::QueryOpsServiceResponse *response);

        void
        tombstone_dict_info(const ::EA::proto::QueryOpsServiceRequest *request,
                              ::EA::proto::QueryOpsServiceResponse *response);

    private:
        QueryDictManager();

    private:
        friend class CacheFile;
        Cache<std::string, CacheFilePtr> _cache;
        bthread_mutex_t _dict_cache_mutex;
    };

    inline QueryDictManager::QueryDictManager() {
        bthread_mutex_init(&_dict_cache_mutex, nullptr);
    }

    inline QueryDictManager::~QueryDictManager() {
        bthread_mutex_destroy(&_dict_cache_mutex);
    }
}  // namespace EA::dict

#endif  // EA_OPS_DICT_QUERY_DICT_MANAGER_H_
