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
#include "ea/base/proto_help.h"
#include "turbo/strings/utility.h"

namespace EA {

    std::string config_type_to_string(EA::proto::ConfigType type) {
        switch (type) {
            case EA::proto::CF_JSON:
                return "json";
            case EA::proto::CF_TEXT:
                return "text";
            case EA::proto::CF_INI:
                return "ini";
            case EA::proto::CF_YAML:
                return "yaml";
            case EA::proto::CF_XML:
                return "xml";
            case EA::proto::CF_GFLAGS:
                return "gflags";
            case EA::proto::CF_TOML:
                return "toml";
            default:
                return "unknown format";
        }
    }

    turbo::ResultStatus<EA::proto::ConfigType> string_to_config_type(const std::string &str) {
        auto lc = turbo::StrToLower(str);
        if (lc == "json") {
            return EA::proto::CF_JSON;
        } else if (lc == "text") {
            return EA::proto::CF_TEXT;
        } else if (lc == "ini") {
            return EA::proto::CF_INI;
        } else if (lc == "yaml") {
            return EA::proto::CF_YAML;
        } else if (lc == "xml") {
            return EA::proto::CF_XML;
        } else if (lc == "gflags") {
            return EA::proto::CF_GFLAGS;
        } else if (lc == "toml") {
            return EA::proto::CF_TOML;
        }
        return turbo::InvalidArgumentError("unknown format '{}'", str);
    }

    std::string platform_to_string(EA::proto::Platform type) {
        switch (type) {
            case EA::proto::PF_lINUX:
                return "linux";
            case EA::proto::PF_OSX:
                return "osx";
            case EA::proto::PF_WINDOWS:
                return "windows";
            default:
                return "unknown format";
        }
    }

    turbo::ResultStatus<EA::proto::Platform> string_to_platform(const std::string &str) {
        auto lc = turbo::StrToLower(str);
        if (str == "linux") {
            return EA::proto::PF_lINUX;
        } else if (str == "osx") {
            return EA::proto::PF_OSX;
        } else if (str == "windows") {
            return EA::proto::PF_WINDOWS;
        }
        return turbo::InvalidArgumentError("unknown platform '{}'", str);
    }

    std::string get_op_string(EA::proto::OpType type) {
        switch (type) {
            case EA::proto::OP_CREATE_CONFIG:
                return "create config";
            case EA::proto::OP_REMOVE_CONFIG:
                return "remove config";
            case EA::proto::OP_CREATE_PLUGIN:
                return "create plugin";
            case EA::proto::OP_REMOVE_PLUGIN:
                return "remove plugin";
            case EA::proto::OP_RESTORE_TOMBSTONE_PLUGIN:
                return "restore plugin";
            case EA::proto::OP_REMOVE_TOMBSTONE_PLUGIN:
                return "remove tombstone plugin";
            case EA::proto::OP_UPLOAD_PLUGIN:
                return "upload plugin";
            case EA::proto::OP_CREATE_DICT:
                return "create dict";
            case EA::proto::OP_REMOVE_DICT:
                return "remove dict";
            case EA::proto::OP_RESTORE_TOMBSTONE_DICT:
                return "restore dict";
            case EA::proto::OP_REMOVE_TOMBSTONE_DICT:
                return "remove tombstone dict";
            case EA::proto::OP_UPLOAD_DICT:
                return "upload dict";
            default:
                return "unknown operation";
        }
    }

    std::string get_op_string(EA::proto::QueryOpType type) {
        switch (type) {
            case EA::proto::QUERY_LIST_CONFIG_VERSION:
                return "list config version";
            case EA::proto::QUERY_LIST_CONFIG:
                return "list config";
            case EA::proto::QUERY_GET_CONFIG:
                return "get config";
            case EA::proto::QUERY_PLUGIN_INFO:
                return "plugin info";
            case EA::proto::QUERY_LIST_PLUGIN:
                return "plugin list";
            case EA::proto::QUERY_LIST_PLUGIN_VERSION:
                return "list plugin version";
            case EA::proto::QUERY_TOMBSTONE_PLUGIN_INFO:
                return "tombstone plugin info";
            case EA::proto::QUERY_TOMBSTONE_LIST_PLUGIN:
                return "tombstone list plugin";
            case EA::proto::QUERY_TOMBSTONE_LIST_PLUGIN_VERSION:
                return "tombstone list plugin version";
            case EA::proto::QUERY_DOWNLOAD_PLUGIN:
                return "download plugin";
            case EA::proto::QUERY_INFO_DICT:
                return "dict info";
            case EA::proto::QUERY_LIST_DICT:
                return "dict list";
            case EA::proto::QUERY_LIST_DICT_VERSION:
                return "list dict version";
            case EA::proto::QUERY_TOMBSTONE_DICT_INFO:
                return "tombstone dict info";
            case EA::proto::QUERY_TOMBSTONE_LIST_DICT:
                return "tombstone list dict";
            case EA::proto::QUERY_TOMBSTONE_LIST_DICT_VERSION:
                return "tombstone list dict version";
            case EA::proto::QUERY_DOWNLOAD_DICT:
                return "download dict";
            default:
                return "unknown operation";
        }
    }

    turbo::Status string_to_version(const std::string &str, EA::proto::Version *v) {
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

    std::string version_to_string(const EA::proto::Version &v) {
        return turbo::Format("{}.{}.{}", v.major(), v.minor(), v.patch());
    }

    std::string make_plugin_filename(const std::string &name, const EA::proto::Version &version,
                                     EA::proto::Platform platform) {
        if (platform == EA::proto::PF_lINUX) {
            return turbo::Format("lib{}.so.{}.{}.{}", name, version.major(), version.minor(), version.patch());
        } else if (platform == EA::proto::PF_OSX) {
            return turbo::Format("lib{}.{}.{}.{}.dylib", name, version.major(), version.minor(), version.patch());
        } else {
            return turbo::Format("lib{}.{}.{}.{}.dll", name, version.major(), version.minor(), version.patch());
        }

    }

}  // namespace EA::rpc
