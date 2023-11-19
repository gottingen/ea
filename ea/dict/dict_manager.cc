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

#include "ea/dict/dict_manager.h"
#include "ea/dict/dict_state_machine.h"
#include "ea/dict/dict_meta.h"
#include "ea/base/file_util.h"
#include "turbo/files/utility.h"
#include "braft/raft.h"

namespace EA::dict {
    void DictManager::create_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        if (!request.has_request_dict()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "empty dict request");
            return;
        }
        auto &create_request = request.request_dict().dict();
        auto &name = create_request.name();
        turbo::ModuleVersion version(create_request.version().major(), create_request.version().minor(),
                                     create_request.version().patch());
        {
            BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
            auto tit = _tombstone_dicts.find(name);
            // do not rewrite.
            if ((tit != _tombstone_dicts.end()) && (tit->second.find(version) != tit->second.end())) {
                /// already exists
                TLOG_INFO("dict :{} version: {} is tombstone", name, version.to_string());
                DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "dict already removed");
                return;
            }
        }

        BAIDU_SCOPED_LOCK(_dict_mutex);
        if (_dicts.find(name) == _dicts.end()) {
            _dicts[name] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
        }
        auto it = _dicts.find(name);
        // do not rewrite.
        if (it->second.find(version) != it->second.end()) {
            /// already exists
            TLOG_INFO("dict :{} version: {} exist", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "dict already exist");
            return;
        }
        if (!it->second.empty() && it->second.rbegin()->first >= version) {
            /// Version numbers must increase monotonically
            TLOG_INFO("dict :{} version: {} must be larger than current:{}", name, version.to_string(),
                      it->second.rbegin()->first.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR,
                                                 "Version numbers must increase monotonically");
            return;
        }
        std::string rocks_key = make_dict_key(name, version);
        std::string rocks_value;
        EA::proto::DictEntity entity;
        auto st = transfer_info_to_entity(&create_request, &entity);
        if (!st.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, std::string(st.message()));
            return;
        }
        if (!entity.SerializeToString(&rocks_value)) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        auto ret = DictMeta::get_rkv()->put(rocks_key, rocks_value);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        it->second[version] = entity;
        TLOG_INFO("dict :{} version: {} create", name, version.to_string());
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void DictManager::upload_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        /// check valid
        if (!request.has_request_dict()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "miss field dict request_dict");
            return;
        }
        auto &upload_request = request.request_dict();
        if (!request.request_dict().has_offset()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "miss field dict offset");
            return;
        }

        if (!request.request_dict().has_content()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "miss field dict content");
            return;
        }
        if (request.request_dict().content().empty()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "empty dict content");
            return;
        }

        if (!upload_request.dict().has_version()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "miss field dict version");
            return;
        }

        auto &name = upload_request.dict().name();

        BAIDU_SCOPED_LOCK(_dict_mutex);
        auto it = _dicts.find(name);
        if (it == _dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        turbo::ModuleVersion version(upload_request.dict().version().major(),
                                     upload_request.dict().version().minor(),
                                     upload_request.dict().version().patch());
        auto pit = it->second.find(version);
        if (pit == it->second.end()) {
            /// not exists
            TLOG_INFO("dict :{} version: {} not exist", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "dict not exist");
        }

        std::string file_path = make_dict_store_path(name, version, pit->second.ext());
        int fd = ::open(file_path.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd < 0) {
            TLOG_WARN("upload dict :{} version: {} open file error", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "open dict error");
        }
        ssize_t nw = ea_pwrite(fd, upload_request.content().data(), upload_request.content().size(),
                                 upload_request.offset());
        if (nw < 0) {
            TLOG_WARN("upload dict :{} version: {} open file error", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "open file error");
        }

        pit->second.set_upload_size(upload_request.offset() + nw);
        ::fsync(fd);
        ftruncate(fd, pit->second.upload_size());
        if (pit->second.upload_size() == pit->second.size()) {
            pit->second.set_finish(true);
        }
        if (pit->second.finish()) {
            /// check sum
            int64_t nszie;
            auto cksm = turbo::FileUtility::md5_sum_file(file_path, &nszie);
            if (!cksm.ok()) {
                TLOG_WARN("upload dict :{} version: {} check md5 fail", name, version.to_string());
                DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "check md5 fail");
            }
            if (cksm.value() != pit->second.cksm()) {
                TLOG_WARN("upload dict :{} version: {} check md5 fail, expect: {} get: {}", name, version.to_string(),
                          pit->second.cksm(), cksm.value());
                DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "md5 not match");
            }
        }
        ::close(fd);
        /// persist
        std::string rocks_key = make_dict_key(name, version);
        std::string rocks_value;
        if (!pit->second.SerializeToString(&rocks_value)) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        auto ret = DictMeta::get_rkv()->put(rocks_key, rocks_value);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "write db fail");
            return;
        }
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void DictManager::remove_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        if (!request.has_request_dict()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "empty dict request");
            return;
        }
        auto &remove_request = request.request_dict().dict();
        auto &name = remove_request.name();
        bool remove_signal = remove_request.has_version();
        BAIDU_SCOPED_LOCK(_dict_mutex);
        if (!remove_signal) {
            remove_dict_all(request, done);
            return;
        }
        auto it = _dicts.find(name);
        if (it == _dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        turbo::ModuleVersion version(remove_request.version().major(), remove_request.version().minor(),
                                     remove_request.version().patch());
        auto pit = it->second.find(version);
        if (pit == it->second.end()) {
            /// not exists
            TLOG_INFO("dict :{} version: {} not exist", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "dict not exist");
            return;
        }

        /// mark move to tombstone and write to rocksdb
        std::string rocks_key = make_dict_key(name, version);
        std::string rocks_value;
        pit->second.set_tombstone(true);
        if (!pit->second.SerializeToString(&rocks_value)) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        auto ret = DictMeta::get_rkv()->put(rocks_key, rocks_value);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }

        /// update memory
        {
            /// move to tombstone
            BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
            if (_tombstone_dicts.find(name) == _tombstone_dicts.end()) {
                _tombstone_dicts[name] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
            }
            _tombstone_dicts[name][version] = pit->second;
        }
        /// erase from files
        it->second.erase(version);
        /// if no version under dict, remove it
        if (it->second.empty()) {
            _dicts.erase(name);
        }
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void DictManager::remove_tombstone_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        if (!request.has_request_dict()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "empty dict request");
            return;
        }
        auto &remove_request = request.request_dict().dict();
        auto &name = remove_request.name();
        bool remove_signal = remove_request.has_version();
        BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
        if (!remove_signal) {
            remove_tombstone_dict_all(request, done);
            return;
        }
        auto it = _tombstone_dicts.find(name);
        if (it == _tombstone_dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        turbo::ModuleVersion version(remove_request.version().major(), remove_request.version().minor(),
                                     remove_request.version().patch());
        auto vit = it->second.find(version);
        if (vit == it->second.end()) {
            /// not exists
            TLOG_INFO("dict :{} version: {} not exist", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "dict not exist");
            return;
        }

        /// mark move to tombstone and write to rocksdb
        std::string rocks_key = make_dict_key(name, version);

        std::string file_path = make_dict_store_path(name, vit->first, vit->second.ext());

        auto ret = DictMeta::get_rkv()->remove(rocks_key);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }

        /// erase from files
        it->second.erase(version);
        /// if no version under dict, remove it
        if (it->second.empty()) {
            _dicts.erase(name);
        }
        /// remove dict file
        std::error_code ec;
        if (turbo::filesystem::exists(file_path, ec)) {
            turbo::filesystem::remove(file_path, ec);
        }
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void DictManager::remove_dict_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        auto &remove_request = request.request_dict().dict();
        auto &name = remove_request.name();
        auto it = _dicts.find(name);
        if (it == _dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        std::vector<std::string> keys;
        std::vector<std::string> values;

        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            std::string key = make_dict_key(name, vit->first);
            std::string value;
            vit->second.set_tombstone(true);
            if (!vit->second.SerializeToString(&value)) {
                DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
                return;
            }
            keys.push_back(key);
            values.push_back(value);
        }

        auto ret = DictMeta::get_rkv()->mput(keys, values);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        /// update memory
        {
            /// move to tombstone
            BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
            if (_tombstone_dicts.find(name) == _tombstone_dicts.end()) {
                _tombstone_dicts[name] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
            }
            auto tomb_it = _tombstone_dicts.find(name);
            for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
                tomb_it->second[vit->first] = vit->second;
            }
        }
        _dicts.erase(name);
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void
    DictManager::remove_tombstone_dict_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        auto &remove_request = request.request_dict().dict();
        auto &name = remove_request.name();
        auto it = _dicts.find(name);
        if (it == _dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        std::vector<std::string> keys;
        std::vector<std::string> paths;
        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            std::string key = make_dict_key(name, vit->first);

            std::string file_path = make_dict_store_path(name, vit->first, vit->second.ext());
            keys.push_back(key);
            paths.push_back(file_path);
        }

        auto ret = DictMeta::get_rkv()->mremove(keys);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        /// update memory
        _tombstone_dicts.erase(name);
        /// remove dict files
        std::error_code ec;
        for (auto &f: paths) {
            if (turbo::filesystem::exists(f, ec)) {
                turbo::filesystem::remove(f, ec);
            }
        }
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void DictManager::restore_dict(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        if (!request.has_request_dict()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "empty dict request");
            return;
        }
        auto &restore_request = request.request_dict().dict();
        auto &name = restore_request.name();
        bool remove_signal = restore_request.has_version();
        BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
        if (!remove_signal) {
            restore_dict_all(request, done);
            return;
        }
        auto it = _tombstone_dicts.find(name);
        if (it == _tombstone_dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        turbo::ModuleVersion version(restore_request.version().major(), restore_request.version().minor(),
                                     restore_request.version().patch());
        auto pit = it->second.find(version);
        if (pit == it->second.end()) {
            /// not exists
            TLOG_INFO("dict :{} version: {} not exist", name, version.to_string());
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INPUT_PARAM_ERROR, "dict not exist");
        }

        /// mark move to tombstone and write to rocksdb
        std::string rocks_key = make_dict_key(name, version);
        std::string rocks_value;
        pit->second.set_tombstone(false);
        if (!pit->second.SerializeToString(&rocks_value)) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
            return;
        }

        auto ret = DictMeta::get_rkv()->put(rocks_key, rocks_value);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "write from db fail");
            return;
        }

        /// update memory
        {
            /// move to normal
            BAIDU_SCOPED_LOCK(_dict_mutex);
            if (_dicts.find(name) == _dicts.end()) {
                _dicts[name] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
            }
            _dicts[name][version] = pit->second;
        }
        /// erase from dicts
        it->second.erase(version);
        /// if no version under dict, remove it
        if (it->second.empty()) {
            _dicts.erase(name);
        }
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    void DictManager::restore_dict_all(const ::EA::proto::OpsServiceRequest &request, braft::Closure *done) {
        auto &restore_request = request.request_dict().dict();
        auto &name = restore_request.name();
        auto it = _tombstone_dicts.find(name);
        if (it == _tombstone_dicts.end()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "dict not exist");
            return;
        }
        std::vector<std::string> keys;
        std::vector<std::string> values;


        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            std::string key = make_dict_key(name, vit->first);
            std::string value;
            vit->second.set_tombstone(false);
            if (!vit->second.SerializeToString(&value)) {
                DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::PARSE_TO_PB_FAIL, "serializeToArray fail");
                return;
            }
            keys.push_back(key);
            values.push_back(value);
        }

        auto ret = DictMeta::get_rkv()->mput(keys, values);
        if (!ret.ok()) {
            DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::INTERNAL_ERROR, "delete from db fail");
            return;
        }
        /// update memory
        {
            /// move to tombstone
            BAIDU_SCOPED_LOCK(_dict_mutex);
            if (_dicts.find(name) == _dicts.end()) {
                _dicts[name] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
            }
            auto normal_it = _dicts.find(name);
            for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
                normal_it->second[vit->first] = vit->second;
            }
        }
        _tombstone_dicts.erase(name);
        DICT_SERVICE_SET_DONE_AND_RESPONSE(done, proto::SUCCESS, "success");
    }

    int DictManager::load_snapshot() {
        BAIDU_SCOPED_LOCK(_dict_mutex);
        BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
        TLOG_INFO("start to load files snapshot");
        _dicts.clear();
        _tombstone_dicts.clear();
        auto r = DictMeta::get_rkv()->scan(DictManager::load_dict_snapshot);
        if(!r.ok()) {
            return -1;
        }
        TLOG_INFO("load files snapshot done");
        return 0;
    }

    int DictManager::load_snapshot_file(const std::string &file_path) {
        auto fname = turbo::filesystem::path(file_path).filename();
        auto local_path = turbo::filesystem::path(FLAGS_dict_data_root) / fname;
        std::error_code ec;
        if (!turbo::filesystem::exists(local_path, ec)) {
            if (ec) {
                TLOG_ERROR("{}", ec.message());
                return -1;
            }
            turbo::filesystem::copy(file_path, local_path, ec);
            if (ec) {
                TLOG_ERROR("{}", ec.message());
                return -1;
            }
            return 0;
        }
        auto s_size = turbo::filesystem::file_size(file_path, ec);
        if (ec) {
            TLOG_ERROR("{}", ec.message());
            return -1;
        }
        auto l_size = turbo::filesystem::file_size(local_path, ec);
        if (ec) {
            TLOG_ERROR("{}", ec.message());
            return -1;
        }
        if (l_size != s_size) {
            turbo::filesystem::remove(local_path, ec);
            turbo::filesystem::copy(file_path, local_path, ec);
            if (ec) {
                TLOG_ERROR("{}", ec.message());
                return -1;
            }
        }
        return 0;
    }

    int DictManager::load_dict_snapshot(const std::string &key, const std::string &value) {
        proto::DictEntity dict_pb;
        if (!dict_pb.ParseFromString(value)) {
            TLOG_ERROR("parse from pb fail when load database snapshot, key:{}", value);
            return -1;
        }
        auto this_ptr = DictManager::get_instance();
        if (dict_pb.tombstone()) {
            if (this_ptr->_tombstone_dicts.find(dict_pb.name()) == this_ptr->_tombstone_dicts.end()) {
                this_ptr->_tombstone_dicts[dict_pb.name()] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
            }
            auto it = this_ptr->_tombstone_dicts.find(dict_pb.name());
            turbo::ModuleVersion version(dict_pb.version().major(), dict_pb.version().minor(),
                                         dict_pb.version().patch());
            it->second[version] = dict_pb;
        } else {
            if (this_ptr->_dicts.find(dict_pb.name()) == this_ptr->_dicts.end()) {
                this_ptr->_dicts[dict_pb.name()] = std::map<turbo::ModuleVersion, EA::proto::DictEntity>();
            }
            auto it = this_ptr->_dicts.find(dict_pb.name());
            turbo::ModuleVersion version(dict_pb.version().major(), dict_pb.version().minor(),
                                         dict_pb.version().patch());
            it->second[version] = dict_pb;
        }

        return 0;
    }

    int DictManager::save_snapshot(const std::string &base_dir, const std::string &prefix,
                                   std::vector<std::string> &files) {
        //
        std::error_code ec;
        {
            BAIDU_SCOPED_LOCK(_dict_mutex);
            for (auto it = _dicts.begin(); it != _dicts.end(); ++it) {
                for (auto pit = it->second.begin(); pit != it->second.end(); ++pit) {
                    auto filename = make_dict_filename(pit->second.name(), pit->first, pit->second.ext());
                    std::string file_path = turbo::Format("{}/{}", prefix, filename);
                    std::string target = base_dir + file_path;
                    std::string source = turbo::Format("{}/{}", FLAGS_dict_data_root, filename);

                    if (!turbo::filesystem::exists(source, ec)) {
                        continue;
                    }
                    turbo::filesystem::create_hard_link(source, target, ec);
                    if (ec) {
                        TLOG_ERROR("dict snapshot error: {}", ec.message());
                        return -1;
                    }
                    files.push_back(file_path);
                }
            }
        }
        {
            BAIDU_SCOPED_LOCK(_tombstone_dict_mutex);
            for (auto it = _tombstone_dicts.begin(); it != _tombstone_dicts.end(); ++it) {
                for (auto pit = it->second.begin(); pit != it->second.end(); ++pit) {
                    auto filename = make_dict_filename(pit->second.name(), pit->first, pit->second.ext());
                    std::string file_path = turbo::Format("{}/{}", prefix, filename);
                    std::string target = base_dir + file_path;
                    std::string source = turbo::Format("{}/{}", FLAGS_dict_data_root, filename);

                    if (!turbo::filesystem::exists(source, ec)) {
                        continue;
                    }
                    turbo::filesystem::create_hard_link(source, target, ec);
                    if (ec) {
                        TLOG_ERROR("dict snapshot error: {}", ec.message());
                        return -1;
                    }
                    files.push_back(file_path);
                }
            }
        }
        return 0;
    }

    std::string DictManager::make_dict_key(const std::string &name, const turbo::ModuleVersion &version) {
        return name + version.to_string();
    }

    turbo::Status
    DictManager::transfer_info_to_entity(const EA::proto::DictInfo *info, EA::proto::DictEntity *entity) {
        entity->set_upload_size(0);
        entity->set_finish(false);
        entity->set_tombstone(false);
        entity->set_name(info->name());
        entity->set_time(info->time());
        entity->set_ext(info->ext());
        entity->set_size(info->size());
        if (!info->has_cksm()) {
            return turbo::InvalidArgumentError("no chsm");
        }
        entity->set_cksm(info->cksm());
        if (!info->has_time()) {
            return turbo::InvalidArgumentError("no time");
        }
        entity->set_time(info->time());
        if (!info->has_version()) {
            return turbo::InvalidArgumentError("no version");
        }
        *(entity->mutable_version()) = info->version();
        return turbo::OkStatus();
    }

    void DictManager::transfer_entity_to_info(const EA::proto::DictEntity *entity, EA::proto::DictInfo *info) {
        info->set_upload_size(entity->upload_size());
        info->set_finish(entity->finish());
        info->set_tombstone(entity->tombstone());
        info->set_name(entity->name());
        info->set_time(entity->time());
        info->set_ext(entity->ext());
        info->set_size(entity->size());
        info->set_cksm(entity->cksm());
        info->set_time(entity->time());
        *(info->mutable_version()) = entity->version();
    }

    std::string DictManager::make_dict_filename(const std::string &name, const turbo::ModuleVersion &version,
                                                const std::string &ext) {
        if (!ext.empty()) {
            return turbo::Format("{}.{}.{}", name, ext, version.to_string());
        } else {
            return turbo::Format("{}.{}", name, version.to_string());
        }

    }

    std::string DictManager::make_dict_store_path(const std::string &name, const turbo::ModuleVersion &version,
                                                  const std::string &ext) {
        if (!ext.empty()) {
            return turbo::Format("{}/{}.{}.{}", FLAGS_dict_data_root, name, ext, version.to_string());
        } else {
            return turbo::Format("{}/{}.{}", FLAGS_dict_data_root, name, version.to_string());
        }
    }

}  // namespace EA::dict