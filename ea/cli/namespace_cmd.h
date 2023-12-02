// Copyright 2023 The Turbo Authors.
//
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
#ifndef EA_CLI_NAMESPACE_CMD_H_
#define EA_CLI_NAMESPACE_CMD_H_

#include "turbo/flags/flags.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include "turbo/format/table.h"
#include "turbo/base/status.h"
#include <string>

namespace EA::cli {

    // We could manually make a few variables and use shared pointers for each; this
    // is just done this way to be nicely organized

    struct NameSpaceOptionContext {
        static NameSpaceOptionContext *get_instance() {
            static NameSpaceOptionContext ins;
            return &ins;
        }
        // for namespace
        std::string namespace_name;
        int64_t     namespace_quota;
    };

    // Function declarations.
    void setup_namespace_cmd(turbo::App &app);

    void run_namespace_cmd(turbo::App *app);

    void run_ns_create_cmd();

    void run_ns_remove_cmd();

    void run_ns_modify_cmd();

    void run_ns_list_cmd();

    void run_ns_info_cmd();

    [[nodiscard]] turbo::Status
    make_namespace_create(EA::discovery::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_namespace_remove(EA::discovery::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_namespace_modify(EA::discovery::DiscoveryManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_namespace_query(EA::discovery::DiscoveryQueryRequest *req);

    turbo::Table show_discovery_query_ns_response(const EA::discovery::DiscoveryQueryResponse &res);
}  // namespace EA::cli

#endif  // EA_CLI_NAMESPACE_CMD_H_
