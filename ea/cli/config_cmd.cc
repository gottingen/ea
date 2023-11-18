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
#include "ea/cli/config_cmd.h"
#include "ea/cli/option_context.h"
#include "eaproto/router/router.interface.pb.h"
#include "turbo/format/print.h"
#include "ea/cli/show_help.h"
#include "ea/rpc/router_interact.h"
#include "turbo/module/module_version.h"
#include "turbo/files/filesystem.h"
#include "turbo/files/sequential_read_file.h"
#include "turbo/times/clock.h"
#include "ea/rpc/proto_help.h"

namespace EA::client {
    void setup_config_cmd(turbo::App &app) {
        // Create the option and subcommand objects.
        auto opt = ConfigOptionContext::get_instance();
        auto *ns = app.add_subcommand("config", "config operations");
        ns->callback([ns]() { run_config_cmd(ns); });

        auto cc = ns->add_subcommand("create", " create config");
        cc->add_option("-n,--name", opt->config_name, "config name")->required();
        auto *inputs = cc->add_option_group("inputs", "config input source");
        inputs->add_option("-d,--data", opt->config_data, "config content");
        inputs->add_option("-f, --file", opt->config_file, "local config file");
        inputs->require_option(1);
        cc->add_option("-v, --version", opt->config_version, "config version [1.2.3]")->required();
        cc->add_option("-t, --type", opt->config_type, "config type [json|toml|yaml|xml|gflags|text|ini]")->default_val("json");

        cc->callback([]() { run_config_create_cmd(); });
        auto cl = ns->add_subcommand("list", " list config");
        cl->add_option("-n,--name", opt->config_name, "config name");
        cl->callback([]() { run_config_list_cmd(); });

        auto cg = ns->add_subcommand("get", " get config");
        cg->add_option("-n,--name", opt->config_name, "config name")->required();
        cg->add_option("-v, --version", opt->config_version, "config version");
        cg->add_option("-o, --output", opt->config_file, "config save file");
        cg->callback([]() { run_config_get_cmd(); });

        auto cr = ns->add_subcommand("remove", " remove config");
        cr->add_option("-n,--name", opt->config_name, "config name")->required();
        cr->add_option("-v, --version", opt->config_version, "config version [1.2.3]");
        cr->callback([]() { run_config_remove_cmd(); });
    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_config_cmd(turbo::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            turbo::Println("{}", app->help());
        }
    }

    void run_config_create_cmd() {
        EA::proto::OpsServiceRequest request;
        EA::proto::OpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_config_create(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("config_manage", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(), response.op_type(),
                         response.errmsg());
        ss.add_table(std::move(table));
    }

    void run_config_list_cmd() {
        if (!ConfigOptionContext::get_instance()->config_name.empty()) {
            run_config_version_list_cmd();
            return;
        }
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_config_list(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("config_query", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(), response.op_type(),
                         response.errmsg());
        ss.add_table(std::move(table));
        if(response.errcode() == EA::proto::SUCCESS) {
            table = show_query_ops_config_list_response(response);
            ss.add_table(std::move(table));
        }
    }

    void run_config_version_list_cmd() {
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_config_list_version(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("config_query", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(), response.op_type(),
                         response.errmsg());
        ss.add_table(std::move(table));
        if(response.errcode() == EA::proto::SUCCESS) {
            table = show_query_ops_config_list_version_response(response);
            ss.add_table(std::move(table));
        }
    }

    void run_config_get_cmd() {
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_config_get(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("config_query", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(), response.op_type(),
                         response.errmsg());
        ss.add_table(std::move(table));
        if(response.errcode() != EA::proto::SUCCESS) {
            return;
        }
        turbo::Status save_status;
        if(!ConfigOptionContext::get_instance()->config_file.empty()) {
            save_status = save_config_to_file(ConfigOptionContext::get_instance()->config_file, response);
        }
        table = show_query_ops_config_get_response(response,save_status);
        ss.add_table(std::move(table));
    }

    turbo::Status save_config_to_file(const std::string & path, const EA::proto::QueryOpsServiceResponse &res) {
        turbo::SequentialWriteFile file;
        auto s = file.open(ConfigOptionContext::get_instance()->config_file, true);
        if(!s.ok()) {
            return s;
        }
        s= file.write(res.config_response().config().content());
        if(!s.ok()) {
            return s;
        }
        file.close();
        return turbo::OkStatus();
    }

    void run_config_remove_cmd() {
        EA::proto::OpsServiceRequest request;
        EA::proto::OpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_config_remove(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("config_manage", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(), response.op_type(),
                         response.errmsg());
       ss.add_table(std::move(table));
    }

    [[nodiscard]] turbo::Status
    make_config_create(EA::proto::OpsServiceRequest *req) {
        req->set_op_type(EA::proto::OP_CREATE_CONFIG);
        auto rc = req->mutable_request_config();
        auto opt = ConfigOptionContext::get_instance();
        rc->set_name(opt->config_name);
        rc->set_time(static_cast<int>(turbo::ToTimeT(turbo::Now())));
        auto r = EA::rpc::string_to_config_type(opt->config_type);
        if (!r.ok()) {
            return r.status();
        }
        rc->set_type(r.value());
        auto v = rc->mutable_version();
        auto st = EA::rpc::string_to_version(opt->config_version, v);
        if (!st.ok()) {
            return st;
        }
        if (!opt->config_data.empty()) {
            rc->set_content(opt->config_data);
            return turbo::OkStatus();
        }
        if (opt->config_file.empty()) {
            return turbo::InvalidArgumentError("no config content");
        }
        turbo::SequentialReadFile file;
        auto rs = file.open(opt->config_file);
        if (!rs.ok()) {
            return rs;
        }
        auto rr = file.read(&opt->config_data);
        if (!rr.ok()) {
            return rr.status();
        }
        rc->set_content(opt->config_data);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_config_list(EA::proto::QueryOpsServiceRequest *req) {
        req->set_op_type(EA::proto::QUERY_LIST_CONFIG);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_config_list_version(EA::proto::QueryOpsServiceRequest *req) {
        req->set_op_type(EA::proto::QUERY_LIST_CONFIG_VERSION);
        auto opt = ConfigOptionContext::get_instance();
        req->mutable_query_config()->set_name(opt->config_name);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_config_get(EA::proto::QueryOpsServiceRequest *req) {
        req->set_op_type(EA::proto::QUERY_GET_CONFIG);
        auto rc = req->mutable_query_config();
        auto opt = ConfigOptionContext::get_instance();
        rc->set_name(opt->config_name);
        if (!opt->config_version.empty()) {
            auto v = rc->mutable_version();
            return EA::rpc::string_to_version(opt->config_version, v);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_config_remove(EA::proto::OpsServiceRequest *req) {
        req->set_op_type(EA::proto::OP_REMOVE_CONFIG);
        auto rc = req->mutable_request_config();
        auto opt = ConfigOptionContext::get_instance();
        rc->set_name(opt->config_name);
        if (!opt->config_version.empty()) {
            auto v = rc->mutable_version();
            return EA::rpc::string_to_version(opt->config_version, v);
        }
        return turbo::OkStatus();
    }

    turbo::Table show_query_ops_config_list_response(const EA::proto::QueryOpsServiceResponse &res) {
        turbo::Table result;
        auto &config_list = res.config_response().config_list();
        result.add_row(turbo::Table::Row_t{"config size", turbo::Format(config_list.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        result.add_row(turbo::Table::Row_t{"number", "config"});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        int i = 0;
        for (auto &ns: config_list) {
            result.add_row(turbo::Table::Row_t{turbo::Format(i++), ns});
            last = result.size() - 1;
            result[last].format().font_color(turbo::Color::yellow);

        }
        return result;
    }

    turbo::Table show_query_ops_config_list_version_response(const EA::proto::QueryOpsServiceResponse &res) {
        turbo::Table result;
        auto &config_versions = res.config_response().versions();
        result.add_row(turbo::Table::Row_t{"version size", turbo::Format(config_versions.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        result.add_row(turbo::Table::Row_t{"number", "version"});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        int i = 0;
        for (auto &ns: config_versions) {
            result.add_row(
                    turbo::Table::Row_t{turbo::Format(i++), turbo::Format("{}.{}.{}", ns.major(), ns.minor(), ns.patch())});
            last = result.size() - 1;
            result[last].format().font_color(turbo::Color::yellow);

        }
        return result;
    }

    turbo::Table show_query_ops_config_get_response(const EA::proto::QueryOpsServiceResponse &res, const turbo::Status &save_status) {
        turbo::Table result_table;

        result_table.add_row(turbo::Table::Row_t{"version", turbo::Format("{}.{}.{}", res.config_response().config().version().major(),
                                                            res.config_response().config().version().minor(),
                                                            res.config_response().config().version().patch())});
        auto last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        result_table.add_row(turbo::Table::Row_t{"type", EA::rpc::config_type_to_string(res.config_response().config().type())});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        result_table.add_row(turbo::Table::Row_t{"size", turbo::Format(res.config_response().config().content().size())});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        turbo::Time cs = turbo::FromTimeT(res.config_response().config().time());
        result_table.add_row(turbo::Table::Row_t{"time", turbo::FormatTime(cs)});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        if(!ConfigOptionContext::get_instance()->config_file.empty()) {
            result_table.add_row(turbo::Table::Row_t{"file", ConfigOptionContext::get_instance()->config_file});
            last = result_table.size() - 1;
            result_table[last].format().font_color(turbo::Color::green);
            result_table.add_row(turbo::Table::Row_t{"status", save_status.ok() ? "ok" : save_status.message()});
            last = result_table.size() - 1;
            result_table[last].format().font_color(turbo::Color::green);
        }

        return result_table;
    }


}  // namespace EA::client
