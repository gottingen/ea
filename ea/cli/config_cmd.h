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

#ifndef EA_CLI_CONFIG_CMD_H_
#define EA_CLI_CONFIG_CMD_H_

#include "turbo/flags/flags.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include "turbo/base/status.h"
#include "turbo/format/table.h"
#include <string>
#include "ea/client/config_client.h"

namespace EA::cli {

    struct ConfigOptionContext {
        static ConfigOptionContext *get_instance() {
            static ConfigOptionContext ins;
            return &ins;
        }
        // for config
        std::string config_name;
        std::string config_data;
        std::string config_file;
        std::string config_version;
        std::string config_type;
        std::string config_json;
        std::string config_example;
        std::vector<std::string> config_watch_list;
        std::string config_watch_dir;
        bool clean_local;
        EA::servlet::ConfigInfo config_request;
    };

    struct ConfigCmd {
        static void setup_config_cmd(turbo::App &app);

        static void run_config_cmd(turbo::App *app);

        static void run_config_create_cmd();

        static void run_config_list_cmd();

        static void run_config_dump_cmd();

        static void run_config_test_cmd();

        static void run_config_version_list_cmd();

        static void run_config_get_cmd();

        static void run_config_remove_cmd();

        static void run_config_watch_cmd();

        [[nodiscard]] static turbo::Status
        make_example_config_dump(EA::servlet::ConfigInfo *req);

        [[nodiscard]] static turbo::Status
        make_config_list(EA::servlet::QueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_config_list_version(EA::servlet::QueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_config_get(EA::servlet::QueryRequest *req);

        [[nodiscard]] static turbo::Status
        make_config_remove(EA::servlet::MetaManagerRequest *req);


        static turbo::Table show_query_ops_config_list_response(const EA::servlet::QueryResponse &res);

        static turbo::Table show_query_ops_config_list_version_response(const EA::servlet::QueryResponse &res);

        static turbo::Table
        show_query_ops_config_get_response(const EA::servlet::QueryResponse &res, const turbo::Status &save_status);

        static turbo::Status save_config_to_file(const std::string &path, const EA::servlet::QueryResponse &res);

        static turbo::Status save_config_to_file(const std::string &basedir, const EA::client::ConfigCallbackData &data);
    };

}  // namespace EA::cli

#endif // EA_CLI_CONFIG_CMD_H_
