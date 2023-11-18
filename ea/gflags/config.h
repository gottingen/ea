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

#ifndef EA_GFLAGS_CONFIG_H_
#define EA_GFLAGS_CONFIG_H_

#include "gflags/gflags_declare.h"

namespace EA {

    /// for eaconfig
    DECLARE_int64(config_time_between_connect_error_ms);
    DECLARE_string(config_server_bns);
    DECLARE_string(config_backup_server_bns);
    DECLARE_int32(config_request_timeout);
    DECLARE_int32(config_connect_timeout);
    DECLARE_int32(config_snapshot_interval_s);
    DECLARE_int32(config_election_timeout_ms);
    DECLARE_string(config_log_uri);
    DECLARE_string(config_stable_uri);
    DECLARE_string(config_snapshot_uri);
    DECLARE_int64(config_check_migrate_interval_us);
    DECLARE_string(config_db_path);
    DECLARE_string(config_snapshot_sst);
    DECLARE_string(config_listen);
    DECLARE_int32(config_replica_number);

}  // namespace EA
#endif  // EA_GFLAGS_CONFIG_H_
