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
#include "eaproto/router/router.interface.pb.h"
#include "turbo/base/status.h"
#include "turbo/format/table.h"
#include <string>


namespace EA::client {

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
    };

    void setup_config_cmd(turbo::App &app);

    void run_config_cmd(turbo::App *app);

    void run_config_create_cmd();

    void run_config_list_cmd();

    void run_config_version_list_cmd();

    void run_config_get_cmd();

    void run_config_remove_cmd();

    [[nodiscard]] turbo::Status
    make_config_create(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_config_list(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_config_list_version(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_config_get(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_config_remove(EA::proto::OpsServiceRequest *req);


    turbo::Table show_query_ops_config_list_response(const EA::proto::QueryOpsServiceResponse &res);

    turbo::Table show_query_ops_config_list_version_response(const EA::proto::QueryOpsServiceResponse &res);

    turbo::Table show_query_ops_config_get_response(const EA::proto::QueryOpsServiceResponse &res, const turbo::Status &save_status);

    turbo::Status save_config_to_file(const std::string &path, const EA::proto::QueryOpsServiceResponse &res);

}  // namespace EA::client

#endif // EA_CLI_CONFIG_CMD_H_
