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
//
// Created by jeff on 23-11-29.
//

#ifndef EA_CLI_ATOMIC_CMD_H_
#define EA_CLI_ATOMIC_CMD_H_

#include "turbo/flags/flags.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include "turbo/format/table.h"
#include "turbo/base/status.h"
#include <string>

namespace EA::cli {

    struct AtomicOptionContext {
        static AtomicOptionContext *get_instance() {
            static AtomicOptionContext ins;
            return &ins;
        }

        // for config
        int64_t servlet_id{0};
        int64_t start_id{0};
        int64_t count{0};
        int64_t increment{0};
        int64_t force{false};
    };

    struct AtomicCmd {
        static void setup_atomic_cmd(turbo::App &app);

        static void run_atomic_cmd(turbo::App *app);

        static void run_atomic_create_cmd();

        static void run_atomic_remove_cmd();

        static void run_atomic_gen_cmd();

        static void run_atomic_update_cmd();
    };

}  // namespace EA::cli

#endif  // EA_CLI_ATOMIC_CMD_H_
