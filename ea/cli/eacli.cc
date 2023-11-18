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
#include "ea/cli/ops_cmd.h"
#include "turbo/flags/flags.h"
#include "turbo/format/print.h"
#include "ea/cli/option_context.h"

int main(int argc, char **argv) {
    turbo::App app{"elastic ann search client"};
    auto opt = EA::client::OptionContext::get_instance();
    app.add_flag("-V, --verbose", opt->verbose, "verbose detail message default(false)")->default_val(false);
    app.add_option("-s,--server", opt->server, "server address default(\"127.0.0.0:8888\")")->default_val("127.0.0.0:8888");
    app.add_option("-l,--lb", opt->load_balancer, "load balance default(\"rr\")")->default_val("rr");
    app.add_option("-m,--timeout", opt->timeout_ms, "timeout ms default(2000)")->default_val(int32_t(2000));
    app.add_option("-c,--connect", opt->connect_timeout_ms, "connect timeout ms default(100)")->default_val(int32_t(100));
    app.add_option("-r,--retry", opt->max_retry, "max try time default(3)")->default_val(int32_t(3));
    app.add_option("-i,--interval", opt->time_between_meta_connect_error_ms, "time between meta connect error ms default(1000)")->default_val(int32_t(1000));
    app.callback([&app] {
        if (app.get_subcommands().empty()) {
            turbo::Println("{}", app.help());
        }
    });

    // Call the setup functions for the subcommands.
    // They are kept alive by a shared pointer in the
    // lambda function
    EA::client::setup_ops_cmd(app);
    // More setup if needed, i.e., other subcommands etc.

    TURBO_FLAGS_PARSE(app, argc, argv);

    return 0;
}
