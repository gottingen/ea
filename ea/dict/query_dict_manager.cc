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
#include "ea/dict/query_dict_manager.h"
#include "ea/dict/dict_manager.h"
#include "ea/base/file_util.h"
#include "ea/base/tlog.h"

namespace EA::dict {

    CacheFile::~CacheFile() {
        {
            BAIDU_SCOPED_LOCK(QueryDictManager::get_instance()->_dict_cache_mutex);
            if (fd > 0) {
                ::close(fd);
                fd = -1;
            }
            std::error_code ec;
            turbo::filesystem::remove(file_path, ec);
        }
    }

    void QueryDictManager::init() {
        kReadLinkDir = FLAGS_dict_data_root + "/read_link";
        std::error_code ec;
        if (turbo::filesystem::exists(kReadLinkDir, ec)) {
            turbo::filesystem::remove_all(kReadLinkDir, ec);
        }
        turbo::filesystem::create_directories(kReadLinkDir, ec);
    }

    void
    QueryDictManager::download_dict(const ::EA::proto::QueryOpsServiceRequest *request,
                                    ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        auto &download_request = request->query_dict();
        if (!download_request.has_version()) {
            response->set_errmsg("file not set version");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        turbo::ModuleVersion version = turbo::ModuleVersion(download_request.version().major(),
                                                            download_request.version().minor(),
                                                            download_request.version().patch());
        EA::proto::DictEntity entity;
        auto &name = download_request.name();
        {
            BAIDU_SCOPED_LOCK(DictManager::get_instance()->_dict_mutex);
            auto &dicts = DictManager::get_instance()->_dicts;
            auto it = dicts.find(name);
            if (it == dicts.end() || it->second.empty()) {
                response->set_errmsg("dict not exist");
                response->set_errcode(proto::INPUT_PARAM_ERROR);
                return;
            }
            auto pit = it->second.find(version);
            if (pit == it->second.end()) {
                response->set_errmsg("dict not exist");
                response->set_errcode(proto::INPUT_PARAM_ERROR);
                return;
            }
            entity = pit->second;
            if (!entity.finish()) {
                response->set_errmsg("dict not upload finish");
                response->set_errcode(proto::INPUT_PARAM_ERROR);
                return;
            }
        }

        if (!download_request.has_offset()) {
            response->set_errmsg("dict not set offset");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }

        if (!download_request.has_count()) {
            response->set_errmsg("dict not set count");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        std::string key = DictManager::make_dict_key(name, version);

        CacheFilePtr cache_file;
        auto libname = DictManager::make_dict_filename(name, version, entity.ext());
        std::string source_path = turbo::Format("{}/{}", FLAGS_dict_data_root, libname);
        std::string link_path = turbo::Format("{}/read_link/{}", FLAGS_dict_data_root, libname);

        if (_cache.find(key, &cache_file) != 0) {
            {
                BAIDU_SCOPED_LOCK(_dict_cache_mutex);
                std::error_code ec;
                if (!turbo::filesystem::exists(link_path, ec)) {
                    turbo::filesystem::create_hard_link(source_path, link_path, ec);
                    if (ec) {
                        response->set_errmsg("create dict read link file error");
                        response->set_errcode(proto::INTERNAL_ERROR);
                        return;
                    }
                }
            }
            int fd = ::open(link_path.c_str(), O_RDONLY, 0644);
            if (fd < 0) {
                response->set_errmsg("read dict file error");
                response->set_errcode(proto::INTERNAL_ERROR);
                return;
            }
            cache_file = std::make_shared<CacheFile>();
            cache_file->fd = fd;
            cache_file->file_path = link_path;
            _cache.add(key, cache_file);
        }

        auto len = download_request.count();
        auto offset = download_request.offset();
        char *buf = new char[len];
        if (offset + len > entity.size()) {
            len = entity.size() - offset;
        }
        ssize_t n = ea_pread(cache_file->fd, buf, len, offset);

        if (n < 0 || n != (ssize_t) len) {
            TLOG_ERROR("Fail to pread file:{} for req:{}", source_path, request->DebugString());
            response->set_errcode(proto::INTERNAL_ERROR);
            response->set_errmsg("dict:" + name + " read failed");
            delete[] buf;
            return;
        }
        response->mutable_dict_response()->set_content(buf, len);
        delete[] buf;
        DictManager::transfer_entity_to_info(&entity, response->mutable_dict_response()->mutable_dict());
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryDictManager::dict_info(const ::EA::proto::QueryOpsServiceRequest *request,
                                     ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        BAIDU_SCOPED_LOCK(DictManager::get_instance()->_dict_mutex);
        auto &dicts = DictManager::get_instance()->_dicts;
        auto &get_request = request->query_dict();
        auto &name = get_request.name();
        auto it = dicts.find(name);
        if (it == dicts.end() || it->second.empty()) {
            response->set_errmsg("dict not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        turbo::ModuleVersion version;

        if (!get_request.has_version()) {
            // use newest
            // version = it->second.rend()->first;
            auto cit = it->second.rbegin();
            DictManager::transfer_entity_to_info(&cit->second, response->mutable_dict_response()->mutable_dict());
            response->set_errmsg("success");
            response->set_errcode(proto::SUCCESS);
            return;
        }

        version = turbo::ModuleVersion(get_request.version().major(), get_request.version().minor(),
                                       get_request.version().patch());

        auto cit = it->second.find(version);
        if (cit == it->second.end()) {
            /// not exists
            response->set_errmsg("dict not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }

        DictManager::transfer_entity_to_info(&cit->second, response->mutable_dict_response()->mutable_dict());
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryDictManager::tombstone_dict_info(const ::EA::proto::QueryOpsServiceRequest *request,
                                               ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        BAIDU_SCOPED_LOCK(DictManager::get_instance()->_tombstone_dict_mutex);
        auto &tombstone_dicts = DictManager::get_instance()->_tombstone_dicts;
        auto &get_request = request->query_dict();
        auto &name = get_request.name();
        auto it = tombstone_dicts.find(name);
        if (it == tombstone_dicts.end() || it->second.empty()) {
            response->set_errmsg("dict not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        turbo::ModuleVersion version;

        if (!get_request.has_version()) {
            // use newest
            // version = it->second.rend()->first;
            auto cit = it->second.rbegin();
            DictManager::transfer_entity_to_info(&cit->second, response->mutable_dict_response()->mutable_dict());
            response->set_errmsg("success");
            response->set_errcode(proto::SUCCESS);
            return;
        }

        version = turbo::ModuleVersion(get_request.version().major(), get_request.version().minor(),
                                       get_request.version().patch());

        auto cit = it->second.find(version);
        if (cit == it->second.end()) {
            /// not exists
            response->set_errmsg("dict not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }

        DictManager::transfer_entity_to_info(&cit->second, response->mutable_dict_response()->mutable_dict());
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }


    void QueryDictManager::list_dict(const ::EA::proto::QueryOpsServiceRequest *request,
                                     ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        BAIDU_SCOPED_LOCK(DictManager::get_instance()->_dict_mutex);
        auto dicts = DictManager::get_instance()->_dicts;
        response->mutable_dict_response()->mutable_dict_list()->Reserve(dicts.size());
        for (auto it = dicts.begin(); it != dicts.end(); ++it) {
            response->mutable_dict_response()->add_dict_list(it->first);
        }
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryDictManager::tombstone_list_dict(const ::EA::proto::QueryOpsServiceRequest *request,
                                               ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        BAIDU_SCOPED_LOCK(DictManager::get_instance()->_tombstone_dict_mutex);
        auto &tombstone_dicts = DictManager::get_instance()->_tombstone_dicts;
        response->mutable_dict_response()->mutable_dict_list()->Reserve(tombstone_dicts.size());
        for (auto it = tombstone_dicts.begin(); it != tombstone_dicts.end(); ++it) {
            response->mutable_dict_response()->add_dict_list(it->first);
        }
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryDictManager::list_dict_version(const ::EA::proto::QueryOpsServiceRequest *request,
                                             ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        auto &get_request = request->query_dict();
        BAIDU_SCOPED_LOCK(DictManager::get_instance()->_dict_mutex);
        auto &dicts = DictManager::get_instance()->_dicts;
        auto &name = get_request.name();
        auto it = dicts.find(name);
        if (it == dicts.end()) {
            response->set_errmsg("dict not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        response->mutable_dict_response()->mutable_versions()->Reserve(it->second.size());
        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            *(response->mutable_dict_response()->add_versions()) = vit->second.version();
        }
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryDictManager::tombstone_list_dict_version(const ::EA::proto::QueryOpsServiceRequest *request,
                                                       ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        auto &get_request = request->query_dict();
        BAIDU_SCOPED_LOCK(DictManager::get_instance()->_tombstone_dict_mutex);
        auto &tombstone_dicts = DictManager::get_instance()->_tombstone_dicts;
        auto &name = get_request.name();
        auto it = tombstone_dicts.find(name);
        if (it == tombstone_dicts.end()) {
            response->set_errmsg("dict not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        response->mutable_dict_response()->mutable_versions()->Reserve(it->second.size());
        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            *(response->mutable_dict_response()->add_versions()) = vit->second.version();
        }
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }
}  // namespace EA::dict
