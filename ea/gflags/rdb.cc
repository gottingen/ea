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

#include "ea/gflags/rdb.h"
#include "gflags/gflags.h"
#include "brpc/reloadable_flags.h"
#include "rocksdb/perf_context.h"

namespace EA {

    /// for store engine
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
    DEFINE_bool(use_direct_io_for_flush_and_compaction, false, "default false");
    DEFINE_bool(use_direct_reads, false, "default false");

}  // namespace EA
