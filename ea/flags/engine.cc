// Copyright 2023 The Elastic Architecture Infrastructure Authors.
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

#include "ea/flags/engine.h"
#include "gflags/gflags.h"
#include "brpc/reloadable_flags.h"
#include "rocksdb/perf_context.h"

namespace EA {
    /// for store engine
    DEFINE_int64(flush_memtable_interval_us, 10 * 60 * 1000 * 1000LL, "flush memtable interval, default(10 min)");
    DEFINE_bool(cstore_scan_fill_cache, true, "cstore_scan_fill_cache");
    DEFINE_int32(rocks_transaction_lock_timeout_ms, 20000,
                 "rocksdb transaction_lock_timeout, real lock_time is 'time + rand_less(time)' (ms)");
    DEFINE_int32(rocks_default_lock_timeout_ms, 30000, "rocksdb default_lock_timeout(ms)");
    DEFINE_bool(rocks_use_partitioned_index_filters, false, "rocksdb use Partitioned Index Filters");
    DEFINE_bool(rocks_skip_stats_update_on_db_open, false, "rocks_skip_stats_update_on_db_open");
    DEFINE_int32(rocks_block_size, 64 * 1024, "rocksdb block_cache size, default: 64KB");
    DEFINE_int64(rocks_block_cache_size_mb, 8 * 1024, "rocksdb block_cache_size_mb, default: 8G");
    DEFINE_uint64(rocks_hard_pending_compaction_g, 256, "rocksdb hard_pending_compaction_bytes_limit , default: 256G");
    DEFINE_uint64(rocks_soft_pending_compaction_g, 64, "rocksdb soft_pending_compaction_bytes_limit , default: 64G");
    DEFINE_uint64(rocks_compaction_readahead_size, 0, "rocksdb compaction_readahead_size, default: 0");
    DEFINE_int32(rocks_data_compaction_pri, 3, "rocksdb data_cf compaction_pri, default: 3(kMinOverlappingRatio)");
    DEFINE_double(rocks_level_multiplier, 10, "data_cf rocksdb max_bytes_for_level_multiplier, default: 10");
    DEFINE_double(rocks_high_pri_pool_ratio, 0.5, "rocksdb cache high_pri_pool_ratio, default: 0.5");
    DEFINE_int32(rocks_max_open_files, 1024, "rocksdb max_open_files, default: 1024");
    DEFINE_int32(rocks_max_subcompactions, 4, "rocks_max_subcompactions");
    DEFINE_int32(rocks_max_background_compactions, 20, "max_background_compactions");
    DEFINE_bool(rocks_optimize_filters_for_hits, false, "rocks_optimize_filters_for_hits");
    DEFINE_int32(slowdown_write_sst_cnt, 10, "level0_slowdown_writes_trigger");
    DEFINE_int32(stop_write_sst_cnt, 40, "level0_stop_writes_trigger");
    DEFINE_bool(rocks_use_ribbon_filter, false,
                "use Ribbon filter:https://github.com/facebook/rocksdb/wiki/RocksDB-Bloom-Filter");
    DEFINE_bool(rocks_use_hyper_clock_cache, false,
                "use HyperClockCache:https://github.com/facebook/rocksdb/pull/10963");
    DEFINE_bool(rocks_use_sst_partitioner_fixed_prefix, false,
                "use SstPartitionerFixedPrefix:https://github.com/facebook/rocksdb/pull/6957");
    DEFINE_bool(rocks_kSkipAnyCorruptedRecords, false,
                "We ignore any corruption in the WAL and try to salvage as much data as possible");
    DEFINE_bool(rocks_data_dynamic_level_bytes, true,
                "rocksdb level_compaction_dynamic_level_bytes for data column_family, default true");
    DEFINE_int32(max_background_jobs, 24, "max_background_jobs");
    DEFINE_int32(max_write_buffer_number, 6, "max_write_buffer_number");
    DEFINE_int32(write_buffer_size, 128 * 1024 * 1024, "write_buffer_size");
    DEFINE_int32(min_write_buffer_number_to_merge, 2, "min_write_buffer_number_to_merge");
    DEFINE_int32(rocks_binlog_max_files_size_gb, 100, "binlog max size default 100G");
    DEFINE_int32(rocks_binlog_ttl_days, 7, "binlog ttl default 7 days");

    DEFINE_int32(level0_file_num_compaction_trigger, 5, "Number of files to trigger level-0 compaction");
    DEFINE_int32(max_bytes_for_level_base, 1024 * 1024 * 1024, "total size of level 1.");
    DEFINE_bool(enable_bottommost_compression, false, "enable zstd for bottommost_compression");
    DEFINE_int32(target_file_size_base, 128 * 1024 * 1024, "target_file_size_base");
    DEFINE_int32(addpeer_rate_limit_level, 1, "addpeer_rate_limit_level; "
                                              "0:no limit, 1:limit when stalling, 2:limit when compaction pending. default(1)");
    DEFINE_bool(delete_files_in_range, true, "delete_files_in_range");
    DEFINE_bool(l0_compaction_use_lz4, false, "L0 sst compaction use lz4 or not");
    DEFINE_bool(real_delete_old_binlog_cf, true, "default true");
    DEFINE_bool(rocksdb_fifo_allow_compaction, false, "default false");
    DEFINE_bool(use_direct_io_for_flush_and_compaction, false, "default false");
    DEFINE_bool(use_direct_reads, false, "default false");
    DEFINE_int32(level0_max_sst_num, 500, "max level0 num for fast importer");
    DEFINE_int32(rocksdb_cost_sample, 100, "rocksdb_cost_sample");
    DEFINE_int64(qps_statistics_minutes_ago, 60, "qps_statistics_minutes_ago, default: 1h"); // 默认以前一小时的统计信息作为参考
    DEFINE_int64(max_tokens_per_second, 100000, "max_tokens_per_second, default: 10w");
    DEFINE_int64(use_token_bucket, 0, "use_token_bucket, 0:close; 1:open, default: 0");
    DEFINE_int64(get_token_weight, 5, "get_token_weight, default: 5");
    DEFINE_int64(min_global_extended_percent, 40, "min_global_extended_percent, default: 40%");
    DEFINE_int64(token_bucket_adjust_interval_s, 60, "token_bucket_adjust_interval_s, default: 60s");
    DEFINE_int64(token_bucket_burst_window_ms, 10, "token_bucket_burst_window_ms, default: 10ms");
    DEFINE_int64(dml_use_token_bucket, 0, "dml_use_token_bucket, default: 0");
    DEFINE_int64(sql_token_bucket_timeout_min, 5, "sql_token_bucket_timeout_min, default: 5min");
    DEFINE_int64(qos_reject_interval_s, 30, "qos_reject_interval_s, default: 30s");
    DEFINE_int64(qos_reject_ratio, 90, "qos_reject_ratio, default: 90%");
    DEFINE_int64(qos_reject_timeout_s, 30 * 60, "qos_reject_timeout_s, default: 30min");
    DEFINE_int64(qos_reject_max_scan_ratio, 50, "qos_reject_max_scan_ratio, default: 50%");
    DEFINE_int64(qos_reject_growth_multiple, 100, "qos_reject_growth_multiple, default: 100倍");
    DEFINE_int64(qos_need_reject, 0, "qos_need_reject, default: 0");
    DEFINE_int64(sign_concurrency, 8, "sign_concurrency, default: 8");
    DEFINE_bool(disable_wal, false, "disable rocksdb interanal WAL log, only use raft log");
    DEFINE_int64(exec_1pc_out_fsm_timeout_ms, 5 * 1000, "exec 1pc out of fsm, timeout");
    DEFINE_int64(exec_1pc_in_fsm_timeout_ms, 100, "exec 1pc in fsm, timeout");
    DEFINE_int64(retry_interval_us, 500 * 1000, "retry interval ");
    DEFINE_int32(transaction_clear_delay_ms, 600 * 1000,
                 "delay duration to clear prepared and expired transactions");
    DEFINE_int32(long_live_txn_interval_ms, 300 * 1000,
                 "delay duration to clear prepared and expired transactions");
    DEFINE_int64(clean_finished_txn_interval_us, 600 * 1000 * 1000LL,
                 "clean_finished_txn_interval_us");
    DEFINE_int64(one_pc_out_fsm_interval_us, 20 * 1000 * 1000LL,
                 "clean_finished_txn_interval_us");
    // 分裂slow down max time：5s
    DEFINE_int32(transaction_query_primary_region_interval_ms, 15 * 1000,
                 "interval duration send request to primary region");
}
