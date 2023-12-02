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
// Created by jeff on 23-11-30.
//
#include "ea/client/servlet_instance_builder.h"
#include "ea/client/utility.h"
#include "turbo/files/sequential_read_file.h"
#include "butil/endpoint.h"
#include "ea/client/loader.h"

namespace EA::client {

    ServletInstanceBuilder::ServletInstanceBuilder(EA::discovery::ServletInstance *ins) :_instance(ins) {
        _instance->Clear();
    }

    void ServletInstanceBuilder::set(EA::discovery::ServletInstance *ins) {
        _instance = ins;
        _instance->Clear();
    }

    turbo::Status ServletInstanceBuilder::build_from_json(const std::string &json_str) {
        auto rs = Loader::load_proto(json_str, *_instance);
        if (!rs.ok()) {
            return rs;
        }
        /// check field
        if (!_instance->has_namespace_name() || _instance->namespace_name().empty()) {
            return turbo::DataLossError("miss required field namespace_name");
        }

        if (!_instance->has_zone_name() || _instance->zone_name().empty()) {
            return turbo::DataLossError("miss required field zone_name");
        }

        if (!_instance->has_servlet_name() || _instance->servlet_name().empty()) {
            return turbo::DataLossError("miss required field servlet_name");
        }

        if (!_instance->has_address() || _instance->address().empty()) {
            return turbo::DataLossError("miss required field address");
        }

        if (!_instance->has_env() || _instance->env().empty()) {
            return turbo::DataLossError("miss required field address");
        }

        butil::EndPoint peer;
        if(butil::str2endpoint(_instance->address().c_str(),&peer) != 0) {
            return turbo::InvalidArgumentError("bad address");
        }

        return turbo::OkStatus();
    }

    turbo::Status ServletInstanceBuilder::build_from_json_file(const std::string &json_path) {
        turbo::SequentialReadFile file;
        auto rs = file.open(json_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string content;
        auto frs = file.read(&content);
        if (!frs.ok()) {
            return frs.status();
        }
        return build_from_json(content);
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_namespace(const std::string &namespace_name) {
        _instance->set_namespace_name(namespace_name);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_zone(const std::string &zone) {
        _instance->set_zone_name(zone);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_servlet(const std::string &servlet) {
        _instance->set_servlet_name(servlet);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_color(const std::string &color) {
        _instance->set_color(color);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_env(const std::string &env) {
        _instance->set_env(env);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_user(const std::string &user) {
        _instance->set_user(user);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_passwd(const std::string &passwd) {
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_status(const std::string &s) {
        EA::discovery::Status status;
        if(EA::discovery::Status_Parse(s, &status)) {
            _instance->set_status(status);
        } else {
            _instance->set_status(EA::discovery::NORMAL);
        }
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_address(const std::string &address) {
        _instance->set_address(address);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_status(const EA::discovery::Status &s) {
        _instance->set_status(EA::discovery::NORMAL);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_weight(int weight) {
        _instance->set_weight(weight);
        return *this;
    }

    ServletInstanceBuilder &ServletInstanceBuilder::set_time(int time) {
        _instance->set_timestamp(time);
        return *this;
    }

}  // namespace EA::client
