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

//
// Created by jeff on 23-11-29.
//

#ifndef EA_CLI_RAFT_CMD_H_
#define EA_CLI_RAFT_CMD_H_

#include "turbo/flags/flags.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include "turbo/format/table.h"
#include "turbo/base/result_status.h"
#include "ea/client/discovery_sender.h"
#include <string>

namespace EA::cli {

    struct RaftOptionContext {
        static RaftOptionContext *get_instance() {
            static RaftOptionContext ins;
            return &ins;
        }
        // for config
        std::string raft_group;
        std::string opt_peer;
        std::string cluster;
        std::string new_leader;
        int64_t     vote_time_ms;
        std::vector<std::string> old_peers;
        std::vector<std::string> new_peers;
        EA::client::DiscoverySender sender;
        std::string  discovery_server;
        bool force{false};
    };

    struct RaftCmd {

        static void setup_raft_cmd(turbo::App &app);

        static void run_raft_cmd(turbo::App *app);

        static void run_status_cmd();

        static void run_snapshot_cmd();

        static void run_vote_cmd();

        static void run_shutdown_cmd();

        static void run_set_cmd();

        static void run_trans_cmd();

        static turbo::ResultStatus<int> to_region_id();

        static turbo::Table show_raft_result(EA::discovery::RaftControlResponse &res);
    };

    }  // namespace EA::cli

#endif  // EA_CLI_RAFT_CMD_H_
