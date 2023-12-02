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

#include "ea/flags/discovery.h"
#include "gflags/gflags.h"

namespace EA {

    /// for discovery
    DEFINE_string(discovery_server_peers, "127.0.0.1:8010", "discovery server peers");
    DEFINE_int32(discovery_replica_number, 3, "Meta replica num");
    DEFINE_int32(discovery_snapshot_interval_s, 600, "raft snapshot interval(s)");
    DEFINE_int32(discovery_election_timeout_ms, 1000, "raft election timeout(ms)");
    DEFINE_string(discovery_raft_group, "discovery_raft", "discovery raft group");
    DEFINE_string(discovery_log_uri, "local://./discovery/raft_log/", "raft log uri");
    DEFINE_string(discovery_stable_uri, "local://./discovery/raft_data/stable", "raft stable path");
    DEFINE_string(discovery_snapshot_uri, "local://./discovery/raft_data/snapshot", "raft snapshot path");
    DEFINE_int64(discovery_check_migrate_interval_us, 60 * 1000 * 1000LL, "check discovery server migrate interval (60s)");
    DEFINE_int32(discovery_tso_snapshot_interval_s, 60, "tso raft snapshot interval(s)");
    DEFINE_string(discovery_db_path, "./discovery/rocks_db", "rocks db path");
    DEFINE_string(discovery_listen,"127.0.0.1:8010", "discovery listen addr");
    DEFINE_int32(discovery_request_timeout, 30000,
                 "discovery as server request timeout, default:30000ms");
    DEFINE_int32(discovery_connect_timeout, 5000,
                 "discovery as server connect timeout, default:5000ms");

    DEFINE_string(backup_discovery_server_peers, "", "backup_discovery_server_peers");
    DEFINE_int64(time_between_discovery_connect_error_ms, 0, "time_between_discovery_connect_error_ms. default(0ms)");


}  // namespace EA
