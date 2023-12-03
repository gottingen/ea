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

#include "ea/cli/proto_help.h"
#include "turbo/strings/utility.h"
#include "turbo/meta/reflect.h"

namespace EA::cli {

    std::string config_type_to_string(EA::discovery::ConfigType type) {
        switch (type) {
            case EA::discovery::CF_JSON:
                return "json";
            case EA::discovery::CF_TEXT:
                return "text";
            case EA::discovery::CF_INI:
                return "ini";
            case EA::discovery::CF_YAML:
                return "yaml";
            case EA::discovery::CF_XML:
                return "xml";
            case EA::discovery::CF_GFLAGS:
                return "gflags";
            case EA::discovery::CF_TOML:
                return "toml";
            default:
                return "unknown format";
        }
    }

    turbo::ResultStatus<EA::discovery::ConfigType> string_to_config_type(const std::string &str) {
        auto lc = turbo::StrToLower(str);
        if (lc == "json") {
            return EA::discovery::CF_JSON;
        } else if (lc == "text") {
            return EA::discovery::CF_TEXT;
        } else if (lc == "ini") {
            return EA::discovery::CF_INI;
        } else if (lc == "yaml") {
            return EA::discovery::CF_YAML;
        } else if (lc == "xml") {
            return EA::discovery::CF_XML;
        } else if (lc == "gflags") {
            return EA::discovery::CF_GFLAGS;
        } else if (lc == "toml") {
            return EA::discovery::CF_TOML;
        }
        return turbo::InvalidArgumentError("unknown format '{}'", str);
    }

    std::string get_op_string(EA::discovery::OpType type) {
        return EA::discovery::OpType_Name(type);
    }

    std::string get_op_string(EA::RaftControlOp type) {
        return EA::RaftControlOp_Name(type);
    }

    std::string get_op_string(EA::discovery::QueryOpType type) {
        return EA::discovery::QueryOpType_Name(type);
    }

    turbo::Status string_to_version(const std::string &str, EA::discovery::Version *v) {
        std::vector<std::string> vs = turbo::StrSplit(str, ".");
        if (vs.size() != 3)
            return turbo::InvalidArgumentError("version {} error, should be like 1.2.3", str);
        int64_t m;
        if (!turbo::SimpleAtoi(vs[0], &m)) {
            return turbo::InvalidArgumentError("version {} error, should be like 1.2.3", str);
        }
        v->set_major(m);
        if (!turbo::SimpleAtoi(vs[1], &m)) {
            return turbo::InvalidArgumentError("version {} error, should be like 1.2.3", str);
        }
        v->set_minor(m);
        if (!turbo::SimpleAtoi(vs[2], &m)) {
            return turbo::InvalidArgumentError("version {} error, should be like 1.2.3", str);
        }
        v->set_patch(m);
        return turbo::OkStatus();
    }

    std::string version_to_string(const EA::discovery::Version &v) {
        return turbo::Format("{}.{}.{}", v.major(), v.minor(), v.patch());
    }


}  // namespace EA::cli
