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

#include "ea/cli/dict_cmd.h"
#include "ea/cli/option_context.h"
#include "eaproto/router/router.interface.pb.h"
#include "turbo/format/print.h"
#include "ea/cli/show_help.h"
#include "ea/rpc/router_interact.h"
#include "turbo/module/module_version.h"
#include "turbo/files/filesystem.h"
#include "turbo/files/sequential_read_file.h"
#include "turbo/times/clock.h"
#include "ea/base/file_util.h"
#include "turbo/files/utility.h"

namespace EA::client {

    void setup_dict_cmd(turbo::App &app) {
        // Create the option and subcommand objects.
        auto opt = DictOptionContext::get_instance();
        auto *ns = app.add_subcommand("dict", "dict operations");
        ns->callback([ns]() { run_dict_cmd(ns); });

        auto cc = ns->add_subcommand("create", " create dict");
        cc->add_option("-n,--name", opt->dict_name, "dict name")->required();
        cc->add_option("-v, --version", opt->dict_version, "dict version [1.2.3]")->required();
        cc->add_option("-e, --extension", opt->dict_ext, "platform type [linux|osx|windows]")->default_val("");
        cc->add_option("-f, --file", opt->dict_file, "local dict file")->required();
        cc->callback([]() { run_dict_create_cmd(); });

        // upload
        auto cp = ns->add_subcommand("upload", " upload dict");
        cp->add_option("-n,--name", opt->dict_name, "dict name")->required();
        cp->add_option("-v, --version", opt->dict_version, "dict version [1.2.3]")->required();
        cp->add_option("-e, --extension", opt->dict_ext, "platform type [linux|osx|windows]")->default_val("");
        cp->add_option("-f, --file", opt->dict_file, "local dict file")->required();
        cp->add_option("-b, --block", opt->dict_block_size, "block size once")->default_val(int64_t{4096});
        cp->callback([]() { run_dict_upload_cmd(); });

        /// list/ list version
        auto cl = ns->add_subcommand("list", " list dict");
        cl->add_option("-n,--name", opt->dict_name, "dict name");
        cl->add_flag("-t,--tombstone", opt->dict_query_tombstone, "dict name")->default_val(false);
        cl->callback([]() { run_dict_list_cmd(); });

        /// info
        auto cg = ns->add_subcommand("info", " get dict info");
        cg->add_flag("-t,--tombstone", opt->dict_query_tombstone, "dict name")->default_val(false);
        cg->add_option("-n,--name", opt->dict_name, "dict name")->required();
        cg->add_option("-v, --version", opt->dict_version, "dict version");
        cg->callback([]() { run_dict_info_cmd(); });

        /// download
        auto cd = ns->add_subcommand("download", " download dict info");
        cd->add_option("-n,--name", opt->dict_name, "dict name")->required();
        cd->add_option("-v, --version", opt->dict_version, "dict version");
        cd->add_option("-f, --file", opt->dict_file, "local dict file");
        cd->add_option("-b, --block", opt->dict_block_size, "block size once")->default_val(int64_t{4096});
        cd->callback([]() { run_dict_download_cmd(); });

        /// remove
        auto cr = ns->add_subcommand("remove", " remove dict");
        cr->add_flag("-t,--tombstone", opt->dict_query_tombstone, "dict name")->default_val(false);
        cr->add_option("-n,--name", opt->dict_name, "dict name")->required();
        cr->add_option("-v, --version", opt->dict_version, "dict version [1.2.3]");
        cr->callback([]() { run_dict_remove_cmd(); });
        /// restore
        auto ct = ns->add_subcommand("restore", " restore dict");
        ct->add_option("-n,--name", opt->dict_name, "dict name")->required();
        ct->add_option("-v, --version", opt->dict_version, "dict version [1.2.3]");
        ct->callback([]() { run_dict_restore_cmd(); });
    }

    /// The function that runs our code.
    /// This could also simply be in the callback lambda itself,
    /// but having a separate function is cleaner.
    void run_dict_cmd(turbo::App *app) {
        // Do stuff...
        if (app->get_subcommands().empty()) {
            turbo::Println("{}", app->help());
        }
    }

    void run_dict_create_cmd() {
        EA::proto::OpsServiceRequest request;
        EA::proto::OpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_dict_create(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("dict_manage", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                               response.op_type(),
                                               response.errmsg());
        ss.add_table(std::move(table));
    }

    void run_dict_upload_cmd() {
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;
        {

            ScopeShower ss;
            auto rs = make_dict_info(&request);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
                return;
            }
            rs = RouterInteract::get_instance()->send_request("dict_query", request, response);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
                return;
            }
            auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                                   response.op_type(),
                                                   response.errmsg());
            ss.add_table(std::move(table));
            if (response.errcode() != EA::proto::SUCCESS) {
                return;
            }

            // already upload
            if (response.dict_response().dict().finish()) {
                table = show_query_ops_dict_info_response(response);
                ss.add_table(std::move(table));
                return;
            }
        }
        auto upload_size = response.dict_response().dict().upload_size();
        auto total_size = response.dict_response().dict().size();

        EA::proto::OpsServiceRequest upload_request;
        EA::proto::OpsServiceResponse upload_response;
        ScopeShower ss;
        auto rs = make_dict_upload(&upload_request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, upload_request)));
            return;
        }
        auto block = DictOptionContext::get_instance()->dict_block_size;
        std::string buf;
        buf.reserve(block);
        turbo::SequentialReadFile file;
        rs = file.open(DictOptionContext::get_instance()->dict_file);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, upload_request)));
            return;
        }
        rs = file.skip(upload_size);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, upload_request)));
            return;
        }
        turbo::Println("upload size:{} total size: {} block size: {}", upload_size, total_size, block);
        while (upload_size < total_size) {
            buf.clear();
            auto frs = file.read(&buf, block);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::pre_send_error(frs.status(), upload_request)));
                return;
            }
            turbo::Println("offset:{} count: {}", upload_size, frs.value());
            upload_request.mutable_request_dict()->set_offset(upload_size);
            upload_request.mutable_request_dict()->set_count(frs.value());
            upload_request.mutable_request_dict()->set_content(buf);
            auto r = RouterInteract::get_instance()->send_request("dict_manage", upload_request, upload_response);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::rpc_error_status(r, upload_request.op_type())));
                return;
            }
            upload_size += frs.value();
        }


        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, upload_response.errcode(),
                                               upload_response.op_type(),
                                               upload_response.errmsg());
        ss.add_table(std::move(table));
    }

    void run_dict_download_cmd() {
        auto opt = DictOptionContext::get_instance();
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;
        {

            ScopeShower ss;
            auto rs = make_dict_info(&request);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
                return;
            }
            rs = RouterInteract::get_instance()->send_request("dict_query", request, response);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
                return;
            }
            auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                                   response.op_type(),
                                                   response.errmsg());
            ss.add_table(std::move(table));
            if (response.errcode() != EA::proto::SUCCESS) {
                return;
            }

            // already upload
            if (!response.dict_response().dict().finish()) {
                table = show_query_ops_dict_info_response(response);
                ss.add_table(std::move(table));
                return;
            }
        }

        auto &dict_info = response.dict_response().dict();
        opt->dict_version = version_to_string(dict_info.version());
        auto total_size = dict_info.size();
        auto download_size = 0;
        auto cksm = dict_info.cksm();

        EA::proto::QueryOpsServiceRequest download_request;
        EA::proto::QueryOpsServiceResponse download_response;
        ScopeShower ss;
        auto rs = make_dict_download(&download_request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, download_request)));
            return;
        }
        auto block = opt->dict_block_size;
        turbo::SequentialWriteFile file;
        std::string file_path = opt->dict_file;
        if(file_path.empty()) {
            file_path = make_dict_filename(dict_info.name(), dict_info.version(), dict_info.ext());
        }
        rs = file.open(file_path, true);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, download_request)));
            return;
        }

        int64_t current_block_size;
        turbo::Println("need to download count: {}", total_size);
        while (download_size < total_size) {
            auto left = total_size - download_size;
            if(left > block) {
                current_block_size = block;
            } else {
                current_block_size = left;
            }
            download_request.mutable_query_dict()->set_offset(download_size);
            download_request.mutable_query_dict()->set_count(current_block_size);
            auto r = RouterInteract::get_instance()->send_request("dict_query", download_request, download_response);
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::rpc_error_status(r, download_request.op_type())));
                return;
            }
            auto frs = file.write(download_response.dict_response().content().data(), download_response.dict_response().content().size());
            if (!rs.ok()) {
                ss.add_table(std::move(ShowHelper::pre_send_error(frs, download_request)));
                return;
            }
            download_size += download_response.dict_response().content().size();
            turbo::Println("offset:{} count: {}", download_size, current_block_size);
        }
        file.close();
        auto download_cksm = turbo::FileUtility::md5_sum_file(file_path);
        if(!download_cksm.ok()) {
            turbo::Println("cksm download:{} fail : {}", file_path, download_cksm.status().message());
            return;
        }
        if(cksm != download_cksm.value()) {
            turbo::Println("cksm download dict :{} fail, get:{} expect:{} ", file_path, download_cksm.value(), cksm);
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, download_response.errcode(),
                                               download_response.op_type(),
                                               download_response.errmsg());
        ss.add_table(std::move(table));
    }


    void run_dict_remove_cmd() {
        EA::proto::OpsServiceRequest request;
        EA::proto::OpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_dict_remove(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("dict_manage", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                               response.op_type(),
                                               response.errmsg());
        ss.add_table(std::move(table));
    }

    void run_dict_restore_cmd() {
        EA::proto::OpsServiceRequest request;
        EA::proto::OpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_dict_restore(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("dict_manage", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                               response.op_type(),
                                               response.errmsg());
        ss.add_table(std::move(table));
    }

    void run_dict_info_cmd() {
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_dict_info(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("dict_query", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                               response.op_type(),
                                               response.errmsg());
        ss.add_table(std::move(table));
        if (response.errcode() != EA::proto::SUCCESS) {
            return;
        }
        table = show_query_ops_dict_info_response(response);
        ss.add_table(std::move(table));
    }

    void run_dict_list_cmd() {
        if (!DictOptionContext::get_instance()->dict_name.empty()) {
            run_dict_version_list_cmd();
            return;
        }
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_dict_list(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("dict_query", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                               response.op_type(),
                                               response.errmsg());
        ss.add_table(std::move(table));
        if (response.errcode() == EA::proto::SUCCESS) {
            table = show_query_ops_dict_list_response(response);
            ss.add_table(std::move(table));
        }
    }


    void run_dict_version_list_cmd() {
        EA::proto::QueryOpsServiceRequest request;
        EA::proto::QueryOpsServiceResponse response;

        ScopeShower ss;
        auto rs = make_list_dict_version(&request);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::pre_send_error(rs, request)));
            return;
        }
        rs = RouterInteract::get_instance()->send_request("dict_query", request, response);
        if (!rs.ok()) {
            ss.add_table(std::move(ShowHelper::rpc_error_status(rs, request.op_type())));
            return;
        }
        auto table = ShowHelper::show_response(OptionContext::get_instance()->server, response.errcode(),
                                               response.op_type(),
                                               response.errmsg());
        ss.add_table(std::move(table));
        if (response.errcode() == EA::proto::SUCCESS) {
            table = show_query_ops_dict_list_version_response(response);
            ss.add_table(std::move(table));
        }
    }

    [[nodiscard]] turbo::Status
    make_dict_create(EA::proto::OpsServiceRequest *req) {
        req->set_op_type(EA::proto::OP_CREATE_DICT);
        auto rc = req->mutable_request_dict()->mutable_dict();
        auto opt = DictOptionContext::get_instance();
        rc->set_name(opt->dict_name);
        rc->set_time(static_cast<int>(turbo::ToTimeT(turbo::Now())));
        rc->set_ext(opt->dict_ext);
        auto v = rc->mutable_version();
        auto st = string_to_version(opt->dict_version, v);
        if (!st.ok()) {
            return st;
        }
        std::error_code ec;
        if (!turbo::filesystem::exists(opt->dict_file)) {
            return turbo::NotFoundError("dict file not found");
        }
        auto file_size = turbo::filesystem::file_size(opt->dict_file);
        if (file_size <= 0) {
            return turbo::NotFoundError("file size < 0");
        }
        rc->set_size(file_size);
        int64_t nsize;
        auto cksm = turbo::FileUtility::md5_sum_file(opt->dict_file, &nsize);
        if (!cksm.ok()) {
            return cksm.status();
        }
        rc->set_cksm(cksm.value());
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_dict_upload(EA::proto::OpsServiceRequest *req) {
        req->set_op_type(EA::proto::OP_UPLOAD_DICT);
        auto rc = req->mutable_request_dict()->mutable_dict();
        auto opt = DictOptionContext::get_instance();
        rc->set_name(opt->dict_name);
        rc->set_time(static_cast<int>(turbo::ToTimeT(turbo::Now())));
        rc->set_ext(opt->dict_ext);
        auto v = rc->mutable_version();
        auto st = string_to_version(opt->dict_version, v);
        if (!st.ok()) {
            return st;
        }
        std::error_code ec;
        if (!turbo::filesystem::exists(opt->dict_file, ec)) {
            return turbo::NotFoundError("dict file not found");
        }
        auto file_size = turbo::filesystem::file_size(opt->dict_file);
        if (file_size <= 0) {
            return turbo::NotFoundError("file size < 0");
        }
        rc->set_size(file_size);
        int64_t nsize;
        auto cksm = turbo::FileUtility::md5_sum_file(opt->dict_file, &nsize);
        if (!cksm.ok()) {
            return cksm.status();
        }
        rc->set_cksm(cksm.value());
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_dict_remove(EA::proto::OpsServiceRequest *req) {
        auto opt = DictOptionContext::get_instance();
        if (opt->dict_query_tombstone) {
            req->set_op_type(EA::proto::OP_REMOVE_TOMBSTONE_DICT);
        } else {
            req->set_op_type(EA::proto::OP_REMOVE_DICT);
        }
        auto rc = req->mutable_request_dict()->mutable_dict();
        rc->set_name(opt->dict_name);
        if (!opt->dict_version.empty()) {
            auto v = rc->mutable_version();
            return string_to_version(opt->dict_version, v);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_dict_restore(EA::proto::OpsServiceRequest *req) {
        req->set_op_type(EA::proto::OP_RESTORE_TOMBSTONE_DICT);
        auto rc = req->mutable_request_dict()->mutable_dict();
        auto opt = DictOptionContext::get_instance();
        rc->set_name(opt->dict_name);
        if (!opt->dict_version.empty()) {
            auto v = rc->mutable_version();
            return string_to_version(opt->dict_version, v);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_dict_list(EA::proto::QueryOpsServiceRequest *req) {
        auto opt = DictOptionContext::get_instance();
        if (opt->dict_query_tombstone) {
            req->set_op_type(EA::proto::QUERY_TOMBSTONE_LIST_DICT);
        } else {
            req->set_op_type(EA::proto::QUERY_LIST_DICT);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_list_dict_version(EA::proto::QueryOpsServiceRequest *req) {
        auto opt = DictOptionContext::get_instance();
        if (opt->dict_query_tombstone) {
            req->set_op_type(EA::proto::QUERY_TOMBSTONE_LIST_DICT_VERSION);
        } else {
            req->set_op_type(EA::proto::QUERY_LIST_DICT_VERSION);
        }
        req->mutable_query_dict()->set_name(opt->dict_name);
        return turbo::OkStatus();
    }


    turbo::Table show_query_ops_dict_list_response(const EA::proto::QueryOpsServiceResponse &res) {
        turbo::Table result;
        auto &dict_list = res.dict_response().dict_list();
        result.add_row(turbo::Table::Row_t{"tombstone", turbo::Format(DictOptionContext::get_instance()->dict_query_tombstone)});
        auto last = result.size() - 1;
        if (!DictOptionContext::get_instance()->dict_query_tombstone) {
            result[last].format().font_color(turbo::Color::green);
        } else {
            result[last].format().font_color(turbo::Color::red);
        }

        result.add_row(turbo::Table::Row_t{"dict size", turbo::Format(dict_list.size())});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        result.add_row(turbo::Table::Row_t{"number", "dict"});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        int i = 0;
        for (auto &ns: dict_list) {
            result.add_row(turbo::Table::Row_t{turbo::Format(i++), ns});
            last = result.size() - 1;
            result[last].format().font_color(turbo::Color::yellow);

        }
        return result;
    }

    turbo::Table show_query_ops_dict_list_version_response(const EA::proto::QueryOpsServiceResponse &res) {
        turbo::Table result;
        auto &dict_versions = res.dict_response().versions();

        result.add_row(turbo::Table::Row_t{"tombstone", turbo::Format(DictOptionContext::get_instance()->dict_query_tombstone)});
        auto last = result.size() - 1;
        if (!DictOptionContext::get_instance()->dict_query_tombstone) {
            result[last].format().font_color(turbo::Color::green);
        } else {
            result[last].format().font_color(turbo::Color::red);
        }

        result.add_row(turbo::Table::Row_t{"dict", DictOptionContext::get_instance()->dict_name});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);

        result.add_row(turbo::Table::Row_t{"version size", turbo::Format(dict_versions.size())});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        result.add_row(turbo::Table::Row_t{"number", "version"});
        last = result.size() - 1;
        result[last].format().font_color(turbo::Color::green);
        int i = 0;
        for (auto &ns: dict_versions) {
            result.add_row(
                    turbo::Table::Row_t{turbo::Format(i++),
                                        turbo::Format("{}.{}.{}", ns.major(), ns.minor(), ns.patch())});
            last = result.size() - 1;
            result[last].format().font_color(turbo::Color::yellow);

        }
        return result;
    }

    [[nodiscard]] turbo::Status
    make_dict_info(EA::proto::QueryOpsServiceRequest *req) {
        auto opt = DictOptionContext::get_instance();
        if (!opt->dict_query_tombstone) {
            req->set_op_type(EA::proto::QUERY_INFO_DICT);
        } else {
            req->set_op_type(EA::proto::QUERY_TOMBSTONE_DICT_INFO);
        }
        auto rc = req->mutable_query_dict();
        rc->set_name(opt->dict_name);
        if (!opt->dict_version.empty()) {
            auto v = rc->mutable_version();
            return string_to_version(opt->dict_version, v);
        }
        return turbo::OkStatus();
    }

    [[nodiscard]] turbo::Status
    make_dict_download(EA::proto::QueryOpsServiceRequest *req) {
        auto opt = DictOptionContext::get_instance();
        req->set_op_type(EA::proto::QUERY_DOWNLOAD_DICT);
        auto rc = req->mutable_query_dict();
        rc->set_name(opt->dict_name);
        auto v = rc->mutable_version();
        return string_to_version(opt->dict_version, v);
    }

    turbo::Table show_query_ops_dict_info_response(const EA::proto::QueryOpsServiceResponse &res) {
        turbo::Table result_table;
        auto result = res.dict_response().dict();
        // name
        result_table.add_row(turbo::Table::Row_t{"name ", result.name()});
        auto last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        // version
        result_table.add_row(turbo::Table::Row_t{"version", turbo::Format("{}.{}.{}", result.version().major(),
                                                                          result.version().minor(),
                                                                          result.version().patch())});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        // finish
        result_table.add_row(turbo::Table::Row_t{"upload finish", turbo::Format(result.finish())});
        last = result_table.size() - 1;
        if (result.finish()) {
            result_table[last].format().font_color(turbo::Color::green);
        } else {
            result_table[last].format().font_color(turbo::Color::red);
        }
        // tombstone
        result_table.add_row(turbo::Table::Row_t{"tombstone", turbo::Format(result.tombstone())});
        last = result_table.size() - 1;
        if (result.tombstone()) {
            result_table[last].format().font_color(turbo::Color::red);
        } else {
            result_table[last].format().font_color(turbo::Color::green);
        }
        // platform
        result_table.add_row(turbo::Table::Row_t{"ext", result.ext()});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);

        // size
        result_table.add_row(turbo::Table::Row_t{"size", turbo::Format(result.size())});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        // upload size
        result_table.add_row(turbo::Table::Row_t{"upload size", turbo::Format(result.upload_size())});
        last = result_table.size() - 1;
        if (result.upload_size() != result.size()) {
            result_table[last].format().font_color(turbo::Color::red);
        } else {
            result_table[last].format().font_color(turbo::Color::green);
        }
        // cksm
        result_table.add_row(turbo::Table::Row_t{"cksm", result.cksm()});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);
        // time
        turbo::Time cs = turbo::FromTimeT(result.time());
        result_table.add_row(turbo::Table::Row_t{"time", turbo::FormatTime(cs)});
        last = result_table.size() - 1;
        result_table[last].format().font_color(turbo::Color::green);

        return result_table;
    }

    std::string make_dict_filename(const std::string &name, const EA::proto::Version &version,const std::string &ext) {
        if (ext.empty()) {
            return turbo::Format("{}.{}.{}.{}", name, version.major(), version.minor(), version.patch());
        } else {
            return turbo::Format("{}.{}.{}.{}.{}", name, ext, version.major(), version.minor(), version.patch());
        }

    }

}  // namespace EA::client
