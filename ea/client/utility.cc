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

#include "ea/client/utility.h"
#include "turbo/strings/utility.h"
#include "turbo/container/flat_hash_set.h"

namespace EA::client {

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

    turbo::Status string_to_version(const std::string &str, EA::servlet::Version *v) {
        std::vector<std::string> vs = turbo::StrSplit(str, ".");
        if (vs.size() != 3)
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        int64_t m;
        if (!turbo::SimpleAtoi(vs[0], &m)) {
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        }
        v->set_major(m);
        if (!turbo::SimpleAtoi(vs[1], &m)) {
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        }
        v->set_minor(m);
        if (!turbo::SimpleAtoi(vs[2], &m)) {
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        }
        v->set_patch(m);
        return turbo::OkStatus();
    }

    turbo::Status string_to_module_version(const std::string &str, turbo::ModuleVersion *v) {

        std::vector<std::string> vs = turbo::StrSplit(str, ".");
        if (vs.size() != 3)
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        int64_t ma;
        if (!turbo::SimpleAtoi(vs[0], &ma)) {
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        }
        int64_t mi;
        if (!turbo::SimpleAtoi(vs[1], &mi)) {
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        }
        int64_t pa;
        if (!turbo::SimpleAtoi(vs[2], &pa)) {
            return turbo::InvalidArgumentError("version error, should be like 1.2.3");
        }
        *v = turbo::ModuleVersion(ma, mi, pa);
        return turbo::OkStatus();
    }

    std::string version_to_string(const EA::servlet::Version &v) {
        return turbo::Format("{}.{}.{}", v.major(), v.minor(), v.patch());
    }

    std::string module_version_to_string(const turbo::ModuleVersion &v) {
        return turbo::Format("{}.{}.{}", v.major, v.minor, v.patch);
    }

    static turbo::flat_hash_set<char> AllowChar{'a', 'b', 'c', 'd', 'e', 'f', 'g',
                                                'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                                'o', 'p', 'q', 'r', 's', 't',
                                                'u', 'v', 'w', 'x', 'y', 'z',
                                                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                                '_',
                                                'A', 'B', 'C', 'D', 'E', 'F', 'G',
                                                'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                                'O', 'P', 'Q', 'R', 'S', 'T',
                                                'U', 'V', 'Q', 'X', 'Y', 'Z',
    };

    turbo::Status check_valid_name_type(std::string_view ns) {
        int i = 0;
        for (auto c: ns) {
            if (AllowChar.find(c) == AllowChar.end()) {
                return turbo::InvalidArgumentError(
                        "the {} char {} of {} is not allow used in name the valid set is[a-z,A-Z,0-9,_]", i, c, ns);
            }
            ++i;
        }
        return turbo::OkStatus();
    }

}  // namespace EA::client
