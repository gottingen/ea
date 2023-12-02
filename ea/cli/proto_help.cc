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

    std::string config_type_to_string(EA::servlet::ConfigType type) {
        switch (type) {
            case EA::servlet::CF_JSON:
                return "json";
            case EA::servlet::CF_TEXT:
                return "text";
            case EA::servlet::CF_INI:
                return "ini";
            case EA::servlet::CF_YAML:
                return "yaml";
            case EA::servlet::CF_XML:
                return "xml";
            case EA::servlet::CF_GFLAGS:
                return "gflags";
            case EA::servlet::CF_TOML:
                return "toml";
            default:
                return "unknown format";
        }
    }

    turbo::ResultStatus<EA::servlet::ConfigType> string_to_config_type(const std::string &str) {
        auto lc = turbo::StrToLower(str);
        if (lc == "json") {
            return EA::servlet::CF_JSON;
        } else if (lc == "text") {
            return EA::servlet::CF_TEXT;
        } else if (lc == "ini") {
            return EA::servlet::CF_INI;
        } else if (lc == "yaml") {
            return EA::servlet::CF_YAML;
        } else if (lc == "xml") {
            return EA::servlet::CF_XML;
        } else if (lc == "gflags") {
            return EA::servlet::CF_GFLAGS;
        } else if (lc == "toml") {
            return EA::servlet::CF_TOML;
        }
        return turbo::InvalidArgumentError("unknown format '{}'", str);
    }

    std::string get_op_string(EA::servlet::OpType type) {
        return EA::servlet::OpType_Name(type);
    }

    std::string get_op_string(EA::servlet::RaftControlOp type) {
        return EA::servlet::RaftControlOp_Name(type);
    }

    std::string get_op_string(EA::servlet::QueryOpType type) {
        return EA::servlet::QueryOpType_Name(type);
    }

    turbo::Status string_to_version(const std::string &str, EA::servlet::Version *v) {
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

    std::string version_to_string(const EA::servlet::Version &v) {
        return turbo::Format("{}.{}.{}", v.major(), v.minor(), v.patch());
    }


}  // namespace EA::cli
