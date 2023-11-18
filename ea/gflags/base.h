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
//
// Created by jeff on 23-11-19.
//

#ifndef EA_GFLAGS_BASE_H_
#define EA_GFLAGS_BASE_H_

#include "gflags/gflags_declare.h"

namespace EA {

    /// for common
    DECLARE_int64(memory_gc_interval_s);
    DECLARE_int64(memory_stats_interval_s);
    DECLARE_int64(min_memory_use_size);
    DECLARE_int64(min_memory_free_size_to_release);
    DECLARE_int64(mem_tracker_gc_interval_s);
    DECLARE_int64(process_memory_limit_bytes);
    DECLARE_int64(query_memory_limit_ratio);
    DECLARE_bool(need_health_check);
    DECLARE_int32(raft_write_concurrency);
    DECLARE_int32(service_write_concurrency);
    DECLARE_int32(snapshot_load_num);
    DECLARE_int64(incremental_info_gc_time);
    DECLARE_bool(enable_self_trace);
    DECLARE_bool(servitysinglelog);
    DECLARE_bool(open_service_write_concurrency);
    DECLARE_bool(schema_ignore_case);
    DECLARE_bool(disambiguate_select_name);
    DECLARE_int32(new_sign_read_concurrency);
    DECLARE_bool(open_new_sign_read_concurrency);
    DECLARE_bool(need_verify_ddl_permission);
    DECLARE_int32(histogram_split_threshold_percent);
    DECLARE_int32(limit_slow_sql_size);
    DECLARE_bool(like_predicate_use_re2);
    DECLARE_bool(transfor_hll_raw_to_sparse);

}  // namespace EA

#endif  // EA_GFLAGS_BASE_H_
