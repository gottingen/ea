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

#include "ea/gflags/config.h"
#include "gflags/gflags.h"

namespace EA {

    /// for config
    DEFINE_int64(config_time_between_connect_error_ms, 0, "config_time_between_connect_error_ms. default(0ms)");
    DEFINE_string(config_server_bns, "127.0.0.1:8020", "config server bns");
    DEFINE_string(config_backup_server_bns, "", "config_backup_server_bns");
    DEFINE_int32(config_request_timeout, 30000, "config as server request timeout, default:30000ms");
    DEFINE_int32(config_connect_timeout, 5000, "config as server connect timeout, default:5000ms");
    DEFINE_int32(config_snapshot_interval_s, 600, "raft snapshot interval(s)");
    DEFINE_int32(config_election_timeout_ms, 1000, "raft election timeout(ms)");
    DEFINE_string(config_log_uri, "local://./raft_data/config/log", "raft log uri");
    DEFINE_string(config_stable_uri, "local://./raft_data/config/stable", "raft stable path");
    DEFINE_string(config_snapshot_uri, "local://./raft_data/config/snapshot", "raft snapshot path");
    DEFINE_int64(config_check_migrate_interval_us, 60 * 1000 * 1000LL, "check config server migrate interval (60s)");
    DEFINE_string(config_db_path, "./rocks_db/config", "rocks db path");
    DEFINE_string(config_snapshot_sst, "/config.sst","rocks sst file for config");
    DEFINE_string(config_listen,"127.0.0.1:8020", "config listen addr");
    DEFINE_int32(config_replica_number, 1, "config service replica num");

}  // namespace EA
