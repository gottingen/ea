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


#ifndef EA_GFLAGS_DICT_H_
#define EA_GFLAGS_DICT_H_

#include "gflags/gflags_declare.h"

namespace EA {

    /// for eadict
    DECLARE_int64(dict_time_between_connect_error_ms);
    DECLARE_string(dict_server_bns);
    DECLARE_string(dict_backup_server_bns);
    DECLARE_int32(dict_request_timeout);
    DECLARE_int32(dict_connect_timeout);
    DECLARE_int32(dict_snapshot_interval_s);
    DECLARE_int32(dict_election_timeout_ms);
    DECLARE_string(dict_log_uri);
    DECLARE_string(dict_stable_uri);
    DECLARE_string(dict_snapshot_uri);
    DECLARE_int64(dict_check_migrate_interval_us);
    DECLARE_string(dict_data_root);
    DECLARE_string(dict_db_path);
    DECLARE_string(dict_snapshot_sst);
    DECLARE_string(dict_listen);
    DECLARE_int32(dict_replica_number);

}  // namespace EA

#endif  // EA_GFLAGS_DICT_H_
