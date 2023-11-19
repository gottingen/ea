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


#ifndef EA_DICT_DICT_MANAGER_H_
#define EA_DICT_DICT_MANAGER_H_

#include "turbo/container/flat_hash_map.h"
#include "eaproto/ops/ops.interface.pb.h"
#include "turbo/container/flat_hash_map.h"
#include "turbo/module/module_version.h"
#include "turbo/files/filesystem.h"
#include "turbo/base/status.h"
#include <braft/raft.h>
#include <bthread/mutex.h>
#include "ea/gflags/dict.h"

namespace EA::dict {

    class DictManager {
    public:
        static DictManager *get_instance() {
            static DictManager ins;
            return &ins;
        }

        ~DictManager();

        void create_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void upload_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void remove_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void restore_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        void remove_tombstone_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        int load_snapshot();

        int load_snapshot_file(const std::string& file_path);

        int save_snapshot(const std::string &base_dir, const std::string &prefix, std::vector<std::string> &files);

        static std::string make_dict_key(const std::string &name, const turbo::ModuleVersion &version);

        static std::string make_dict_filename(const std::string &name, const turbo::ModuleVersion &version, const std::string &ext);
        static std::string make_dict_store_path(const std::string &name, const turbo::ModuleVersion &version, const std::string &ext);

    private:
        DictManager();

        friend class QueryDictManager;

        static int load_dict_snapshot(const std::string &key, const std::string &value);

        void remove_dict_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);
        void remove_tombstone_dict_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);
        void restore_dict_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done);

        static turbo::Status transfer_info_to_entity(const EA::proto::DictInfo *info, EA::proto::DictEntity*entity);
        static void transfer_entity_to_info(const EA::proto::DictEntity *info, EA::proto::DictInfo*entity);
    private:
        bthread_mutex_t _dict_mutex;
        bthread_mutex_t _tombstone_dict_mutex;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::proto::DictEntity>> _dicts;
        turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::proto::DictEntity>> _tombstone_dicts;
    };

    ///
    /// inlines
    ///

    inline DictManager::DictManager() {
        bthread_mutex_init(&_dict_mutex, nullptr);
        bthread_mutex_init(&_tombstone_dict_mutex, nullptr);
        std::error_code ec;
        if(!turbo::filesystem::exists(FLAGS_dict_data_root, ec)) {
            turbo::filesystem::create_directories(FLAGS_dict_data_root, ec);
        }
    }

    inline DictManager::~DictManager() {
        bthread_mutex_destroy(&_dict_mutex);
        bthread_mutex_destroy(&_tombstone_dict_mutex);
    }
}  // namespace EA::dict

#endif  // EA_DICT_DICT_MANAGER_H_
