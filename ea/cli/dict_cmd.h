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


#ifndef EA_CLI_DICT_CMD_H_
#define EA_CLI_DICT_CMD_H_

#include "turbo/flags/flags.h"
#include "eaproto/router/router.interface.pb.h"
#include "turbo/base/status.h"
#include "turbo/format/table.h"
#include <string>

namespace EA::client {

    struct DictOptionContext {
        static DictOptionContext *get_instance() {
            static DictOptionContext ins;
            return &ins;
        }

        std::string dict_name;
        std::string dict_file;
        std::string dict_version;
        std::string dict_ext;
        int64_t     dict_block_size{4096};
        bool        dict_query_tombstone{false};
    };


    void setup_dict_cmd(turbo::App &app);

    void run_dict_cmd(turbo::App *app);

    void run_dict_create_cmd();

    void run_dict_upload_cmd();

    void run_dict_download_cmd();

    void run_dict_remove_cmd();

    void run_dict_restore_cmd();

    void run_dict_info_cmd();

    void run_dict_list_cmd();

    void run_dict_version_list_cmd();

    [[nodiscard]] turbo::Status
    make_dict_create(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_dict_upload(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_dict_remove(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_dict_restore(EA::proto::OpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_dict_info(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_dict_list(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_dict_download(EA::proto::QueryOpsServiceRequest *req);

    [[nodiscard]] turbo::Status
    make_list_dict_version(EA::proto::QueryOpsServiceRequest *req);

    std::string make_dict_filename(const std::string &name, const EA::proto::Version &version,
                                     const std::string &ext);

    turbo::Table show_query_ops_dict_list_response(const EA::proto::QueryOpsServiceResponse &res);
    turbo::Table show_query_ops_dict_list_version_response(const EA::proto::QueryOpsServiceResponse &res);
    turbo::Table show_query_ops_dict_info_response(const EA::proto::QueryOpsServiceResponse &res);
}  // namespace EA::client

#endif  // EA_CLI_DICT_CMD_H_
