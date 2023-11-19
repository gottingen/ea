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

#include "ea/gflags/plugin.h"
#include "gflags/gflags.h"
#include "brpc/reloadable_flags.h"
#include "rocksdb/perf_context.h"

namespace EA {

    /// for plugin
    DEFINE_int64(plugin_time_between_connect_error_ms, 0, "plugin_time_between_connect_error_ms. default(0ms)");
    DEFINE_string(plugin_server_bns, "127.0.0.1:8030", "plugin server bns");
    DEFINE_string(plugin_backup_server_bns, "", "plugin_backup_server_bns");
    DEFINE_int32(plugin_request_timeout, 30000, "service as server request timeout, default:30000ms");
    DEFINE_int32(plugin_connect_timeout, 5000, "service as server connect timeout, default:5000ms");
    DEFINE_int32(plugin_snapshot_interval_s, 600, "raft snapshot interval(s)");
    DEFINE_int32(plugin_election_timeout_ms, 1000, "raft election timeout(ms)");
    DEFINE_string(plugin_log_uri, "local://./raft_data/plugin/log", "raft log uri");
    DEFINE_string(plugin_stable_uri, "local://./raft_data/plugin/stable", "raft stable path");
    DEFINE_string(plugin_snapshot_uri, "local://./raft_data/plugin/snapshot", "raft snapshot path");
    DEFINE_int64(plugin_check_migrate_interval_us, 60 * 1000 * 1000LL, "check plugin server migrate interval (60s)");
    DEFINE_string(plugin_data_root,"./data/plugin","plugin data dir");
    DEFINE_string(plugin_db_path, "./rocks_db/plugin", "rocks db path");
    DEFINE_string(plugin_snapshot_sst, "/plugin_meta.sst","rocks sst file for service");
    DEFINE_string(plugin_listen,"127.0.0.1:8030", "plugin listen addr");
    DEFINE_int32(plugin_replica_number, 1, "plugin service replica num");

}  // namespace EA
