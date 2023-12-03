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



#ifndef EA_FLAGS_DISCOVERY_H_
#define EA_FLAGS_DISCOVERY_H_

#include "gflags/gflags_declare.h"

namespace EA {

    /// for discovery
    DECLARE_string(discovery_server_peers);
    DECLARE_int32(discovery_replica_number);
    DECLARE_int32(discovery_snapshot_interval_s);
    DECLARE_int32(discovery_election_timeout_ms);
    DECLARE_string(discovery_raft_group);
    DECLARE_string(discovery_log_uri);
    DECLARE_string(discovery_stable_uri);
    DECLARE_string(discovery_snapshot_uri);
    DECLARE_int64(discovery_check_migrate_interval_us);
    DECLARE_int32(discovery_tso_snapshot_interval_s);
    DECLARE_string(discovery_db_path);
    DECLARE_string(discovery_listen);
    DECLARE_int32(discovery_request_timeout);
    DECLARE_int32(discovery_connect_timeout);
    DECLARE_string(backup_discovery_server_peers);
    DECLARE_int64(time_between_discovery_connect_error_ms);

}  // namespace EA

#endif  // EA_FLAGS_DISCOVERY_H_
