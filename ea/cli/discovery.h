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
// Created by jeff on 23-11-30.
//

#ifndef EA_CLI_DISCOVERY_H_
#define EA_CLI_DISCOVERY_H_

#include "turbo/flags/flags.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include "turbo/base/status.h"
#include "turbo/format/table.h"
#include <string>
#include "ea/client/config_client.h"

namespace EA::cli {


    struct DiscoveryOptionContext {
        static DiscoveryOptionContext *get_instance() {
            static DiscoveryOptionContext ins;
            return &ins;
        }

        // for config
        std::string namespace_name;
        std::string servlet_name;
        std::string zone_name;
        std::string env;
        std::string color;
        std::string status;
        std::string address;
        std::string dump_file;
        std::string json_file;
        bool quiet{false};
        int64_t weight{-1};
    };

    struct DiscoveryCmd {
        static void setup_discovery_cmd(turbo::App &app);

        static void run_discovery_cmd(turbo::App *app);

        static void run_discovery_add_instance_cmd();

        static void run_discovery_remove_instance_cmd();

        static void run_discovery_update_instance_cmd();

        static void run_discovery_list_instance_cmd();

        static void run_discovery_info_instance_cmd();

        static void run_discovery_dump_cmd();

        [[nodiscard]] static turbo::Status
        make_discovery_add_instance(EA::discovery::DiscoveryManagerRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_remove_instance(EA::discovery::DiscoveryManagerRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_update_instance(EA::discovery::DiscoveryManagerRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_list_instance(EA::discovery::DiscoveryQueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_discovery_info_instance(EA::discovery::DiscoveryQueryRequest *req);

        [[nodiscard]] static turbo::ResultStatus<EA::discovery::Status> string_to_status(const std::string &status);

        static turbo::Table show_query_instance_list_response(const EA::discovery::DiscoveryQueryResponse &res);
        static turbo::Table show_query_instance_info_response(const EA::discovery::DiscoveryQueryResponse &res);
    };
}  // namespace EA::cli

#endif  // EA_CLI_DISCOVERY_H_
