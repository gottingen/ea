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
#include "ea/client/loader.h"
#include "turbo/files/sequential_read_file.h"
#include "json2pb/json_to_pb.h"
#include "json2pb/pb_to_json.h"

namespace EA::client {

    turbo::Status Loader::load_proto(const std::string &content, google::protobuf::Message &message) {
        std::string err;
        if (!json2pb::JsonToProtoMessage(content, &message, &err)) {
            return turbo::InvalidArgumentError(err);
        }
        return turbo::OkStatus();
    }


    turbo::Status Loader::load_proto_from_file(const std::string &path, google::protobuf::Message &message) {
        turbo::SequentialReadFile file;
        auto rs = file.open(path);
        if (!rs.ok()) {
            return rs;
        }
        std::string config_data;
        auto rr = file.read(&config_data);
        if (!rr.ok()) {
            return rr.status();
        }
        return load_proto(config_data, message);
    }

}  // namespace EA::client
