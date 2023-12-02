// Copyright 2023 The Elastic Architecture Infrastructure Authors.
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

#include "ea/cli/raft_cmd.h"
#include "ea/cli/option_context.h"
#include "turbo/format/print.h"
#include "ea/cli/show_help.h"
#include "turbo/files/sequential_read_file.h"
#include "ea/client/config_info_builder.h"

namespace EA::cli {

    void RaftCmd::setup_raft_cmd(turbo::App &app) {
        auto opt = RaftOptionContext::get_instance();
        auto *ns = app.add_subcommand("raft", "raft control operations");
        ns->callback([ns]() { run_raft_cmd(ns); });

        ns->add_option("-m,--discovery_server", opt->discovery_server, "server address default(\"127.0.0.1:8010\")")->default_val(
                "127.0.0.1:8010");

        auto cg = ns->add_subcommand("status", "cluster status");
        cg->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cg->callback([]() { run_status_cmd(); });

        auto cs = ns->add_subcommand("snapshot", "cluster snapshot");
        cs->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cs->callback([]() { run_snapshot_cmd(); });

        auto cv = ns->add_subcommand("vote", "cluster vote");
        cv->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cv->add_option("-t,--time", opt->vote_time_ms, "election time ms")->required();
        cv->callback([]() { run_vote_cmd(); });

        auto cd = ns->add_subcommand("shutdown", "cluster shutdown");
        cd->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cd->callback([]() { run_shutdown_cmd(); });

        auto cset = ns->add_subcommand("set", "cluster set peer");
        cset->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        cset->add_option("-o,--old", opt->old_peers, "old peers")->required();
        cset->add_option("-n,--new", opt->new_peers, "new peers")->required();
        cset->add_option("-f,--force", opt->force, "new peers")->default_val(false);
        cset->callback([]() { run_set_cmd(); });

        auto ct = ns->add_subcommand("trans", "cluster trans leader");
        ct->add_option("-c,--cluster", opt->cluster, "cluster [discovery|tso|atomic]")->required();
        ct->add_option("-n,--new-leader", opt->new_leader, "cluster new leader")->required();
        ct->callback([]() { run_trans_cmd(); });

        auto func = []() {
            auto opt = RaftOptionContext::get_instance();
            auto r = opt->sender.init(opt->discovery_server);
            if(!r.ok()) {
                turbo::Println(turbo::color::red, "init error:{}", opt->discovery_server);
                exit(0);
            }
        };
        ns->parse_complete_callback(func);
    }

    void RaftCmd::run_raft_cmd(turbo::App *app) {
        if (app->get_subcommands().empty()) {
            turbo::Println("{}", app->help());
        }
    }

    void RaftCmd::run_status_cmd() {
        EA::discovery::RaftControlRequest request;
        EA::discovery::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(EA::discovery::ListPeer);
        auto id = to_region_id();
        if(!id.ok()) {
            turbo::Println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::discovery::SUCCESS);
        if (response.errcode() == EA::discovery::SUCCESS) {
            table = show_raft_result(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void RaftCmd::run_snapshot_cmd() {
        EA::discovery::RaftControlRequest request;
        EA::discovery::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(EA::discovery::SnapShot);
        auto id = to_region_id();
        if(!id.ok()) {
            turbo::Println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::discovery::SUCCESS);
    }

    void RaftCmd::run_vote_cmd() {
        EA::discovery::RaftControlRequest request;
        EA::discovery::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(EA::discovery::ResetVoteTime);
        auto id = to_region_id();
        if(!id.ok()) {
            turbo::Println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::discovery::SUCCESS);
    }

    void RaftCmd::run_shutdown_cmd() {
        EA::discovery::RaftControlRequest request;
        EA::discovery::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(EA::discovery::ShutDown);
        auto id = to_region_id();
        if(!id.ok()) {
            turbo::Println("unknown cluster");
            return;
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::discovery::SUCCESS);
    }

    void RaftCmd::run_set_cmd() {
        EA::discovery::RaftControlRequest request;
        EA::discovery::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(EA::discovery::SetPeer);
        auto id = to_region_id();
        if(!id.ok()) {
            turbo::Println("unknown cluster");
            return;
        }
        auto opt = RaftOptionContext::get_instance();
        for(auto old : opt->old_peers) {
            request.add_old_peers(old);
        }
        for(auto peer : opt->new_peers) {
            request.add_old_peers(peer);
        }
        if(opt->force) {
            request.set_force(true);
        }
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::discovery::SUCCESS);
        if (response.errcode() == EA::discovery::SUCCESS) {
            table = show_raft_result(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    void RaftCmd::run_trans_cmd() {
        EA::discovery::RaftControlRequest request;
        EA::discovery::RaftControlResponse response;
        ScopeShower ss;
        request.set_op_type(EA::discovery::TransLeader);
        auto id = to_region_id();
        if(!id.ok()) {
            turbo::Println("unknown cluster");
            return;
        }
        auto opt = RaftOptionContext::get_instance();
        PREPARE_ERROR_RETURN_OR_OK(ss, id.status(), request);
        request.set_new_leader(opt->new_leader);
        request.set_region_id(id.value());
        auto rs = RaftOptionContext::get_instance()->sender.send_request("raft_control", request, response, 1);
        RPC_ERROR_RETURN_OR_OK(ss, rs, request);
        auto table = ShowHelper::show_response(response.errcode(), request.op_type(),
                                               response.errmsg());
        ss.add_table("result", std::move(table), response.errcode() == EA::discovery::SUCCESS);
        if (response.errcode() == EA::discovery::SUCCESS) {
            table = show_raft_result(response);
            ss.add_table("summary", std::move(table), true);
        }
    }

    turbo::ResultStatus<int> RaftCmd::to_region_id() {
        auto opt = RaftOptionContext::get_instance();
        if(opt->cluster == "discovery") {
            return 0;
        }
        if(opt->cluster == "tso") {
            return 2;
        }
        if(opt->cluster == "atomic") {
            return 1;
        }
        return turbo::InvalidArgumentError("unknown");
    }

    turbo::Table RaftCmd::show_raft_result(EA::discovery::RaftControlResponse &res) {
        turbo::Table summary;
        summary.add_row({"leader","peers"});
        turbo::Table peers;
        for(auto &peer : res.peers()) {
            peers.add_row({peer});
        }
        summary.add_row({turbo::Table().add_row({res.leader()}), peers});
        return summary;
    }
}  // namespace EA::cli
