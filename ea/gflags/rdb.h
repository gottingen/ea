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


#ifndef EA_GFLAGS_RDB_H_
#define EA_GFLAGS_RDB_H_

#include "gflags/gflags_declare.h"

namespace EA {

    /// for store engine
    DECLARE_int32(rocks_transaction_lock_timeout_ms);
    DECLARE_int32(rocks_default_lock_timeout_ms);
    DECLARE_bool(rocks_use_partitioned_index_filters);
    DECLARE_bool(rocks_skip_stats_update_on_db_open);
    DECLARE_int32(rocks_block_size);
    DECLARE_int64(rocks_block_cache_size_mb);
    DECLARE_uint64(rocks_hard_pending_compaction_g);
    DECLARE_uint64(rocks_soft_pending_compaction_g);
    DECLARE_uint64(rocks_compaction_readahead_size);
    DECLARE_int32(rocks_data_compaction_pri);
    DECLARE_double(rocks_level_multiplier);
    DECLARE_double(rocks_high_pri_pool_ratio);
    DECLARE_int32(rocks_max_open_files);
    DECLARE_int32(rocks_max_subcompactions);
    DECLARE_int32(rocks_max_background_compactions);
    DECLARE_bool(rocks_optimize_filters_for_hits);
    DECLARE_int32(slowdown_write_sst_cnt);
    DECLARE_int32(stop_write_sst_cnt);
    DECLARE_bool(rocks_use_ribbon_filter);
    DECLARE_bool(rocks_use_hyper_clock_cache);
    DECLARE_bool(rocks_use_sst_partitioner_fixed_prefix);
    DECLARE_bool(rocks_kSkipAnyCorruptedRecords);
    DECLARE_bool(rocks_data_dynamic_level_bytes);
    DECLARE_int32(max_background_jobs);
    DECLARE_int32(max_write_buffer_number);
    DECLARE_int32(write_buffer_size);
    DECLARE_int32(min_write_buffer_number_to_merge);
    DECLARE_int32(rocks_binlog_max_files_size_gb);
    DECLARE_int32(rocks_binlog_ttl_days);
    DECLARE_int32(level0_file_num_compaction_trigger);
    DECLARE_int32(max_bytes_for_level_base);
    DECLARE_bool(enable_bottommost_compression);
    DECLARE_int32(target_file_size_base);
    DECLARE_int32(addpeer_rate_limit_level);
    DECLARE_bool(delete_files_in_range);
    DECLARE_bool(use_direct_io_for_flush_and_compaction);
    DECLARE_bool(use_direct_reads);

}  // namespace EA

#endif  // EA_GFLAGS_RDB_H_
