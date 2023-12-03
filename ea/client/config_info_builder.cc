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

#include "ea/client/config_info_builder.h"
#include "ea/client/utility.h"
#include "turbo/files/sequential_read_file.h"
#include "ea/client/loader.h"

namespace EA::client {

    ConfigInfoBuilder::ConfigInfoBuilder(EA::discovery::ConfigInfo *info) : _info(info) {
        _info->Clear();
    }


    void ConfigInfoBuilder::set_info(EA::discovery::ConfigInfo *info) {
        _info = info;
        _info->Clear();
    }

    turbo::Status ConfigInfoBuilder::build_from_json(const std::string &json_str) {
        auto rs = Loader::load_proto(json_str, *_info);
        if (!rs.ok()) {
            return rs;
        }
        /// check field
        if (!_info->has_name() || _info->name().empty()) {
            return turbo::DataLossError("miss required field name");
        }
        if (!_info->has_version() ||
            (_info->version().major() == 0 && _info->version().minor() == 0 && _info->version().patch() == 0)) {
            return turbo::DataLossError("miss field version");
        }

        if (!_info->has_content() || _info->content().empty()) {
            return turbo::DataLossError("miss required field name");
        }
        return turbo::OkStatus();
    }

    turbo::Status ConfigInfoBuilder::build_from_json_file(const std::string &json_path) {
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

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const EA::discovery::Version &version,
                                                     const EA::discovery::ConfigType &type) {
        turbo::SequentialReadFile file;
        auto rs = file.open(file_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string content;
        auto frs = file.read(&content);
        if (!frs.ok()) {
            return frs.status();
        }

        return build_from_content(name, content, version, type);
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const EA::discovery::Version &version,
                                                     const std::string &type) {
        turbo::SequentialReadFile file;
        auto rs = file.open(file_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string content;
        auto frs = file.read(&content);
        if (!frs.ok()) {
            return frs.status();
        }
        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }

        return build_from_content(name, content, version, rt.value());
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const std::string &version,
                                                     const EA::discovery::ConfigType &type) {
        turbo::SequentialReadFile file;
        auto rs = file.open(file_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string content;
        auto frs = file.read(&content);
        if (!frs.ok()) {
            return frs.status();
        }

        EA::discovery::Version tmp_version;
        rs = string_to_version(version, &tmp_version);
        if (!rs.ok()) {
            return rs;
        }

        return build_from_content(name, content, tmp_version, type);
    }

    turbo::Status ConfigInfoBuilder::build_from_file(const std::string &name, const std::string &file_path,
                                                     const std::string &version,
                                                     const std::string &type) {
        turbo::SequentialReadFile file;
        auto rs = file.open(file_path);
        if (!rs.ok()) {
            return rs;
        }
        std::string content;
        auto frs = file.read(&content);
        if (!frs.ok()) {
            return frs.status();
        }

        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }

        EA::discovery::Version tmp_version;
        rs = string_to_version(version, &tmp_version);
        if (!rs.ok()) {
            return rs;
        }

        return build_from_content(name, content, tmp_version, rt.value());
    }

    turbo::Status ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                                        const EA::discovery::Version &version,
                                                        const EA::discovery::ConfigType &type) {
        _info->set_name(name);
        _info->set_content(content);
        *_info->mutable_version() = version;
        _info->set_type(type);
        return turbo::OkStatus();
    }

    turbo::Status ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                                        const EA::discovery::Version &version,
                                                        const std::string &type) {
        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }
        return build_from_content(name, content, version, rt.value());
    }

    turbo::Status ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                                        const std::string &version,
                                                        const EA::discovery::ConfigType &type) {
        EA::discovery::Version tmp_version;
        auto rs = string_to_version(version, &tmp_version);
        if (!rs.ok()) {
            return rs;
        }
        return build_from_content(name, content, tmp_version, type);
    }

    turbo::Status
    ConfigInfoBuilder::build_from_content(const std::string &name, const std::string &content,
                                          const std::string &version,
                                          const std::string &type) {
        auto rt = string_to_config_type(type);
        if (!rt.ok()) {
            return rt.status();
        }
        EA::discovery::Version tmp_version;
        auto rs = string_to_version(version, &tmp_version);
        if (!rs.ok()) {
            return rs;
        }
        return build_from_content(name, content, tmp_version, rt.value());

    }

}  // namespace EA::client
