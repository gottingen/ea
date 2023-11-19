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
#ifndef EA_GFLAGS_PLUGIN_H_
#define EA_GFLAGS_PLUGIN_H_

#include "gflags/gflags_declare.h"

namespace EA {
    /// for eaplugin
    DECLARE_int64(plugin_time_between_connect_error_ms);
    DECLARE_string(plugin_server_bns);
    DECLARE_string(plugin_backup_server_bns);
    DECLARE_int32(plugin_request_timeout);
    DECLARE_int32(plugin_connect_timeout);
    DECLARE_int32(plugin_snapshot_interval_s);
    DECLARE_int32(plugin_election_timeout_ms);
    DECLARE_string(plugin_log_uri);
    DECLARE_string(plugin_stable_uri);
    DECLARE_string(plugin_snapshot_uri);
    DECLARE_int64(plugin_check_migrate_interval_us);
    DECLARE_string(plugin_data_root);
    DECLARE_string(plugin_db_path);
    DECLARE_string(plugin_snapshot_sst);
    DECLARE_string(plugin_listen);
    DECLARE_int32(plugin_replica_number);
}
#endif  // EA_GFLAGS_PLUGIN_H_
