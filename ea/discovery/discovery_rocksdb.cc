// Copyright 2023 The Elastic AI Search Authors.
//
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


#include "ea/discovery/discovery_rocksdb.h"
#include "gflags/gflags.h"
#include "ea/flags/discovery.h"
#include "ea/base/tlog.h"

namespace EA::discovery {

    int DiscoveryRocksdb::init() {
        _rocksdb = RocksStorage::get_instance();
        if (!_rocksdb) {
            TLOG_ERROR("create rocksdb handler failed");
            return -1;
        }
        int ret = _rocksdb->init(FLAGS_discovery_db_path);
        if (ret != 0) {
            TLOG_ERROR("rocksdb init failed: code:{}", ret);
            return -1;
        }
        _handle = _rocksdb->get_meta_info_handle();
        TLOG_WARN("rocksdb init success, db_path:{}", FLAGS_discovery_db_path);
        return 0;
    }

    int DiscoveryRocksdb::put_discovery_info(const std::string &key, const std::string &value) {
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        auto status = _rocksdb->put(write_option, _handle, rocksdb::Slice(key), rocksdb::Slice(value));
        if (!status.ok()) {
            TLOG_WARN("put rocksdb fail, err_msg: {}, key: {}, value: {}",
                       status.ToString(), key, value);
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::put_discovery_info(const std::vector<std::string> &keys,
                                   const std::vector<std::string> &values) {
        if (keys.size() != values.size()) {
            TLOG_WARN("input keys'size is not equal to values' size");
            return -1;
        }
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (size_t i = 0; i < keys.size(); ++i) {
            batch.Put(_handle, keys[i], values[i]);
        }
        auto status = _rocksdb->write(write_option, &batch);
        if (!status.ok()) {
            TLOG_WARN("put batch to rocksdb fail, err_msg: {}",
                       status.ToString());
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::get_discovery_info(const std::string &key, std::string *value) {
        rocksdb::ReadOptions options;
        auto status = _rocksdb->get(options, _handle, rocksdb::Slice(key), value);
        if (!status.ok()) {
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::remove_discovery_info(const std::vector<std::string> &keys) {
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (auto &key: keys) {
            batch.Delete(_handle, key);
        }
        auto status = _rocksdb->write(write_option, &batch);
        if (!status.ok()) {
            TLOG_WARN("delete batch to rocksdb fail, err_msg: {}", status.ToString());
            return -1;
        }
        return 0;
    }

    int DiscoveryRocksdb::write_discovery_info(const std::vector<std::string> &put_keys,
                                     const std::vector<std::string> &put_values,
                                     const std::vector<std::string> &delete_keys) {
        if (put_keys.size() != put_values.size()) {
            TLOG_WARN("input keys'size is not equal to values' size");
            return -1;
        }
        rocksdb::WriteOptions write_option;
        write_option.disableWAL = true;
        rocksdb::WriteBatch batch;
        for (size_t i = 0; i < put_keys.size(); ++i) {
            batch.Put(_handle, put_keys[i], put_values[i]);
        }
        for (auto &delete_key: delete_keys) {
            batch.Delete(_handle, delete_key);
        }
        auto status = _rocksdb->write(write_option, &batch);
        if (!status.ok()) {
            TLOG_WARN("write batch to rocksdb fail,  {}", status.ToString());
            return -1;
        }
        return 0;
    }
}  // namespace EA::discovery
