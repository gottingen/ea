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
#include "ea/client/dumper.h"
#include "turbo/files/sequential_write_file.h"
#include "json2pb/json_to_pb.h"
#include "json2pb/pb_to_json.h"

namespace EA::client {

    turbo::Status Dumper::dump_proto_to_file(const std::string &path, const google::protobuf::Message &message) {
        std::string content;
        auto rs = dump_proto(message, content);
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
        return turbo::ok_status();
    }

    turbo::Status Dumper::dump_proto(const google::protobuf::Message &message, std::string &content) {
        std::string err;
        content.clear();
        if (!json2pb::ProtoMessageToJson(message, &content, &err)) {
            return turbo::invalid_argument_error(err);
        }
        return turbo::ok_status();
    }

}  // namespace EA::client
