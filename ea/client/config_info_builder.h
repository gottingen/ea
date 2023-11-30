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


#ifndef EA_CLIENT_CONFIG_INFO_BUILDER_H_
#define EA_CLIENT_CONFIG_INFO_BUILDER_H_

#include "eapi/servlet/servlet.struct.pb.h"
#include "turbo/base/status.h"

namespace EA::client {

    class ConfigInfoBuilder {
    public:
        ConfigInfoBuilder() = default;

        ~ConfigInfoBuilder() = default;

        explicit ConfigInfoBuilder(EA::servlet::ConfigInfo *info);


        void set_info(EA::servlet::ConfigInfo *info);

        ///
        /// \param json_str
        /// \return
        turbo::Status build_from_json(const std::string &json_str);

        ///
        /// \param json_path
        /// \return
        turbo::Status build_from_json_file(const std::string &json_path);

        ///
        /// \param name
        /// \param file
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_file(const std::string &name, const std::string &file, const EA::servlet::Version &version,
                                      const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);

        ///
        /// \param name
        /// \param file
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_file(const std::string &name, const std::string &file, const EA::servlet::Version &version,
                                      const std::string &type = "json");

        ///
        /// \param name
        /// \param file
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_file(const std::string &name, const std::string &file, const std::string &version,
                                      const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);

        ///
        /// \param name
        /// \param file
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_file(const std::string &name, const std::string &file, const std::string &version,
                                      const std::string &type = "json");

        ///
        /// \param name
        /// \param content
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_content(const std::string &name, const std::string &content, const EA::servlet::Version &version,
                                         const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);
        ///
        /// \param name
        /// \param content
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_content(const std::string &name, const std::string &content, const EA::servlet::Version &version,
                                         const std::string &type = "json");
        ///
        /// \param name
        /// \param content
        /// \param version
        /// \param type
        /// \return
        turbo::Status build_from_content(const std::string &name, const std::string &content, const std::string &version,
                                         const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);
        ///
        /// \param name
        /// \param content
        /// \param version
        /// \param type
        /// \return
        turbo::Status
        build_from_content(const std::string &name, const std::string &content, const std::string &version,
                           const std::string &type = "json");

    private:
        EA::servlet::ConfigInfo *_info{nullptr};
    };
}  // namespace client

#endif  // EA_CLIENT_CONFIG_INFO_BUILDER_H_
