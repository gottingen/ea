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


#include "ea/rdb/storage.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/statistics.h"
#include <iostream>
#include "ea/rdb/compact_listener.h"
#include "ea/rdb/sst_file_writer.h"
#include "ea/rdb/transaction_db_bthread_mutex.h"
#include "turbo/strings/numbers.h"
#include "ea/gflags/rdb.h"

namespace EA::rdb {

    const std::string Storage::RDB_KV_CF = "rdb_kv";
    std::atomic<int64_t> Storage::rdb_kv_cf_remove_range_count = {0};

    const std::string Storage::kRkvPrefix(1, 0x01);
    const std::string Storage::kMaxPrefix(1, 0xFF);

    Storage::Storage() : _is_init(false), _txn_db(nullptr),
                         _rdb_kv_cf_remove_range_count("rdb_kv_cf_remove_range_count") {
    }

    int32_t Storage::init(const std::string &path) {
        if (_is_init) {
            return 0;
        }
        std::shared_ptr<rocksdb::EventListener> my_listener = std::make_shared<MyListener>();
        rocksdb::BlockBasedTableOptions table_options;
        if (FLAGS_rocks_use_partitioned_index_filters) {
            // use Partitioned Index Filters
            // https://github.com/facebook/rocksdb/wiki/Partitioned-Index-Filters
            table_options.index_type = rocksdb::BlockBasedTableOptions::kTwoLevelIndexSearch;
            table_options.partition_filters = true;
            table_options.metadata_block_size = 4096;
            table_options.cache_index_and_filter_blocks = true;
            table_options.pin_top_level_index_and_filter = true;
            table_options.cache_index_and_filter_blocks_with_high_priority = true;
            table_options.pin_l0_filter_and_index_blocks_in_cache = true;
            table_options.block_cache = rocksdb::NewLRUCache(FLAGS_rocks_block_cache_size_mb * 1024 * 1024LL,
                                                             8, false, FLAGS_rocks_high_pri_pool_ratio);
            // 通过cache控制内存，不需要控制max_open_files
            FLAGS_rocks_max_open_files = -1;
        } else {
            table_options.data_block_index_type = rocksdb::BlockBasedTableOptions::kDataBlockBinaryAndHash;
            if (FLAGS_rocks_use_hyper_clock_cache) {
#if ROCKSDB_MAJOR == 7
                auto cache_opt = rocksdb::HyperClockCacheOptions(FLAGS_rocks_block_cache_size_mb * 1024 * 1024LL, FLAGS_rocks_block_size, 8);
                cache_opt.metadata_charge_policy = rocksdb::kDontChargeCacheMetadata; //cache会比rocks_block_cache_size_mb多占用少量内存
                table_options.block_cache = cache_opt.MakeSharedCache();
#endif
            } else {
                table_options.block_cache = rocksdb::NewLRUCache(FLAGS_rocks_block_cache_size_mb * 1024 * 1024LL, 8);
            }
        }
        table_options.format_version = 4;

        table_options.block_size = FLAGS_rocks_block_size;
        if (FLAGS_rocks_use_ribbon_filter) {
            table_options.filter_policy.reset(rocksdb::NewRibbonFilterPolicy(9.9));
        } else {
            table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10));
        }
        _cache = table_options.block_cache.get();
        rocksdb::Options db_options;
        db_options.IncreaseParallelism(FLAGS_max_background_jobs);
        db_options.create_if_missing = true;
        db_options.use_direct_reads = FLAGS_use_direct_reads;
        db_options.use_direct_io_for_flush_and_compaction = FLAGS_use_direct_io_for_flush_and_compaction;
        db_options.max_open_files = FLAGS_rocks_max_open_files;
        db_options.skip_stats_update_on_db_open = FLAGS_rocks_skip_stats_update_on_db_open;
        db_options.compaction_readahead_size = FLAGS_rocks_compaction_readahead_size;
        db_options.WAL_ttl_seconds = 10 * 60;
        db_options.WAL_size_limit_MB = 0;
        //打开后有些集群内存严重上涨
        //db_options.avoid_unnecessary_blocking_io = true;
        db_options.max_background_compactions = FLAGS_rocks_max_background_compactions;
        if (FLAGS_rocks_kSkipAnyCorruptedRecords) {
            db_options.wal_recovery_mode = rocksdb::WALRecoveryMode::kSkipAnyCorruptedRecords;
        }
        db_options.statistics = rocksdb::CreateDBStatistics();
        db_options.max_subcompactions = FLAGS_rocks_max_subcompactions;
        db_options.max_background_flushes = 2;
        db_options.env->SetBackgroundThreads(2, rocksdb::Env::HIGH);
        db_options.listeners.emplace_back(my_listener);
        rocksdb::TransactionDBOptions txn_db_options;
        TLOG_INFO("FLAGS_rocks_transaction_lock_timeout_ms:{} FLAGS_rocks_default_lock_timeout_ms:{}",
                  FLAGS_rocks_transaction_lock_timeout_ms, FLAGS_rocks_default_lock_timeout_ms);
        txn_db_options.transaction_lock_timeout = FLAGS_rocks_transaction_lock_timeout_ms;
        txn_db_options.default_lock_timeout = FLAGS_rocks_default_lock_timeout_ms;
        txn_db_options.custom_mutex_factory = std::shared_ptr<rocksdb::TransactionDBMutexFactory>(
                new TransactionDBBthreadFactory());
        //todo
        _rdb_kv_option.prefix_extractor.reset(
                rocksdb::NewFixedPrefixTransform(1));
        _rdb_kv_option.OptimizeLevelStyleCompaction();
        _rdb_kv_option.compaction_pri = rocksdb::kOldestSmallestSeqFirst;
        _rdb_kv_option.level_compaction_dynamic_level_bytes = FLAGS_rocks_data_dynamic_level_bytes;
        _rdb_kv_option.max_write_buffer_number_to_maintain = _rdb_kv_option.max_write_buffer_number;
        _db_path = path;
        // List Column Family
        std::vector<std::string> column_family_names;
        rocksdb::Status s;
        s = rocksdb::DB::ListColumnFamilies(db_options, path, &column_family_names);
        //db已存在
        if (s.ok()) {
            std::vector<rocksdb::ColumnFamilyDescriptor> column_family_desc;
            std::vector<rocksdb::ColumnFamilyHandle *> handles;
            for (auto &column_family_name: column_family_names) {
                if (column_family_name == RDB_KV_CF) {
                    column_family_desc.push_back(rocksdb::ColumnFamilyDescriptor(RDB_KV_CF, _rdb_kv_option));
                } else {
                    column_family_desc.push_back(
                            rocksdb::ColumnFamilyDescriptor(column_family_name,
                                                            rocksdb::ColumnFamilyOptions()));
                }
            }
            s = rocksdb::TransactionDB::Open(db_options,
                                             txn_db_options,
                                             path,
                                             column_family_desc,
                                             &handles,
                                             &_txn_db);
            if (s.ok()) {
                TLOG_INFO("reopen db:{} success", path);
                for (auto &handle: handles) {
                    _column_families[handle->GetName()] = handle;
                    TLOG_INFO("open column family:{}", handle->GetName());
                }
            } else {
                TLOG_ERROR("reopen db:{} fail, err_message:{}", path, s.ToString());
                return -1;
            }
        } else {
            // new db
            s = rocksdb::TransactionDB::Open(db_options, txn_db_options, path, &_txn_db);
            if (s.ok()) {
                TLOG_INFO("open db:{} success", path);
            } else {
                TLOG_ERROR("open db:{} fail, err_message:{}", path, s.ToString());
                return -1;
            }
        }

        if (0 == _column_families.count(RDB_KV_CF)) {
            rocksdb::ColumnFamilyHandle *rdb_kv_handle;
            s = _txn_db->CreateColumnFamily(_rdb_kv_option, RDB_KV_CF, &rdb_kv_handle);
            if (s.ok()) {
                TLOG_INFO("create column family success, column family: {}", RDB_KV_CF);
                _column_families[RDB_KV_CF] = rdb_kv_handle;
            } else {
                TLOG_ERROR("create column family fail, column family:{}, err_message:{}",
                           RDB_KV_CF, s.ToString());
                return -1;
            }
        }
        _is_init = true;
        collect_rocks_options();
        TLOG_INFO("rocksdb init success");
        return 0;
    }

    void Storage::collect_rocks_options() {
        // gflag -> option_name, 可以通过setOption动态改的参数
        _rocks_options["level0_file_num_compaction_trigger"] = "level0_file_num_compaction_trigger";
        _rocks_options["slowdown_write_sst_cnt"] = "level0_slowdown_writes_trigger";
        _rocks_options["stop_write_sst_cnt"] = "level0_stop_writes_trigger";
        _rocks_options["rocks_hard_pending_compaction_g"] = "hard_pending_compaction_bytes_limit"; // * 1073741824ull;
        _rocks_options["rocks_soft_pending_compaction_g"] = "soft_pending_compaction_bytes_limit"; // * 1073741824ull;
        _rocks_options["target_file_size_base"] = "target_file_size_base";
        _rocks_options["rocks_level_multiplier"] = "max_bytes_for_level_multiplier";
        _rocks_options["max_write_buffer_number"] = "max_write_buffer_number";
        _rocks_options["write_buffer_size"] = "write_buffer_size";
        _rocks_options["max_bytes_for_level_base"] = "max_bytes_for_level_base";
        _rocks_options["rocks_max_background_compactions"] = "max_background_compactions";
        _rocks_options["rocks_max_subcompactions"] = "max_subcompactions";
        _rocks_options["max_background_jobs"] = "max_background_jobs";
    }

    rocksdb::Status Storage::remove_range(const rocksdb::WriteOptions &options,
                                          rocksdb::ColumnFamilyHandle *column_family,
                                          const rocksdb::Slice &begin,
                                          const rocksdb::Slice &end,
                                          bool delete_files_in_range) {
        auto mata_cf = get_rdb_kv_handle();
        if (mata_cf != nullptr && column_family->GetID() == mata_cf->GetID()) {
            _rdb_kv_cf_remove_range_count << 1;
            rdb_kv_cf_remove_range_count++;
        }

        if (delete_files_in_range && FLAGS_delete_files_in_range) {
            auto s = rocksdb::DeleteFilesInRange(_txn_db, column_family, &begin, &end, false);
            if (!s.ok()) {
                return s;
            }
        }
        rocksdb::TransactionDBWriteOptimizations opt;
        opt.skip_concurrency_control = true;
        opt.skip_duplicate_key_check = true;
        rocksdb::WriteBatch batch;
        batch.DeleteRange(column_family, begin, end);
        return _txn_db->Write(options, opt, &batch);
    }

    rocksdb::ColumnFamilyHandle *Storage::get_rdb_kv_handle() {
        if (!_is_init) {
            TLOG_ERROR("rocksdb has not been inited");
            return nullptr;
        }
        if (0 == _column_families.count(RDB_KV_CF)) {
            TLOG_ERROR("rocksdb has no rdb kv column family");
            return nullptr;
        }
        return _column_families[RDB_KV_CF];
    }

    turbo::Status Storage::dump_rkv(const std::string &path) {
        rocksdb::ReadOptions read_options;
        read_options.prefix_same_as_start = false;
        read_options.total_order_seek = true;
        auto iter = new_iterator(read_options, get_rdb_kv_handle());
        iter->SeekToFirst();
        std::unique_ptr<rocksdb::Iterator> iter_lock(iter);
        rocksdb::Options option = get_options(get_rdb_kv_handle());
        SstFileWriter sst_writer(option);
        auto s = sst_writer.open(path);
        if (!s.ok()) {
            TLOG_WARN("Error while opening file {}, Error: {}", path,
                      s.ToString());
            return turbo::InternalError("Error while opening file {}, Error: {}", path,
                                        s.ToString());
        }
        for (; iter->Valid(); iter->Next()) {
            auto res = sst_writer.put(iter->key(), iter->value());
            if (!res.ok()) {
                TLOG_WARN("Error while adding Key: {}, Error: {}",
                          iter->key().ToString(),
                          s.ToString());
                return turbo::InternalError("Error while adding Key: {}, Error: {}",
                                            iter->key().ToString(),
                                            s.ToString());
            }
        }

        //close the file
        s = sst_writer.finish();
        if (!s.ok()) {
            TLOG_WARN("Error while finishing file {}, Error: {}", path,
                      s.ToString());
            return turbo::InternalError("Error while finishing file {}, Error: {}", path,
                                        s.ToString());
        }
        return turbo::OkStatus();
    }

    turbo::Status Storage::clean_rkv() {
        // clean local data
        std::string remove_start_key(kRkvPrefix);
        rocksdb::WriteOptions options;
        auto status = remove_range(options,get_rdb_kv_handle(), remove_start_key,kMaxPrefix,false);
        if (!status.ok()) {
            TLOG_ERROR("remove_range error when on clean rkv load: code={}, msg={}",
                       status.code(), status.ToString());
            return turbo::InternalError("remove_range error when on clean rkv load: code={}, msg={}",
                                        status.code(), status.ToString());
        } else {
            TLOG_WARN("remove range success when on clean rkv:code:{}, msg={}",
                      status.code(), status.ToString());
        }
        return turbo::OkStatus();
    }

    turbo::Status Storage::load_rkv(const std::string &path) {
        rocksdb::IngestExternalFileOptions ifo;
        auto res = ingest_external_file(get_rdb_kv_handle(),{path},ifo);
        if (!res.ok()) {
            TLOG_WARN("Error while load rkv file {}, Error {}",
                      path, res.ToString());
            return turbo::InternalError("Error while load rkv file {}, Error {}",
                                        path, res.ToString());

        }
        return turbo::OkStatus();
    }
}
