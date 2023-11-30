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
#include "ea/cli/meta_cmd.h"
#include "turbo/flags/flags.h"
#include "turbo/format/print.h"
#include "ea/cli/option_context.h"
#include "ea/client/meta.h"
#include "ea/client/base_message_sender.h"
#include "ea/client/router_sender.h"
#include "ea/client/meta_sender.h"
#include "ea/cli/raft_cmd.h"
#include "ea/cli/discovery.h"

int main(int argc, char **argv) {
    turbo::App app{"elastic ann search client"};
    auto opt = EA::cli::OptionContext::get_instance();
    app.add_flag("-V, --verbose", opt->verbose, "verbose detail message default(false)")->default_val(false);
    app.add_option("-s,--server", opt->router_server, "server address default(\"127.0.0.1:8010\")")->default_val(
            "127.0.0.1:8010");
    app.add_option("-m,--meta_server", opt->meta_server, "server address default(\"127.0.0.1:8010\")")->default_val(
            "127.0.0.1:8010");
    app.add_flag("-r,--router", opt->router, "server address default(false)")->default_val(false);
    app.add_option("-T,--timeout", opt->timeout_ms, "timeout ms default(2000)");
    app.add_option("-C,--connect", opt->connect_timeout_ms, "connect timeout ms default(100)");
    app.add_option("-R,--retry", opt->max_retry, "max try time default(3)");
    app.add_option("-I,--interval", opt->time_between_meta_connect_error_ms,
                   "time between meta connect error ms default(1000)");
    app.callback([&app] {
        if (app.get_subcommands().empty()) {
            turbo::Println("{}", app.help());
        }
    });

    auto func = []() {
        turbo::Println(turbo::color::red, "eacli parse call back");
        auto opt = EA::cli::OptionContext::get_instance();
        if (opt->verbose) {
            turbo::Println("cli verbose all operations");
        }
        EA::client::BaseMessageSender *sender{nullptr};
        if (opt->router) {
            auto rs = EA::client::RouterSender::get_instance()->init(opt->router_server);
            if (!rs.ok()) {
                turbo::Println(rs.message());
                exit(0);
            }
            EA::client::RouterSender::get_instance()->set_connect_time_out(opt->connect_timeout_ms)
                    .set_interval_time(opt->time_between_meta_connect_error_ms)
                    .set_retry_time(opt->max_retry)
                    .set_verbose(opt->verbose);
            sender = EA::client::RouterSender::get_instance();
            TLOG_INFO_IF(opt->verbose, "init connect success to router server {}", opt->router_server);
        } else {
            EA::client::MetaSender::get_instance()->set_connect_time_out(opt->connect_timeout_ms)
                    .set_interval_time(opt->time_between_meta_connect_error_ms)
                    .set_retry_time(opt->max_retry)
                    .set_verbose(opt->verbose);
            auto rs = EA::client::MetaSender::get_instance()->init(opt->meta_server);
            if (!rs.ok()) {
                turbo::Println("{}", rs.message());
                exit(0);
            }
            sender = EA::client::MetaSender::get_instance();
            TLOG_INFO_IF(opt->verbose, "init connect success to meta server:{}", opt->meta_server);
        }
        auto r = EA::client::MetaClient::get_instance()->init(sender);
        if (!r.ok()) {
            turbo::Println("set up meta server error:{}", r.message());
            exit(0);
        }
    };
    app.parse_complete_callback(func);

    // Call the setup functions for the subcommands.
    // They are kept alive by a shared pointer in the
    // lambda function
    EA::cli::setup_meta_cmd(app);
    EA::cli::RaftCmd::setup_raft_cmd(app);
    EA::cli::DiscoveryCmd::setup_discovery_cmd(app);
    // More setup if needed, i.e., other subcommands etc.

    TURBO_FLAGS_PARSE(app, argc, argv);

    return 0;
}
