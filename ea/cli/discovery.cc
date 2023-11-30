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
// Created by jeff on 23-11-30.
//
#include "ea/cli/discovery.h"
#include "ea/cli/option_context.h"
#include "turbo/format/print.h"
#include "ea/cli/show_help.h"
#include "ea/cli/router_interact.h"
#include "turbo/module/module_version.h"
#include "turbo/files/filesystem.h"
#include "turbo/files/sequential_read_file.h"
#include "turbo/times/clock.h"
#include "ea/client/meta.h"
#include "nlohmann/json.hpp"
#include "ea/cli/validator.h"
#include "ea/client/meta_sender.h"
#include "ea/client/router_sender.h"
#include "ea/client/dumper.h"
#include "ea/client/loader.h"
#include "ea/client/servlet_instance_builder.h"

namespace EA::cli {
    void DiscoveryCmd::setup_discovery_cmd(turbo::App &app) {
        // Create the option and subcommand objects.
        auto opt = DiscoveryOptionContext::get_instance();
        auto *discovery_cmd = app.add_subcommand("discovery", "discovery operations");
        discovery_cmd->callback([discovery_cmd]() { run_discovery_cmd(discovery_cmd); });

        auto dai = discovery_cmd->add_subcommand("add_instance", " create a instance");
        auto *add_parameters_inputs = dai->add_option_group("parameters_inputs", "config input from parameters");
        auto *add_json_inputs = dai->add_option_group("json_inputs", "config input source from json format");
        add_parameters_inputs->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        add_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        add_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        add_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        add_parameters_inputs->add_option("-e, --env", opt->env, "instance env")->required();
        add_parameters_inputs->add_option("-c, --color", opt->color, "instance color")->default_val("default");
        add_parameters_inputs->add_option("-t, --status", opt->status, "instance color")->default_val("NORMAL");
        add_parameters_inputs->add_option("-w, --weight", opt->weight, "instance weight");
        add_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dai->require_option(1);
        dai->callback([]() { run_discovery_add_instance_cmd(); });

        auto dri = discovery_cmd->add_subcommand("remove_instance", " remove a instance");
        auto *remove_parameters_inputs = dri->add_option_group("parameters_inputs", "config input from parameters");
        auto *remove_json_inputs = dri->add_option_group("json_inputs", "config input source from json format");
        remove_parameters_inputs->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        remove_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        remove_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        remove_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        remove_parameters_inputs->add_option("-e, --env", opt->env, "instance env");
        remove_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dri->require_option(1);
        dri->callback([]() { run_discovery_remove_instance_cmd(); });

        auto dui = discovery_cmd->add_subcommand("update_instance", " create a instance");
        auto *update_parameters_inputs = dui->add_option_group("parameters_inputs", "config input from parameters");
        auto *update_json_inputs = dui->add_option_group("json_inputs", "config input source from json format");
        update_parameters_inputs->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        update_parameters_inputs->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        update_parameters_inputs->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        update_parameters_inputs->add_option("-a, --address", opt->address, "instance address")->required();
        update_parameters_inputs->add_option("-e, --env", opt->env, "instance env")->required();
        update_parameters_inputs->add_option("-c, --color", opt->color, "instance color")->default_val("default");
        update_parameters_inputs->add_option("-t, --status", opt->status, "instance color")->default_val("NORMAL");
        update_parameters_inputs->add_option("-w, --weight", opt->weight, "instance weight");
        update_json_inputs->add_option("-j, --json", opt->json_file, "json input file")->required(true);
        dui->require_option(1);
        dui->callback([]() { run_discovery_update_instance_cmd(); });

        auto dl = discovery_cmd->add_subcommand("list", "list instance");
        dl->add_option("-n,--namespace", opt->namespace_name, "namespace name");
        dl->add_option("-z,--zone", opt->zone_name, "zone name");
        dl->add_option("-s,--servlet", opt->servlet_name, "servlet name");
        dl->add_option("-a, --address", opt->address, "instance address");
        dl->callback([]() { run_discovery_list_instance_cmd(); });

        auto di = discovery_cmd->add_subcommand("info", "info instance");
        di->add_option("-n,--namespace", opt->namespace_name, "namespace name")->required();
        di->add_option("-z,--zone", opt->zone_name, "zone name")->required();
        di->add_option("-s,--servlet", opt->servlet_name, "servlet name")->required();
        di->add_option("-a, --address", opt->address, "instance address")->required();
        di->callback([]() { run_discovery_info_instance_cmd(); });


        auto dd = discovery_cmd->add_subcommand("dump", " dump instance example to json file");
        dd->add_option("-o,--output", opt->dump_file, "dump file path")->default_val("example_discovery.json");
        dd->add_flag("-q,--quiet", opt->quiet, "quiet or print")->default_val(false);
        dd->callback([]() { run_discovery_dump_cmd(); });
        /// run after cmd line parse complete
    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void DiscoveryCmd::run_discovery_cmd(turbo::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            turbo::Println("{}", app->help());
        }
    }

    void DiscoveryCmd::run_discovery_add_instance_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        auto rs = make_discovery_add_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("meta_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void DiscoveryCmd::run_discovery_remove_instance_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        auto rs = make_discovery_remove_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("meta_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void DiscoveryCmd::run_discovery_update_instance_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        auto rs = make_discovery_update_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = RouterInteract::get_instance()->send_request("meta_manager", request, response);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(OptionContext::get_instance()->router_server, response.errcode(),
                                               request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table));
    }

    void DiscoveryCmd::run_discovery_list_instance_cmd() {
        EA::servlet::QueryRequest request;
        EA::servlet::QueryResponse response;

        ScopeShower ss;
        auto rs = make_discovery_list_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = EA::client::MetaClient::get_instance()->meta_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::servlet::SUCCESS);
        if (response.errcode() == EA::servlet::SUCCESS) {
            table = show_query_instance_list_response(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void DiscoveryCmd::run_discovery_info_instance_cmd() {
        EA::servlet::QueryRequest request;
        EA::servlet::QueryResponse response;

        ScopeShower ss;
        auto rs = make_discovery_info_instance(&request);
        PREPARE_ERROR_RETURN_OR_OK(ss, rs, request);
        rs = EA::client::MetaClient::get_instance()->meta_query(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::servlet::SUCCESS);
        if (response.errcode() == EA::servlet::SUCCESS) {
            table = show_query_instance_info_response(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    [[nodiscard]] turbo::Status
    DiscoveryCmd::make_discovery_add_instance(EA::servlet::MetaManagerRequest *req) {
        EA::servlet::ServletInstance *instance_req = req->mutable_instance_info();
        req->set_op_type(EA::servlet::OP_ADD_INSTANCE);
        if (!DiscoveryOptionContext::get_instance()->json_file.empty()) {
            auto rs = EA::client::Loader::load_proto_from_file(DiscoveryOptionContext::get_instance()->json_file,
                                                               *instance_req);
            return rs;
        }
        auto rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        auto status = string_to_status(DiscoveryOptionContext::get_instance()->status);
        if (!status.ok()) {
            return status.status();
        }
        instance_req->set_namespace_name(DiscoveryOptionContext::get_instance()->namespace_name);
        instance_req->set_zone_name(DiscoveryOptionContext::get_instance()->zone_name);
        instance_req->set_servlet_name(DiscoveryOptionContext::get_instance()->servlet_name);
        instance_req->set_color(DiscoveryOptionContext::get_instance()->color);
        instance_req->set_env(DiscoveryOptionContext::get_instance()->env);
        instance_req->set_status(status.value());
        instance_req->set_address(DiscoveryOptionContext::get_instance()->address);
        if (DiscoveryOptionContext::get_instance()->weight != -1) {
            instance_req->set_weight(DiscoveryOptionContext::get_instance()->weight);
        }
        instance_req->set_timestamp(static_cast<int>(::time(nullptr)));
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    DiscoveryCmd::make_discovery_remove_instance(EA::servlet::MetaManagerRequest *req) {
        EA::servlet::ServletInstance *instance_req = req->mutable_instance_info();
        req->set_op_type(EA::servlet::OP_DROP_INSTANCE);
        if (!DiscoveryOptionContext::get_instance()->json_file.empty()) {
            auto rs = EA::client::Loader::load_proto_from_file(DiscoveryOptionContext::get_instance()->json_file,
                                                               *instance_req);
            return rs;
        }
        auto rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }

        instance_req->set_namespace_name(DiscoveryOptionContext::get_instance()->namespace_name);
        instance_req->set_zone_name(DiscoveryOptionContext::get_instance()->zone_name);
        instance_req->set_servlet_name(DiscoveryOptionContext::get_instance()->servlet_name);
        instance_req->set_address(DiscoveryOptionContext::get_instance()->address);

        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    DiscoveryCmd::make_discovery_update_instance(EA::servlet::MetaManagerRequest *req) {
        EA::servlet::ServletInstance *instance_req = req->mutable_instance_info();
        req->set_op_type(EA::servlet::OP_UPDATE_INSTANCE);
        if (!DiscoveryOptionContext::get_instance()->json_file.empty()) {
            auto rs = EA::client::Loader::load_proto_from_file(DiscoveryOptionContext::get_instance()->json_file,
                                                               *instance_req);
            return rs;
        }
        auto rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->namespace_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->zone_name);
        if (!rs.ok()) {
            return rs;
        }
        rs = CheckValidNameType(DiscoveryOptionContext::get_instance()->servlet_name);
        if (!rs.ok()) {
            return rs;
        }
        auto status = string_to_status(DiscoveryOptionContext::get_instance()->status);
        if (!status.ok()) {
            return status.status();
        }
        instance_req->set_namespace_name(DiscoveryOptionContext::get_instance()->namespace_name);
        instance_req->set_zone_name(DiscoveryOptionContext::get_instance()->zone_name);
        instance_req->set_servlet_name(DiscoveryOptionContext::get_instance()->servlet_name);
        instance_req->set_color(DiscoveryOptionContext::get_instance()->color);
        instance_req->set_env(DiscoveryOptionContext::get_instance()->env);
        instance_req->set_status(status.value());
        instance_req->set_address(DiscoveryOptionContext::get_instance()->address);
        if (DiscoveryOptionContext::get_instance()->weight != -1) {
            instance_req->set_weight(DiscoveryOptionContext::get_instance()->weight);
        }
        instance_req->set_timestamp(static_cast<int>(::time(nullptr)));
        return turbo::OkStatus();
    }

    void DiscoveryCmd::run_discovery_dump_cmd() {
        EA::servlet::ServletInstance instance;
        EA::client::ServletInstanceBuilder builder(&instance);
        builder.set_namespace("ex_namespace")
                .set_zone("ex_zone")
                .set_servlet("ex_servlet")
                .set_env("ex_env")
                .set_color("green")
                .set_status("NORMAL")
                .set_weight(10)
                .set_time(time(NULL))
                .set_address("127.0.0.1:12345");
        auto rs = EA::client::Dumper::dump_proto_to_file(DiscoveryOptionContext::get_instance()->dump_file, instance);
        if (!rs.ok()) {
            turbo::Println("dump example discovery instance error:{}", rs.message());
            return;
        }

        if (!DiscoveryOptionContext::get_instance()->quiet) {
            std::string json_str;
            rs = EA::client::Dumper::dump_proto(instance, json_str);
            if (!rs.ok()) {
                turbo::Println("dump example discovery instance error:{}", rs.message());
                return;
            }
            std::cout << ShowHelper::json_format(json_str) << std::endl;
        }

    }

    [[nodiscard]] turbo::Status
    DiscoveryCmd::make_discovery_list_instance(EA::servlet::QueryRequest *req) {
        req->set_op_type(EA::servlet::QUERY_INSTANCE_FLATTEN);
        auto opt = DiscoveryOptionContext::get_instance();
        if (opt->namespace_name.empty()) {
            return turbo::OkStatus();
        }
        req->set_namespace_name(opt->namespace_name);
        if (opt->zone_name.empty()) {
            return turbo::OkStatus();
        }
        req->set_zone(opt->zone_name);
        if (opt->servlet_name.empty()) {
            return turbo::OkStatus();
        }
        req->set_servlet(opt->servlet_name);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    DiscoveryCmd::make_discovery_info_instance(EA::servlet::QueryRequest *req) {
        req->set_op_type(EA::servlet::QUERY_INSTANCE);
        auto opt = DiscoveryOptionContext::get_instance();
        req->set_namespace_name(opt->namespace_name);
        req->set_zone(opt->zone_name);
        req->set_servlet(opt->servlet_name);
        req->set_instance_address(opt->address);
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::ResultStatus<EA::servlet::Status> DiscoveryCmd::string_to_status(const std::string &status) {
        EA::servlet::Status ret;
        if (EA::servlet::Status_Parse(status, &ret)) {
            return ret;
        }
        return turbo::InvalidArgumentError("unknown status");
    }

    turbo::Table DiscoveryCmd::show_query_instance_list_response(const EA::servlet::QueryResponse &res) {
        turbo::Table result;
        auto &instance_list = res.flatten_instances();
        result.add_row(turbo::Table::Row_t{"instance num", turbo::Format(instance_list.size())});
        auto last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        result.add_row(turbo::Table::Row_t{"number", "instance"});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        int i = 0;
        std::vector<EA::servlet::QueryInstance> sorted_list;
        for (auto &ns: instance_list) {
            sorted_list.push_back(ns);
        }
        auto less_fun = [](const EA::servlet::QueryInstance &lhs, const EA::servlet::QueryInstance &rhs) -> bool {
            return std::less()(lhs.address(), rhs.address());
        };
        std::sort(sorted_list.begin(), sorted_list.end(), less_fun);
        for (auto &ns: sorted_list) {
            result.add_row(turbo::Table::Row_t{turbo::Format(i++),
                                               turbo::Format("{}.{}.{}#{}", ns.namespace_name(), ns.zone_name(),
                                                             ns.servlet_name(), ns.address())});
            last = result.size() - 1;
            result[last].format().font_color(turbo::Color::yellow);

        }
        return result;
    }

    turbo::Table DiscoveryCmd::show_query_instance_info_response(const EA::servlet::QueryResponse &res) {
        turbo::Table result;
        auto &instance = res.instance(0);
        result.add_row(turbo::Table::Row_t{"uri", "address", "env", "color","create time", "version","status"});
        result.add_row(
                turbo::Table::Row_t{
                        turbo::Format("{}.{}.{}", instance.namespace_name(), instance.zone_name(),
                                      instance.servlet_name()),
                        instance.address(),
                        instance.env(),
                        instance.color(),
                        turbo::Format(instance.timestamp()),
                        turbo::Format(instance.version()),
                        EA::servlet::Status_Name(instance.status())
                }
        );

        return result;
    }

}  // namespace EA::cli
