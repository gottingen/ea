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

#ifndef EA_CLI_SHOW_HELP_H_
#define EA_CLI_SHOW_HELP_H_

#include "turbo/base/status.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include "turbo/format/table.h"
#include "ea/cli/proto_help.h"

namespace EA::cli {
    class ShowHelper {
    public:
        ~ShowHelper();

        static turbo::Table
        show_response(const std::string_view &server, EA::servlet::ErrCode code, EA::servlet::QueryOpType qt,
                      const std::string &msg);

        static turbo::Table
        show_response(EA::servlet::ErrCode code, EA::servlet::QueryOpType qt,
                      const std::string &msg);

        static turbo::Table
        show_response(const std::string_view &server, EA::servlet::ErrCode code, EA::servlet::OpType qt,
                      const std::string &msg);

        static turbo::Table show_response(EA::servlet::ErrCode code, EA::servlet::OpType qt,
                                          const std::string &msg);

        static turbo::Table show_response(EA::servlet::ErrCode code, EA::servlet::RaftControlOp qt,
                                          const std::string &msg);

        static turbo::Table rpc_error_status(const turbo::Status &s, EA::servlet::OpType qt);

        static turbo::Table rpc_error_status(const turbo::Status &s, EA::servlet::QueryOpType qt);

        static turbo::Table rpc_error_status(const turbo::Status &s, EA::servlet::RaftControlOp qt);

        static turbo::Table pre_send_error(const turbo::Status &s, const EA::servlet::MetaManagerRequest &req);


        static turbo::Table pre_send_error(const turbo::Status &s, const EA::servlet::QueryRequest &req);

        static turbo::Table pre_send_error(const turbo::Status &s, const EA::servlet::RaftControlRequest &req);

        static std::string json_format(const std::string &json_str);

    private:
        static turbo::Table
        show_response_impl(const std::string_view &server, EA::servlet::ErrCode code, int qt, const std::string &qts,
                           const std::string &msg);

        static turbo::Table
        show_response_impl(EA::servlet::ErrCode code, int qt, const std::string &qts, const std::string &msg);

        static turbo::Table rpc_error_status_impl(const turbo::Status &s, int qt, const std::string &qts);

        static std::string get_level_str(int level);
    private:
        using Row_t = turbo::Table::Row_t;
        turbo::Table pre_send_result;
        turbo::Table rpc_result;
        turbo::Table meta_response_result;
        turbo::Table result_table;

    };

    ///
    /// inlines
    ///
    inline turbo::Table
    ShowHelper::show_response(const std::string_view &server, EA::servlet::ErrCode code, EA::servlet::QueryOpType qt,
                              const std::string &msg) {
        return show_response_impl(server, code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline turbo::Table
    ShowHelper::show_response(EA::servlet::ErrCode code, EA::servlet::OpType qt,
                              const std::string &msg) {
        return show_response_impl(code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline turbo::Table
    ShowHelper::show_response(EA::servlet::ErrCode code, EA::servlet::RaftControlOp qt,
                              const std::string &msg) {
        return show_response_impl(code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline turbo::Table
    ShowHelper::show_response(EA::servlet::ErrCode code, EA::servlet::QueryOpType qt,
                              const std::string &msg) {
        return show_response_impl(code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline turbo::Table
    ShowHelper::show_response(const std::string_view &server, EA::servlet::ErrCode code, EA::servlet::OpType qt,
                              const std::string &msg) {
        return show_response_impl(server, code, static_cast<int>(qt), get_op_string(qt), msg);
    }

    inline turbo::Table ShowHelper::rpc_error_status(const turbo::Status &s, EA::servlet::OpType qt) {
        return rpc_error_status_impl(s, static_cast<int>(qt), get_op_string(qt));
    }

    inline turbo::Table ShowHelper::rpc_error_status(const turbo::Status &s, EA::servlet::QueryOpType qt) {
        return rpc_error_status_impl(s, static_cast<int>(qt), get_op_string(qt));
    }

    inline turbo::Table ShowHelper::rpc_error_status(const turbo::Status &s, EA::servlet::RaftControlOp qt) {
        return rpc_error_status_impl(s, static_cast<int>(qt), get_op_string(qt));
    }

    struct ScopeShower {
        ~ScopeShower();

        ScopeShower();

        explicit ScopeShower(const std::string &operation);

        void add_table(turbo::Table &&table);

        void add_table(const std::string &stage, turbo::Table &&table, bool ok = true);

        void add_table(const std::string &stage, const std::string &msg, bool ok);

        void prepare(const turbo::Status &status);

        std::vector<turbo::Table> tables;
        turbo::Table result_table;
    };
}  // namespace EA::cli

#define PREPARE_ERROR_RETURN(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("prepare", std::move(ShowHelper::pre_send_error(rs, request))); \
            return;                                            \
        }                                               \
    }while(0)

#define PREPARE_ERROR_RETURN_OR_OK(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                        \
            show.add_table("prepare", std::move(ShowHelper::pre_send_error(rs, request)), false); \
            return;                                            \
        } else {                                               \
            show.add_table("prepare", "ok", true);                   \
        }                                                      \
    }while(0)

#define RPC_ERROR_RETURN_OR_OK(show, rs, request) \
    do {                                            \
        if(!rs.ok()) {                               \
            show.add_table("rpc", std::move(ShowHelper::rpc_error_status(rs, request.op_type())), false);\
            return;                                    \
        } else {                                        \
            show.add_table("rpc","ok", true);                    \
        }                                             \
    }while(0)

#endif  // EA_CLI_SHOW_HELP_H_
