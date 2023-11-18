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

#include "ea/rdb/rkv.h"
#include "ea/base/tlog.h"

namespace EA::rdb {

    turbo::Status Rkv::put(const std::string &key, const std::string &value) {
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        auto status = _storage->put(write_option, _handle, rocksdb::Slice(make_key(key)), rocksdb::Slice(value));
        if (!status.ok()) {
            TLOG_WARN("put rocksdb fail, err_msg: {}, key: {}, value: {}",
                      status.ToString(), key, value);
            return turbo::MakeStatus(static_cast<int>(status.code()), status.ToString());
        }
        return turbo::OkStatus();
    }

    turbo::Status Rkv::mput(const std::vector<std::string> &keys,
                                      const std::vector<std::string> &values) {
        if (keys.size() != values.size()) {
            TLOG_WARN("input keys'size is not equal to values' size");
            return turbo::InvalidArgumentError("input keys'size is not equal to values' size");
        }
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (size_t i = 0; i < keys.size(); ++i) {
            batch.Put(_handle, make_key(keys[i]), values[i]);
        }
        auto status = _storage->write(write_option, &batch);
        if (!status.ok()) {
            TLOG_WARN("put batch to rocksdb fail, err_msg: {}",
                      status.ToString());
            return turbo::MakeStatus(static_cast<int>(status.code()), status.ToString());
        }
        return turbo::OkStatus();
    }

    turbo::Status Rkv::get(const std::string &key, std::string *value) {
        rocksdb::ReadOptions options;
        auto status = _storage->get(options, _handle, rocksdb::Slice(make_key(key)), value);
        if (!status.ok()) {
            return turbo::MakeStatus(static_cast<int>(status.code()), status.ToString());
        }
        return turbo::OkStatus();
    }

    turbo::Status Rkv::remove(const std::string &key) {
        return mremove({make_key(key)});
    }

    turbo::Status Rkv::mremove(const std::vector<std::string> &keys) {
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (auto &key: keys) {
            batch.Delete(_handle, make_key(key));
        }
        auto status = _storage->write(write_option, &batch);
        if (!status.ok()) {
            TLOG_WARN("delete batch to rocksdb fail, err_msg: {}", status.ToString());
            return turbo::MakeStatus(static_cast<int>(status.code()), status.ToString());
        }
        return turbo::OkStatus();
    }

    std::string Rkv::make_key(const std::string& user_key) {
        return Storage::kRkvPrefix + _namespace + user_key;
    }

    void Rkv::init(const std::string&ns) {
        if(_storage) {
            return;
        }
        _namespace = ns;
        _storage = Storage::get_instance();
        _handle = _storage->get_rdb_kv_handle();
    }

    turbo::Status Rkv::scan(const scan_func &func) {
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = true;
        read_options.total_order_seek = false;
        auto config_prefix =  Storage::kRkvPrefix + _namespace;
        std::unique_ptr<rocksdb::Iterator> iter(
                _storage->new_iterator(read_options, _handle));
        iter->Seek(config_prefix);
        for (; iter->Valid(); iter->Next()) {
            if(!func(iter->key().ToString(),iter->value().ToString())) {
                return turbo::InternalError("");
            }
        }
        return turbo::OkStatus();
    }

}  // namespace EA::rdb
