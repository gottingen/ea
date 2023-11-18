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


#ifndef EA_RDB_STORAGE_H_
#define EA_RDB_STORAGE_H_

#include <string>
#include "rocksdb/db.h"
#include "rocksdb/convenience.h"
#include "rocksdb/slice.h"
#include "rocksdb/cache.h"
#include "rocksdb/listener.h"
#include "rocksdb/options.h"
#include "rocksdb/status.h"
#include "rocksdb/slice_transform.h"
#include "rocksdb/utilities/transaction.h"
#include "rocksdb/utilities/transaction_db.h"
#include "turbo/format/format.h"
#include "turbo/base/status.h"
#include "bvar/bvar.h"
#include "bthread/butex.h"
#include "bthread/mutex.h"

namespace EA::rdb {

    class Storage {
    public:
        static const std::string RDB_KV_CF;
        static std::atomic<int64_t> rdb_kv_cf_remove_range_count;
        static const std::string kRkvPrefix;
        static const std::string kMaxPrefix;
        virtual ~Storage() {}

        static Storage *get_instance() {
            static Storage _instance;
            return &_instance;
        }

        int32_t init(const std::string &path);

        rocksdb::Status write(const rocksdb::WriteOptions &options, rocksdb::WriteBatch *updates) {
            return _txn_db->Write(options, updates);
        }

        rocksdb::Status write(const rocksdb::WriteOptions &options,
                              rocksdb::ColumnFamilyHandle *column_family,
                              const std::vector<std::string> &keys,
                              const std::vector<std::string> &values) {
            rocksdb::WriteBatch batch;
            for (size_t i = 0; i < keys.size(); ++i) {
                batch.Put(column_family, keys[i], values[i]);
            }
            return _txn_db->Write(options, &batch);
        }

        rocksdb::Status get(const rocksdb::ReadOptions &options,
                            rocksdb::ColumnFamilyHandle *column_family,
                            const rocksdb::Slice &key,
                            std::string *value) {
            return _txn_db->Get(options, column_family, key, value);
        }

        rocksdb::Status put(const rocksdb::WriteOptions &options,
                            rocksdb::ColumnFamilyHandle *column_family,
                            const rocksdb::Slice &key,
                            const rocksdb::Slice &value) {
            return _txn_db->Put(options, column_family, key, value);
        }

        rocksdb::Transaction *begin_transaction(
                const rocksdb::WriteOptions &write_options,
                const rocksdb::TransactionOptions &txn_options) {
            return _txn_db->BeginTransaction(write_options, txn_options);
        }

        rocksdb::Status compact_range(const rocksdb::CompactRangeOptions &options,
                                      rocksdb::ColumnFamilyHandle *column_family,
                                      const rocksdb::Slice *begin,
                                      const rocksdb::Slice *end) {
            return _txn_db->CompactRange(options, column_family, begin, end);
        }

        rocksdb::Status flush(const rocksdb::FlushOptions &options,
                              rocksdb::ColumnFamilyHandle *column_family) {
            return _txn_db->Flush(options, column_family);
        }

        rocksdb::Status remove(const rocksdb::WriteOptions &options,
                               rocksdb::ColumnFamilyHandle *column_family,
                               const rocksdb::Slice &key) {
            return _txn_db->Delete(options, column_family, key);
        }

        // Consider setting ReadOptions::ignore_range_deletions = true to speed
        // up reads for key(s) that are known to be unaffected by range deletions.
        rocksdb::Status remove_range(const rocksdb::WriteOptions &options,
                                     rocksdb::ColumnFamilyHandle *column_family,
                                     const rocksdb::Slice &begin,
                                     const rocksdb::Slice &end,
                                     bool delete_files_in_range);


        rocksdb::Iterator *new_iterator(const rocksdb::ReadOptions &options,
                                        rocksdb::ColumnFamilyHandle *family) {
            return _txn_db->NewIterator(options, family);
        }

        rocksdb::Iterator *new_iterator(const rocksdb::ReadOptions &options, const std::string cf) {
            if (_column_families.count(cf) == 0) {
                return nullptr;
            }
            return _txn_db->NewIterator(options, _column_families[cf]);
        }

        turbo::Status dump_rkv(const std::string &path);

        turbo::Status clean_rkv();

        turbo::Status load_rkv(const std::string &path);

        rocksdb::Status ingest_external_file(rocksdb::ColumnFamilyHandle *family,
                                             const std::vector<std::string> &external_files,
                                             const rocksdb::IngestExternalFileOptions &options) {
            return _txn_db->IngestExternalFile(family, external_files, options);
        }

        rocksdb::ColumnFamilyHandle *get_rdb_kv_handle();

        rocksdb::TransactionDB *get_db() {
            return _txn_db;
        }


        rocksdb::Options get_options(rocksdb::ColumnFamilyHandle *family) {
            return _txn_db->GetOptions(family);
        }

        rocksdb::DBOptions get_db_options() {
            return _txn_db->GetDBOptions();
        }

        rocksdb::Cache *get_cache() {
            return _cache;
        }

        const rocksdb::Snapshot *get_snapshot() {
            return _txn_db->GetSnapshot();
        }

        void release_snapshot(const rocksdb::Snapshot *snapshot) {
            _txn_db->ReleaseSnapshot(snapshot);
        }

        void close() {
            delete _txn_db;
        }

        void set_flush_file_number(const std::string &cf_name, uint64_t file_number) {

        }

        uint64_t flush_file_number() {
            return _flush_file_number;
        }

        void collect_rocks_options();
    private:

        Storage();

        std::string _db_path;

        bool _is_init;

        rocksdb::TransactionDB *_txn_db;
        rocksdb::Cache *_cache;

        std::map<std::string, rocksdb::ColumnFamilyHandle *> _column_families;
        rocksdb::ColumnFamilyOptions _rdb_kv_option;
        uint64_t _flush_file_number = 0;
        bvar::Adder<int64_t> _rdb_kv_cf_remove_range_count;

        std::atomic<int32_t> _split_num;
        bthread::Mutex _options_mutex;
        std::unordered_map<std::string, std::string> _rocks_options;
        std::map<std::string, std::string> _defined_options;
        int64_t _oldest_ts_in_binlog_cf = 0;
    };
}  // namespace EA::rdb

#endif  // EA_RDB_STORAGE_H_
