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
#ifndef EA_CLI_USER_CMD_H_
#define EA_CLI_USER_CMD_H_

#include "turbo/flags/flags.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include "turbo/format/table.h"
#include "turbo/base/status.h"
#include <string>

namespace EA::cli {

    struct UserOptionContext {
        static UserOptionContext *get_instance() {
            static UserOptionContext ins;
            return &ins;
        }
        // for namespace
        std::string namespace_name;
        std::string user_name;
        std::string user_passwd;
        std::vector<std::string> user_ips;
        std::vector<std::string> user_rs;
        std::vector<std::string> user_ws;
        std::vector<std::string> user_rz;
        std::vector<std::string> user_wz;
        bool force;
        bool show_pwd{false};
    };

    // We could manually make a few variables and use shared pointers for each; this
    // is just done this way to be nicely organized

    // Function declarations.
    void setup_user_cmd(turbo::App &app);

    void run_user_cmd(turbo::App *app);

    void run_user_create_cmd();

    void run_user_remove_cmd();

    void run_user_add_privilege_cmd();

    void run_user_remove_privilege_cmd();

    void run_user_list_cmd();

    void run_user_flat_cmd();

    void run_user_info_cmd();

    [[nodiscard]] turbo::Status
    make_user_create(EA::servlet::MetaManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_user_remove(EA::servlet::MetaManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_user_add_privilege(EA::servlet::MetaManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_user_remove_privilege(EA::servlet::MetaManagerRequest *req);

    [[nodiscard]] turbo::Status
    make_user_list(EA::servlet::QueryRequest *req);

    [[nodiscard]] turbo::Status
    make_user_flat(EA::servlet::QueryRequest *req);

    [[nodiscard]] turbo::Status
    make_user_info(EA::servlet::QueryRequest *req);

    turbo::Table show_meta_query_user_response(const EA::servlet::QueryResponse &res);
    turbo::Table show_meta_query_user_flat_response(const EA::servlet::QueryResponse &res);

}  // namespace EA::cli

#endif  // EA_CLI_USER_CMD_H_
