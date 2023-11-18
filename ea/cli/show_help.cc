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

#include "ea/cli/show_help.h"
#include "turbo/format/print.h"
#include "turbo/times/civil_time.h"
#include "turbo/times/time.h"

namespace EA::client {
    using Row_t = turbo::Table::Row_t;

    ShowHelper::~ShowHelper() {
        std::cout << pre_send_result << std::endl;
        std::cout << rpc_result << std::endl;
        std::cout << meta_response_result << std::endl;
        std::cout << result_table << std::endl;
    }

    turbo::Table ShowHelper::show_response_impl(const std::string_view &server, EA::proto::ErrCode code, int qt, const std::string &qts, const std::string &msg) {
        turbo::Table response_result;
        response_result.add_row(Row_t{"status", "server", "op code", "op string", "error code", "error message"});
        if (code != EA::proto::SUCCESS) {
            response_result.add_row(
                    Row_t{"fail", server, turbo::Format(qt),qts,turbo::Format("{}", static_cast<int>(code)), msg});
        } else {
            response_result.add_row(
                    Row_t{"success", server, turbo::Format(qt),qts,turbo::Format("{}", static_cast<int>(code)), msg});
        }
        auto last = response_result.size() - 1;
        response_result[last][0].format().font_color(turbo::Color::green).font_style({turbo::FontStyle::bold});

        if (code == EA::proto::SUCCESS) {
            response_result[last][1].format().font_color(turbo::Color::yellow);
        } else {
            response_result[last][1].format().font_color(turbo::Color::red);
        }
        return response_result;
    }

    turbo::Table ShowHelper::rpc_error_status_impl(const turbo::Status &s, int qt, const std::string &qts) {
        turbo::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error code", "error message"});
        auto last = result.size() - 1;
        result[last].format().font_color(turbo::Color::yellow).font_style({turbo::FontStyle::bold});
        if (s.ok()) {
            result.add_row(
                    Row_t{"success", turbo::Format(qt), qts,
                          turbo::Format("{}", static_cast<int>(s.code())), s.message()});
            last = result.size() - 1;
            result[last][0].format().font_color(turbo::Color::green);
            result[last][1].format().font_color(turbo::Color::yellow);
            result[last][2].format().font_color(turbo::Color::yellow);
            result[last][3].format().font_color(turbo::Color::green);
            result[last][4].format().font_color(turbo::Color::green);
        } else {
            result.add_row(
                    Row_t{"fail", turbo::Format(qt), qts,
                          turbo::Format("{}", static_cast<int>(s.code())), s.message()});
            last = result.size() - 1;
            result[last][0].format().font_color(turbo::Color::red);
            result[last][1].format().font_color(turbo::Color::yellow);
            result[last][2].format().font_color(turbo::Color::yellow);
            result[last][3].format().font_color(turbo::Color::red);
            result[last][4].format().font_color(turbo::Color::red);
        }
        return result;
    }



    turbo::Table ShowHelper::pre_send_error(const turbo::Status &s, const EA::proto::OpsServiceRequest &req) {
        turbo::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error message"});
        result[0].format().font_color(turbo::Color::green).font_style({turbo::FontStyle::bold}).font_align(
                turbo::FontAlign::center);
        if (!req.has_op_type()) {
            result.add_row(Row_t{"fail", "nil", "nil", "op_type field is required but not set not set"});
            auto last = result.size() - 1;
            result[last].format().font_color(turbo::Color::red).font_style(
                    {turbo::FontStyle::bold}).font_align(
                    turbo::FontAlign::center);
        } else if (!s.ok()) {
            result.add_row(
                    Row_t{"fail", turbo::Format("{}", static_cast<int>(req.op_type())),
                          EA::rpc::get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(turbo::Color::red).font_style(
                    {turbo::FontStyle::bold}).font_align(
                    turbo::FontAlign::center);
        } else {
            result.add_row(
                    Row_t{"success", turbo::Format("{}", static_cast<int>(req.op_type())),
                          EA::rpc::get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(turbo::Color::green).font_style(
                    {turbo::FontStyle::bold}).font_align(
                    turbo::FontAlign::center);
        }
        return result;

    }

    turbo::Table ShowHelper::pre_send_error(const turbo::Status &s, const EA::proto::QueryOpsServiceRequest &req) {
        turbo::Table result;
        result.add_row(Row_t{"status", "op code", "op string", "error message"});
        result[0].format().font_color(turbo::Color::green).font_style({turbo::FontStyle::bold}).font_align(
                turbo::FontAlign::center);
        if (!req.has_op_type()) {
            result.add_row(Row_t{"fail", "nil", "nil", "op_type field is required but not set not set"});
            auto last = result.size() - 1;
            result[last].format().font_color(turbo::Color::red).font_style(
                    {turbo::FontStyle::bold}).font_align(
                    turbo::FontAlign::center);
        } else if (!s.ok()) {
            result.add_row(
                    Row_t{"fail", turbo::Format("{}", static_cast<int>(req.op_type())),
                          EA::rpc::get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(turbo::Color::red).font_style(
                    {turbo::FontStyle::bold}).font_align(
                    turbo::FontAlign::center);
        } else {
            result.add_row(
                    Row_t{"success", turbo::Format("{}", static_cast<int>(req.op_type())),
                          EA::rpc::get_op_string(req.op_type()),
                          s.message()});
            auto last = result.size() - 1;
            result[last][0].format().font_color(turbo::Color::green).font_style(
                    {turbo::FontStyle::bold}).font_align(
                    turbo::FontAlign::center);
        }
        return result;

    }


}  // namespace EA::client
