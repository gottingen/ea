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
#include "ea/cli/atomic_cmd.h"
#include "ea/cli/option_context.h"
#include "turbo/format/print.h"
#include "ea/cli/show_help.h"
#include "ea/cli/router_interact.h"
#include "turbo/module/module_version.h"
#include "turbo/files/filesystem.h"
#include "turbo/files/sequential_read_file.h"
#include "turbo/times/clock.h"
#include "json2pb/pb_to_json.h"
#include "json2pb/json_to_pb.h"
#include "ea/client/meta.h"
#include "ea/client/config_info_builder.h"
#include "nlohmann/json.hpp"


namespace EA::cli {

    void AtomicCmd::setup_atomic_cmd(turbo::App &app) {
        auto opt = AtomicOptionContext::get_instance();
        auto *atomic = app.add_subcommand("atomic", "atomic operations");
        atomic->callback([atomic]() { run_atomic_cmd(atomic); });

        auto ac = atomic->add_subcommand("create", " create atomic");
        ac->add_option("-a,--app", opt->servlet_id, "servlet id")->required(true);
        ac->add_option("-i, --id", opt->start_id, "start id");
        ac->callback([]() { run_atomic_create_cmd(); });

        auto ar = atomic->add_subcommand("remove", " remove atomic");
        ar->add_option("-a,--servlet", opt->servlet_id, "servlet id")->required(true);
        ar->callback([]() { run_atomic_remove_cmd(); });

        auto ag = atomic->add_subcommand("gen", " gen atomic");
        ag->add_option("-a,--app", opt->servlet_id, "servlet id")->required(true);
        auto agt = ag->add_option_group("grow type", "start id or increment");
        agt->add_option("-i, --id", opt->start_id, "start id");
        agt->add_option("-c, --count", opt->count, "count id");
        agt->require_option(1);
        agt->required(true);
        ag->callback([]() { run_atomic_gen_cmd(); });

        auto au = atomic->add_subcommand("update", " update atomic");
        au->add_option("-a,--app", opt->servlet_id, "servlet id")->required(true);
        auto gt = au->add_option_group("grow type", "start id or increment");
        gt->add_option("-i, --id", opt->start_id, "start id");
        gt->add_option("-c, --count", opt->increment, "count id");
        gt->require_option(1);
        au->add_flag("-f, --force", opt->force, "force")->default_val(false);
        au->callback([]() { run_atomic_update_cmd(); });
    }

    void AtomicCmd::run_atomic_cmd(turbo::App *app) {
        if (app->get_subcommands().empty()) {
            turbo::Println("{}", app->help());
        }
    }

    void AtomicCmd::run_atomic_create_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        request.set_op_type(EA::servlet::OP_ADD_ID_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        atomic_info->set_start_id(opt->start_id);
        auto rs = EA::client::MetaClient::get_instance()->meta_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::servlet::SUCCESS);
        if (response.errcode() == EA::servlet::SUCCESS) {
            turbo::Table summary;
            summary.add_row({"servlet id", "start id", "end id"});
            summary.add_row({turbo::Format(opt->servlet_id), turbo::Format(response.start_id()),
                             turbo::Format(response.end_id())});
            ss.add_table("summary", std::move(summary), true);
        }
    }

    void AtomicCmd::run_atomic_remove_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        request.set_op_type(EA::servlet::OP_DROP_ID_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        auto rs = EA::client::MetaClient::get_instance()->meta_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::servlet::SUCCESS);
    };

    void AtomicCmd::run_atomic_gen_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        request.set_op_type(EA::servlet::OP_GEN_ID_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        if (opt->start_id != 0) {
            atomic_info->set_start_id(opt->start_id);
        }
        turbo::Println("{}", opt->count);
        atomic_info->set_count(opt->count);
        auto rs = EA::client::MetaClient::get_instance()->meta_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::servlet::SUCCESS);
        if (response.errcode() == EA::servlet::SUCCESS) {
            turbo::Table summary;
            summary.add_row({"servlet id", "start id", "end id"});
            summary.add_row({turbo::Format(opt->servlet_id), turbo::Format(response.start_id()),
                             turbo::Format(response.end_id())});
            ss.add_table("summary", std::move(summary), true);
        }
    }

    void AtomicCmd::run_atomic_update_cmd() {
        EA::servlet::MetaManagerRequest request;
        EA::servlet::MetaManagerResponse response;
        ScopeShower ss;
        request.set_op_type(EA::servlet::OP_UPDATE_FOR_AUTO_INCREMENT);
        auto opt = AtomicOptionContext::get_instance();
        auto atomic_info = request.mutable_auto_increment();
        atomic_info->set_servlet_id(opt->servlet_id);
        if (opt->start_id != 0) {
            atomic_info->set_start_id(opt->start_id);
        }
        if (opt->increment != 0) {
            atomic_info->set_increment_id(opt->increment);
        }
        if (opt->force) {
            atomic_info->set_force(opt->force);
        }
        auto rs = EA::client::MetaClient::get_instance()->meta_manager(request, response, nullptr);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), response.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::servlet::SUCCESS);
        if (response.errcode() == EA::servlet::SUCCESS) {
            turbo::Table summary;
            summary.add_row({"servlet id", "start id"});
            summary.add_row({turbo::Format(opt->servlet_id), turbo::Format(response.start_id())});
            ss.add_table("summary", std::move(summary), true);
        }
    }

}  // namespace EA::cli
