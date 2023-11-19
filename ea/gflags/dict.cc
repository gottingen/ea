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

    /// for dict
    DEFINE_int64(dict_time_between_connect_error_ms, 0, "dict_time_between_connect_error_ms. default(0ms)");
    DEFINE_string(dict_server_bns, "127.0.0.1:8040", "dict server bns");
    DEFINE_string(dict_backup_server_bns, "", "dict_backup_server_bns");
    DEFINE_int32(dict_request_timeout, 30000, "dict as server request timeout, default:30000ms");
    DEFINE_int32(dict_connect_timeout, 5000, "dict as server connect timeout, default:5000ms");
    DEFINE_int32(dict_snapshot_interval_s, 600, "raft snapshot interval(s)");
    DEFINE_int32(dict_election_timeout_ms, 1000, "raft election timeout(ms)");
    DEFINE_string(dict_log_uri, "local://./raft_data/config/log", "raft log uri");
    DEFINE_string(dict_stable_uri, "local://./raft_data/dict/stable", "raft stable path");
    DEFINE_string(dict_snapshot_uri, "local://./raft_data/dict/snapshot", "raft snapshot path");
    DEFINE_int64(dict_check_migrate_interval_us, 60 * 1000 * 1000LL, "check dict server migrate interval (60s)");
    DEFINE_string(dict_data_root,"./data/dict","dict data dir");
    DEFINE_string(dict_db_path, "./rocks_db/dict", "rocks db path");
    DEFINE_string(dict_snapshot_sst, "/dict_meta.sst","rocks sst dict for service");
    DEFINE_string(dict_listen,"127.0.0.1:8040", "dict listen addr");
    DEFINE_int32(dict_replica_number, 1, "dict service replica num");

}
