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
#include "ea/restful/config_server.h"
#include "eaproto/ops/ops.interface.pb.h"
#include "ea/rpc/config_server_interact.h"
#include "ea/base/proto_help.h"
#include <butil/endpoint.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <brpc/controller.h>

namespace EA::restful {

    void ConfigServer::create_config(::google::protobuf::RpcController* controller,
                       const ::EA::proto::ConfigEntity* request,
                       ::EA::proto::ConfigRestfulResponse* response,
                       ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        auto cntl = static_cast<brpc::Controller*>(controller);
        cntl->http_response().set_content_type("text/plain");
        if(!request->has_version()) {
            response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no version");
            return;
        }
        if(!request->has_content()) {
            response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no content");
            return;
        }
        if(!request->has_type()) {
            response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no type");
            return;
        }
        EA::proto::OpsServiceRequest req;
        EA::proto::OpsServiceResponse res;
        req.set_op_type(EA::proto::OP_CREATE_CONFIG);
        *req.mutable_request_config()->mutable_version() = request->version();
        req.mutable_request_config()->set_name(request->name());
        req.mutable_request_config()->set_content(request->content());
        req.mutable_request_config()->set_type(request->type());
        auto ret = EA::rpc::ConfigServerInteract::get_instance()->send_request("config_manage", req, res);
        if(!ret.ok()) {
            response->set_errcode(EA::proto::INTERNAL_ERROR);
            response->set_errmsg("rpc to config server:config_manage");
            TLOG_ERROR("rpc to config server:config_manage error:{}", cntl->ErrorText());
            return;
        }
        response->set_errcode(res.errcode());
        response->set_errmsg(res.errmsg());
    }
    void ConfigServer::remove_config(::google::protobuf::RpcController* controller,
                       const ::EA::proto::ConfigEmptyRequest* request,
                       ::EA::proto::ConfigRestfulResponse* response,
                       ::google::protobuf::Closure* done)  {
        brpc::ClosureGuard done_guard(done);
        auto cntl = static_cast<brpc::Controller*>(controller);
        cntl->http_response().set_content_type("text/plain");
        EA::proto::OpsServiceRequest req;
        EA::proto::OpsServiceResponse res;
        req.set_op_type(EA::proto::OP_REMOVE_CONFIG);
        auto name = cntl->http_request().uri().GetQuery("name");
        if(!name) {
            response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no config name");
            return;
        }
        req.mutable_request_config()->set_name(*name);
        auto version = cntl->http_request().uri().GetQuery("version");
        if(version) {
            auto r = string_to_version(*version, req.mutable_request_config()->mutable_version());
            if(!r.ok()) {
                response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
                response->set_errmsg(std::string(r.message()));
                return;
            }
        }

        auto ret = EA::rpc::ConfigServerInteract::get_instance()->send_request("config_manage", req, res);
        if(!ret.ok()) {
            response->set_errcode(EA::proto::INTERNAL_ERROR);
            response->set_errmsg("rpc to config server:config_manage");
            TLOG_ERROR("rpc to config server:config_manage error:{}", cntl->ErrorText());
            return;
        }
        response->set_errcode(res.errcode());
        response->set_errmsg(res.errmsg());
    }
    void ConfigServer::get_config(::google::protobuf::RpcController* controller,
                    const ::EA::proto::ConfigEmptyRequest* request,
                    ::EA::proto::ConfigRestfulResponse* response,
                    ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        auto cntl = static_cast<brpc::Controller*>(controller);
        cntl->http_response().set_content_type("text/plain");
        EA::proto::QueryOpsServiceRequest req;
        EA::proto::QueryOpsServiceResponse res;
        req.set_op_type(EA::proto::QUERY_GET_CONFIG);
        auto name = cntl->http_request().uri().GetQuery("name");
        if(!name) {
            response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no config name");
            return;
        }
        req.mutable_query_config()->set_name(*name);
        auto version = cntl->http_request().uri().GetQuery("version");
        if(version) {
            auto r = string_to_version(*version, req.mutable_query_config()->mutable_version());
            if(!r.ok()) {
                response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
                response->set_errmsg(std::string(r.message()));
                return;
            }
        }

        auto ret = EA::rpc::ConfigServerInteract::get_instance()->send_request("config_query", req, res);
        if(!ret.ok()) {
            response->set_errcode(EA::proto::INTERNAL_ERROR);
            response->set_errmsg("rpc to config server:config_manage");
            TLOG_ERROR("rpc to config server:config_manage error:{}", cntl->ErrorText());
            return;
        }
        response->set_errcode(res.errcode());
        response->set_errmsg(res.errmsg());
        if(res.errcode() == EA::proto::SUCCESS) {
            *response->mutable_config() = res.config_response().config();
        }

    }
    void ConfigServer::get_config_list(::google::protobuf::RpcController* controller,
                         const ::EA::proto::ConfigEmptyRequest* request,
                         ::EA::proto::ConfigRestfulResponse* response,
                         ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        auto cntl = static_cast<brpc::Controller*>(controller);
        cntl->http_response().set_content_type("text/plain");
        EA::proto::QueryOpsServiceRequest req;
        EA::proto::QueryOpsServiceResponse res;
        req.set_op_type(EA::proto::QUERY_LIST_CONFIG);

        auto ret = EA::rpc::ConfigServerInteract::get_instance()->send_request("config_query", req, res);
        if(!ret.ok()) {
            response->set_errcode(EA::proto::INTERNAL_ERROR);
            response->set_errmsg("rpc to config server:config_manage");
            TLOG_ERROR("rpc to config server:config_manage error:{}", cntl->ErrorText());
            return;
        }
        response->set_errcode(res.errcode());
        response->set_errmsg(res.errmsg());
        if(res.errcode() == EA::proto::SUCCESS) {
            *response->mutable_config_list() = res.config_response().config_list();
        }

    }
    void ConfigServer::get_config_version_list(::google::protobuf::RpcController* controller,
                                 const ::EA::proto::ConfigEmptyRequest* request,
                                 ::EA::proto::ConfigRestfulResponse* response,
                                 ::google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        auto cntl = static_cast<brpc::Controller*>(controller);
        cntl->http_response().set_content_type("text/plain");
        EA::proto::QueryOpsServiceRequest req;
        EA::proto::QueryOpsServiceResponse res;
        req.set_op_type(EA::proto::QUERY_LIST_CONFIG_VERSION);
        auto name = cntl->http_request().uri().GetQuery("name");
        if(!name) {
            response->set_errcode(EA::proto::INPUT_PARAM_ERROR);
            response->set_errmsg("no config name");
            return;
        }
        req.mutable_query_config()->set_name(*name);

        auto ret = EA::rpc::ConfigServerInteract::get_instance()->send_request("config_query", req, res);
        if(!ret.ok()) {
            response->set_errcode(EA::proto::INTERNAL_ERROR);
            response->set_errmsg("rpc to config server:config_manage");
            TLOG_ERROR("rpc to config server:config_manage error:{}", cntl->ErrorText());
            return;
        }
        response->set_errcode(res.errcode());
        response->set_errmsg(res.errmsg());
        if(res.errcode() == EA::proto::SUCCESS) {
            *response->mutable_versions() = res.config_response().versions();
        }


    }

}  // namespace EA::restful

