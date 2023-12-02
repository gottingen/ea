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


#include "ea/flags/log.h"
#include "gflags/gflags.h"

namespace EA {
    /// common
    DEFINE_int64(memory_gc_interval_s, 10, "mempry GC interval , default: 10s");
    DEFINE_int64(memory_stats_interval_s, 60, "mempry GC interval , default: 60s");
    DEFINE_int64(min_memory_use_size, 8589934592, "minimum memory use size , default: 8G");
    DEFINE_int64(min_memory_free_size_to_release, 2147483648, "minimum memory free size to release, default: 2G");
    DEFINE_int64(mem_tracker_gc_interval_s, 60, "do memory limit when row number more than #, default: 60");
    DEFINE_int64(process_memory_limit_bytes, -1, "all memory use size, default: -1");
    DEFINE_int64(query_memory_limit_ratio, 90, "query memory use ratio , default: 90%");
    DEFINE_bool(need_health_check, true, "need_health_check");
    DEFINE_int32(raft_write_concurrency, 40, "raft_write concurrency, default:40");
    DEFINE_int32(service_write_concurrency, 40, "service_write concurrency, default:40");
    DEFINE_int32(snapshot_load_num, 4, "snapshot load concurrency, default 4");
    DEFINE_int32(baikal_heartbeat_concurrency, 10, "baikal heartbeat concurrency, default:10");
    DEFINE_int64(incremental_info_gc_time, 600 * 1000 * 1000, "time interval to clear incremental info");
    DEFINE_bool(enable_self_trace, true, "open SELF_TRACE log");
    DEFINE_bool(servitysinglelog, true, "diff servity message in seperate logfile");
    DEFINE_bool(open_service_write_concurrency, true, "open service_write_concurrency, default: true");
    DEFINE_bool(schema_ignore_case, false, "whether ignore case when match db/table name");
    DEFINE_bool(disambiguate_select_name, false, "whether use the first when select name is ambiguous, default false");
    DEFINE_int32(new_sign_read_concurrency, 10, "new_sign_read concurrency, default:20");
    DEFINE_bool(open_new_sign_read_concurrency, false, "open new_sign_read concurrency, default: false");
    DEFINE_bool(need_verify_ddl_permission, false, "default true");
    DEFINE_int32(histogram_split_threshold_percent, 50, "histogram_split_threshold default 0.5");
    DEFINE_int32(limit_slow_sql_size, 50, "each sign to slow query sql counts, default: 50");
    DEFINE_bool(like_predicate_use_re2, false, "LikePredicate use re2");
    DEFINE_bool(transfor_hll_raw_to_sparse, false, "try transfor raw hll to sparse");
}
