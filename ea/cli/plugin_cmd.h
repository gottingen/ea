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


#ifndef EA_CLI_PLUGIN_CMD_H_
#define EA_CLI_PLUGIN_CMD_H_

#include "turbo/flags/flags.h"
#include "eaproto/router/router.interface.pb.h"
#include "turbo/base/status.h"
#include "turbo/format/table.h"
#include <string>

namespace EA::client {

    struct PluginOptionContext {
        static PluginOptionContext *get_instance() {
            static PluginOptionContext ins;
            return &ins;
        }
        // for plugin
        std::string plugin_name;
        std::string plugin_file;
        std::string plugin_version;
        std::string plugin_type;
        int64_t     plugin_block_size{4096};
        bool        plugin_query_tombstone{false};
    };


    void setup_plugin_cmd(turbo::App &app);

    void run_plugin_cmd(turbo::App *app);

    void run_plugin_create_cmd();

    void run_plugin_upload_cmd();

    void run_plugin_download_cmd();

    void run_plugin_remove_cmd();

    void run_plugin_restore_cmd();

    void run_plugin_info_cmd();

    void run_plugin_list_cmd();

    void run_plugin_version_list_cmd();

    [[nodiscard]] turbo::Status
    make_plugin_create(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_plugin_upload(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_plugin_remove(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_plugin_restore(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_plugin_info(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_plugin_list(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_plugin_download(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_list_plugin_version(EA::proto::QueryOpsServiceRequest *req);

    turbo::Table show_query_ops_plugin_list_response(const EA::proto::QueryOpsServiceResponse &res);
    turbo::Table show_query_ops_plugin_list_version_response(const EA::proto::QueryOpsServiceResponse &res);
    turbo::Table show_query_ops_plugin_info_response(const EA::proto::QueryOpsServiceResponse &res);
}  // namespace EA::client

#endif  // EA_CLI_PLUGIN_CMD_H_
