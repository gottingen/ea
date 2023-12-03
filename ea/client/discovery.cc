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

#include "ea/client/discovery.h"
#include "ea/client/utility.h"
#include "turbo/files/sequential_read_file.h"
#include "json2pb/json_to_pb.h"
#include "json2pb/pb_to_json.h"
#include "ea/client/config_info_builder.h"
#include "ea/client/loader.h"
#include "ea/client/dumper.h"

namespace EA::client {

    turbo::Status DiscoveryClient::init(BaseMessageSender *sender) {
        _sender = sender;
        return turbo::OkStatus();
    }



    turbo::Status DiscoveryClient::check_config(const std::string &json_content) {
        EA::discovery::ConfigInfo config_pb;
        std::string errmsg;
        if (!json2pb::JsonToProtoMessage(json_content, &config_pb, &errmsg)) {
            return turbo::InvalidArgumentError(errmsg);
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::check_config_file(const std::string &config_path) {
        turbo::SequentialReadFile file;
        auto rs = file.open(config_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string config_data;
        auto rr = file.read(&config_data);
        if (!rr.ok()) {
            return rr.status();
        }
        return check_config(config_data);
    }

    turbo::Status DiscoveryClient::dump_config_file(const std::string &config_path, const EA::discovery::ConfigInfo &config) {
        turbo::SequentialWriteFile file;
        auto rs = file.open(config_path, true);
        if (!rs.ok()) {
            return rs;
        }
        std::string json;
        std::string err;
        if (!json2pb::ProtoMessageToJson(config, &json, &err)) {
            return turbo::InvalidArgumentError(err);
        }
        rs = file.write(json);
        if (!rs.ok()) {
            return rs;
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::create_config(const std::string &config_name, const std::string &content,
                              const std::string &version, const std::string &config_type, int *retry_times) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;

        request.set_op_type(EA::discovery::OP_CREATE_CONFIG);
        auto rc = request.mutable_config_info();
        ConfigInfoBuilder builder(rc);
        auto rs = builder.build_from_content(config_name, content, version, config_type);
        if(!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnavailableError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::create_config(const EA::discovery::ConfigInfo &request,
                              int *retry_times) {
        EA::discovery::DiscoveryManagerRequest discovery_request;
        EA::discovery::DiscoveryManagerResponse response;
        discovery_request.set_op_type(EA::discovery::OP_CREATE_CONFIG);
        *discovery_request.mutable_config_info() = request;
        auto rs = discovery_manager(discovery_request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnavailableError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_config_by_file(const std::string &config_name,
                                                    const std::string &path,
                                                    const std::string &config_type, const std::string &version,
                                                    int *retry_times) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        EA::client::ConfigInfoBuilder builder(request.mutable_config_info());
        auto rs = builder.build_from_file(config_name, path, version, config_type);
        if(!rs.ok()) {
            return rs;
        }
        rs =  discovery_manager(request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnavailableError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_config_by_json(const std::string &json_path, int *retry_times) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        EA::client::ConfigInfoBuilder builder(request.mutable_config_info());
        auto rs = builder.build_from_json_file(json_path);
        if(!rs.ok()) {
            return rs;
        }
        rs =  discovery_manager(request, response, retry_times);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnavailableError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_config(std::vector<std::string> &configs, int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_LIST_CONFIG);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        auto res_configs = response.config_infos();
        for (auto config: res_configs) {
            configs.push_back(config.name());
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_config_version(const std::string &config_name, std::vector<std::string> &versions,
                                                  int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_LIST_CONFIG_VERSION);
        request.set_config_name(config_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        auto res_configs = response.config_infos();
        for (auto &config: res_configs) {
            versions.push_back(version_to_string(config.version()));
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_config_version(const std::string &config_name, std::vector<turbo::ModuleVersion> &versions,
                                    int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_LIST_CONFIG_VERSION);
        request.set_config_name(config_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        auto res_configs = response.config_infos();
        for (auto config: res_configs) {
            versions.emplace_back(config.version().major(),
                                  config.version().minor(), config.version().patch());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config(const std::string &config_name, const std::string &version, EA::discovery::ConfigInfo &config,
                           int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_GET_CONFIG);
        request.set_config_name(config_name);
        auto rs = string_to_version(version, request.mutable_config_version());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        if (response.config_infos_size() != 1) {
            return turbo::InvalidArgumentError("bad proto for config list size not 1");
        }

        config = response.config_infos(0);
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config(const std::string &config_name, const std::string &version, std::string &config,
                           int *retry_time, std::string *type, uint32_t *time) {
        EA::discovery::ConfigInfo config_pb;
        auto rs = get_config(config_name, version, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        if (type) {
            *type = config_type_to_string(config_pb.type());
        }
        if (time) {
            *time = config_pb.time();
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::save_config(const std::string &config_name, const std::string &version, std::string &path,
                                          int *retry_time) {
        std::string content;
        auto rs = get_config(config_name, version, content, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        turbo::SequentialWriteFile file;
        rs = file.open(path, true);
        if (!rs.ok()) {
            return rs;
        }
        rs = file.write(content);
        if (!rs.ok()) {
            return rs;
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::save_config(const std::string &config_name, const std::string &version, int *retry_time) {
        std::string content;
        std::string type;
        auto rs = get_config(config_name, version, content, retry_time, &type);
        if (!rs.ok()) {
            return rs;
        }
        turbo::SequentialWriteFile file;
        auto path = turbo::Format("{}.{}", config_name, type);
        rs = file.open(path, true);
        if (!rs.ok()) {
            return rs;
        }
        rs = file.write(content);
        if (!rs.ok()) {
            return rs;
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, EA::discovery::ConfigInfo &config,
                                  int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_GET_CONFIG);
        request.set_config_name(config_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        if (response.config_infos_size() != 1) {
            return turbo::InvalidArgumentError("bad proto for config list size not 1");
        }

        config = response.config_infos(0);
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, std::string &version,
                                  int *retry_time) {
        EA::discovery::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = version_to_string(config_pb.version());
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, std::string &version,
                                  std::string &type,
                                  int *retry_time) {
        EA::discovery::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = version_to_string(config_pb.version());
        type = config_type_to_string(config_pb.type());
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, turbo::ModuleVersion &version,
                                  int *retry_time) {
        EA::discovery::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = turbo::ModuleVersion(config_pb.version().major(), config_pb.version().minor(),
                                       config_pb.version().patch());
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, turbo::ModuleVersion &version,
                                  std::string &type,
                                  int *retry_time) {
        EA::discovery::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        version = turbo::ModuleVersion(config_pb.version().major(), config_pb.version().minor(),
                                       config_pb.version().patch());
        type = config_type_to_string(config_pb.type());
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_config_latest(const std::string &config_name, std::string &config, int *retry_time) {
        EA::discovery::ConfigInfo config_pb;
        auto rs = get_config_latest(config_name, config_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        config = config_pb.content();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::remove_config(const std::string &config_name, const std::string &version, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;

        request.set_op_type(EA::discovery::OP_REMOVE_CONFIG);
        auto rc = request.mutable_config_info();
        rc->set_name(config_name);
        auto rs = string_to_version(version, rc->mutable_version());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::remove_config(const std::string &config_name, const turbo::ModuleVersion &version, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;

        request.set_op_type(EA::discovery::OP_REMOVE_CONFIG);
        auto rc = request.mutable_config_info();
        rc->set_name(config_name);
        rc->mutable_version()->set_major(version.major);
        rc->mutable_version()->set_minor(version.minor);
        rc->mutable_version()->set_minor(version.patch);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::remove_config_all_version(const std::string &config_name, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;

        request.set_op_type(EA::discovery::OP_REMOVE_CONFIG);
        auto rc = request.mutable_config_info();
        rc->set_name(config_name);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_namespace(EA::discovery::NameSpaceInfo &info, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_NAMESPACE);

        EA::discovery::NameSpaceInfo *ns_req = request.mutable_namespace_info();
        *ns_req = info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_namespace(const std::string &ns, int64_t quota, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_NAMESPACE);

        EA::discovery::NameSpaceInfo *ns_req = request.mutable_namespace_info();
        auto rs = check_valid_name_type(ns);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_namespace_name(ns);
        if (quota != 0) {
            ns_req->set_quota(quota);
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_namespace_by_json(const std::string &json_str, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_NAMESPACE);
        auto rs = Loader::load_proto(json_str, *request.mutable_namespace_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_namespace_by_file(const std::string &path, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_NAMESPACE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_namespace_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::remove_namespace(const std::string &ns, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_DROP_NAMESPACE);

        EA::discovery::NameSpaceInfo *ns_req = request.mutable_namespace_info();
        auto rs = check_valid_name_type(ns);
        if (!rs.ok()) {
            return rs;
        }
        ns_req->set_namespace_name(ns);
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_namespace(EA::discovery::NameSpaceInfo &ns_info, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_MODIFY_NAMESPACE);
        *request.mutable_namespace_info() = ns_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_namespace_by_json(const std::string &json_str, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_MODIFY_NAMESPACE);
        auto rs = Loader::load_proto(json_str, *request.mutable_namespace_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_namespace_by_file(const std::string &path, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_MODIFY_NAMESPACE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_namespace_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_namespace(std::vector<std::string> &ns_list, int *retry_time) {
        std::vector<EA::discovery::NameSpaceInfo> ns_proto_list;
        auto rs = list_namespace(ns_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &ns: ns_proto_list) {
            ns_list.push_back(ns.namespace_name());
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_namespace(std::vector<EA::discovery::NameSpaceInfo> &ns_list, int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_NAMESPACE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        for (auto &ns: response.namespace_infos()) {
            ns_list.push_back(ns);
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_namespace_to_json(std::vector<std::string> &ns_list, int *retry_time) {
        std::vector<EA::discovery::NameSpaceInfo> ns_proto_list;
        auto rs = list_namespace(ns_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &ns: ns_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(ns, json_content);
            if (!r.ok()) {
                return r;
            }
            ns_list.push_back(json_content);
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_namespace_to_file(const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_namespace_to_json(json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        turbo::SequentialWriteFile file;
        rs = file.open(save_path, true);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &ns: json_list) {
            rs = file.write(ns);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_namespace(const std::string &ns_name, EA::discovery::NameSpaceInfo &ns_pb, int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_NAMESPACE);
        if (ns_name.empty()) {
            return turbo::InvalidArgumentError("namespace name empty");
        }
        request.set_namespace_name(ns_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        if (response.namespace_infos_size() != 1) {
            return turbo::UnknownError("bad proto format for namespace info size {}", response.namespace_infos_size());
        }
        ns_pb = response.namespace_infos(0);
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::get_namespace_json(const std::string &ns_name, std::string &json_str, int *retry_time) {
        EA::discovery::NameSpaceInfo ns_pb;
        auto rs = get_namespace(ns_name, ns_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto(ns_pb, json_str);
    }

    turbo::Status
    DiscoveryClient::save_namespace_json(const std::string &ns_name, const std::string &json_path, int *retry_time) {
        EA::discovery::NameSpaceInfo ns_pb;
        auto rs = get_namespace(ns_name, ns_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto_to_file(json_path, ns_pb);
    }


    turbo::Status DiscoveryClient::create_zone(EA::discovery::ZoneInfo &zone_info, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_ZONE);

        auto *zone_req = request.mutable_zone_info();
        *zone_req = zone_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::create_zone(const std::string &ns, const std::string &zone, int64_t quota, int *retry_time) {
        EA::discovery::ZoneInfo zone_pb;
        zone_pb.set_namespace_name(ns);
        zone_pb.set_zone(zone);
        if (quota != 0) {
            zone_pb.set_quota(quota);
        }
        return create_zone(zone_pb, retry_time);
    }

    turbo::Status DiscoveryClient::create_zone_by_json(const std::string &json_str, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_ZONE);
        auto rs = Loader::load_proto(json_str, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::create_zone_by_file(const std::string &path, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_ZONE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::remove_zone(const std::string &ns, const std::string &zone, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_DROP_ZONE);

        auto *zone_req = request.mutable_zone_info();
        zone_req->set_namespace_name(ns);
        zone_req->set_zone(zone);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_zone(EA::discovery::ZoneInfo &zone_info, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_MODIFY_ZONE);

        auto *zone_req = request.mutable_zone_info();
        *zone_req = zone_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_zone_by_json(const std::string &json_str, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_MODIFY_ZONE);
        auto rs = Loader::load_proto(json_str, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_zone_by_file(const std::string &path, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_MODIFY_ZONE);
        auto rs = Loader::load_proto_from_file(path, *request.mutable_zone_info());
        if (!rs.ok()) {
            return rs;
        }
        rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_zone(std::vector<EA::discovery::ZoneInfo> &zone_list, int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_ZONE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        for (auto &zone: response.zone_infos()) {
            zone_list.push_back(zone);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_zone(const std::string &ns, std::vector<EA::discovery::ZoneInfo> &zone_list, int *retry_time) {
        std::vector<EA::discovery::ZoneInfo> all_zone_list;
        auto rs = list_zone(zone_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: all_zone_list) {
            if (zone.namespace_name() == ns) {
                zone_list.push_back(zone);
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_zone(std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<EA::discovery::ZoneInfo> zone_proto_list;
        auto rs = list_zone(zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            zone_list.push_back(turbo::Format("{},{}", zone.namespace_name(), zone.zone()));
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_zone(std::string &ns, std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<EA::discovery::ZoneInfo> zone_proto_list;
        auto rs = list_zone(ns, zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            zone_list.push_back(turbo::Format("{},{}", zone.namespace_name(), zone.zone()));
        }
        return turbo::OkStatus();
    }


    turbo::Status DiscoveryClient::list_zone_to_json(std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<EA::discovery::ZoneInfo> zone_proto_list;
        auto rs = list_zone(zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(zone, json_content);
            if (!r.ok()) {
                return r;
            }
            zone_list.push_back(json_content);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_zone_to_json(const std::string &ns, std::vector<std::string> &zone_list, int *retry_time) {
        std::vector<EA::discovery::ZoneInfo> zone_proto_list;
        auto rs = list_zone(ns, zone_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: zone_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(zone, json_content);
            if (!r.ok()) {
                return r;
            }
            zone_list.push_back(json_content);
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_zone_to_file(const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_zone_to_json(json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        turbo::SequentialWriteFile file;
        rs = file.open(save_path, true);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_zone_to_file(const std::string &ns, const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_zone_to_json(ns, json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        turbo::SequentialWriteFile file;
        rs = file.open(save_path, true);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_zone(const std::string &ns_name, const std::string &zone_name, EA::discovery::ZoneInfo &zone_pb,
                         int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_ZONE);
        if (ns_name.empty()) {
            return turbo::InvalidArgumentError("namespace name empty");
        }
        request.set_namespace_name(ns_name);
        request.set_zone(zone_name);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        if (response.zone_infos_size() != 1) {
            return turbo::UnknownError("bad proto format for zone info size {}", response.zone_infos_size());
        }
        zone_pb = response.zone_infos(0);
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_zone_json(const std::string &ns_name, const std::string &zone_name, std::string &json_str,
                              int *retry_time) {
        EA::discovery::ZoneInfo zone_pb;
        auto rs = get_zone(ns_name, zone_name, zone_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto(zone_pb, json_str);
    }

    turbo::Status
    DiscoveryClient::save_zone_json(const std::string &ns_name, const std::string &zone_name, const std::string &json_path,
                               int *retry_time) {
        EA::discovery::ZoneInfo zone_pb;
        auto rs = get_zone(ns_name, zone_name, zone_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto_to_file(json_path, zone_pb);
    }

    turbo::Status DiscoveryClient::create_servlet(EA::discovery::ServletInfo &servlet_info, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_SERVLET);

        auto *servlet_req = request.mutable_servlet_info();
        *servlet_req = servlet_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::create_servlet(const std::string &ns, const std::string &zone, const std::string &servlet,
                               int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        servlet_pb.set_namespace_name(ns);
        servlet_pb.set_zone(zone);
        servlet_pb.set_servlet_name(servlet);
        return create_servlet(servlet_pb, retry_time);
    }

    turbo::Status DiscoveryClient::create_servlet_by_json(const std::string &json_str, int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        auto rs = Loader::load_proto(json_str, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return create_servlet(servlet_pb, retry_time);
    }

    turbo::Status DiscoveryClient::create_servlet_by_file(const std::string &path, int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        auto rs = Loader::load_proto_from_file(path, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return create_servlet(servlet_pb, retry_time);
    }

    turbo::Status DiscoveryClient::remove_servlet(const std::string &ns, const std::string &zone, const std::string &servlet,
                                             int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_DROP_SERVLET);

        auto *servlet_req = request.mutable_servlet_info();
        servlet_req->set_namespace_name(ns);
        servlet_req->set_zone(zone);
        servlet_req->set_servlet_name(servlet);
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_servlet(const EA::discovery::ServletInfo &servlet_info, int *retry_time) {
        EA::discovery::DiscoveryManagerRequest request;
        EA::discovery::DiscoveryManagerResponse response;
        request.set_op_type(EA::discovery::OP_CREATE_SERVLET);

        auto *servlet_req = request.mutable_servlet_info();
        *servlet_req = servlet_info;
        auto rs = discovery_manager(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::modify_servlet_by_json(const std::string &json_str, int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        auto rs = Loader::load_proto(json_str, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return modify_servlet(servlet_pb, retry_time);
    }

    turbo::Status DiscoveryClient::modify_servlet_by_file(const std::string &path, int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        auto rs = Loader::load_proto_from_file(path, servlet_pb);
        if (!rs.ok()) {
            return rs;
        }
        return modify_servlet(servlet_pb, retry_time);
    }

    turbo::Status DiscoveryClient::list_servlet(std::vector<EA::discovery::ServletInfo> &servlet_list, int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_ZONE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        for (auto &servlet: response.servlet_infos()) {
            servlet_list.push_back(servlet);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet(const std::string &ns, std::vector<EA::discovery::ServletInfo> &servlet_list,
                             int *retry_time) {
        std::vector<EA::discovery::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.namespace_name() == ns) {
                servlet_list.push_back(servlet);
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet(const std::string &ns, const std::string &zone,
                             std::vector<EA::discovery::ServletInfo> &servlet_list, int *retry_time) {
        std::vector<EA::discovery::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.namespace_name() == ns && servlet.zone() == zone) {
                servlet_list.push_back(servlet);
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_servlet(std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<EA::discovery::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            servlet_list.push_back(servlet.servlet_name());
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet(const std::string &ns, std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<EA::discovery::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.namespace_name() == ns) {
                servlet_list.push_back(servlet.servlet_name());
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet(const std::string &ns, const std::string &zone, std::vector<std::string> &servlet_list,
                             int *retry_time) {
        std::vector<EA::discovery::ServletInfo> all_servlet_list;
        auto rs = list_servlet(all_servlet_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: all_servlet_list) {
            if (servlet.namespace_name() == ns && servlet.zone() == zone) {
                servlet_list.push_back(servlet.servlet_name());
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_servlet_to_json(std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<EA::discovery::ServletInfo> servlet_proto_list;
        auto rs = list_servlet(servlet_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: servlet_proto_list) {
            std::string json_content;
            auto r = Dumper::dump_proto(servlet, json_content);
            if (!r.ok()) {
                return r;
            }
            servlet_list.push_back(json_content);
        }
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet_to_json(const std::string &ns, std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<EA::discovery::ServletInfo> servlet_proto_list;
        auto rs = list_servlet(servlet_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: servlet_proto_list) {
            if (servlet.namespace_name() == ns) {
                std::string json_content;
                auto r = Dumper::dump_proto(servlet, json_content);
                if (!r.ok()) {
                    return r;
                }
                servlet_list.push_back(json_content);
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_servlet_to_json(const std::string &ns, const std::string &zone,
                                                   std::vector<std::string> &servlet_list, int *retry_time) {
        std::vector<EA::discovery::ServletInfo> servlet_proto_list;
        auto rs = list_servlet(servlet_proto_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &servlet: servlet_proto_list) {
            if (servlet.namespace_name() == ns && servlet.zone() == zone) {
                std::string json_content;
                auto r = Dumper::dump_proto(servlet, json_content);
                if (!r.ok()) {
                    return r;
                }
                servlet_list.push_back(json_content);
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status DiscoveryClient::list_servlet_to_file(const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_servlet_to_json(json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        turbo::SequentialWriteFile file;
        rs = file.open(save_path, true);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet_to_file(const std::string &ns, const std::string &save_path, int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_servlet_to_json(ns, json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        turbo::SequentialWriteFile file;
        rs = file.open(save_path, true);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::list_servlet_to_file(const std::string &ns, const std::string &zone, const std::string &save_path,
                                     int *retry_time) {
        std::vector<std::string> json_list;
        auto rs = list_servlet_to_json(ns, zone, json_list, retry_time);
        if (!rs.ok()) {
            return rs;
        }

        turbo::SequentialWriteFile file;
        rs = file.open(save_path, true);
        if (!rs.ok()) {
            return rs;
        }
        for (auto &zone: json_list) {
            rs = file.write(zone);
            if (!rs.ok()) {
                return rs;
            }
        }
        file.close();
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_servlet(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                            EA::discovery::ServletInfo &servlet_pb,
                            int *retry_time) {
        EA::discovery::DiscoveryQueryRequest request;
        EA::discovery::DiscoveryQueryResponse response;
        request.set_op_type(EA::discovery::QUERY_ZONE);
        auto rs = discovery_query(request, response, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        if (response.errcode() != EA::discovery::SUCCESS) {
            return turbo::UnknownError(response.errmsg());
        }
        if (response.servlet_infos_size() != 1) {
            return turbo::UnknownError("bad proto format for servlet infos size: {}", response.servlet_infos_size());
        }
        servlet_pb = response.servlet_infos(0);
        return turbo::OkStatus();
    }

    turbo::Status
    DiscoveryClient::get_servlet_json(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                                 std::string &json_str,
                                 int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        auto rs = get_servlet(ns_name, zone_name, servlet, servlet_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        return Dumper::dump_proto(servlet_pb, json_str);
    }

    turbo::Status
    DiscoveryClient::save_servlet_json(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                                  const std::string &json_path,
                                  int *retry_time) {
        EA::discovery::ServletInfo servlet_pb;
        auto rs = get_servlet(ns_name, zone_name, servlet, servlet_pb, retry_time);
        if (!rs.ok()) {
            return rs;
        }
        rs = Dumper::dump_proto_to_file(json_path, servlet_pb);
        return rs;

    }

}  // namespace EA::client

