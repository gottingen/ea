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

    /**
     * @ingroup ea_proto
     * @brief ConfigInfoBuilder is helper class for build ConfigInfo object,
     *        it does not hold the ConfigInfo object memory, and not thread safe. user should guard
     *        that the object is usable and make sure it is thread safe call. eg.
     * @code
     *        EA::servlet::ConfigInfo info;
     *        ConfigInfoBuilder builder(&info);
     *        std::string content = "listen_port=8010;raft_group=meta_raft";
     *        auto status = builder.build_from_content("meta_config", content, "1.2.3", "json");
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class ConfigInfoBuilder {
    public:
        ConfigInfoBuilder() = default;

        ~ConfigInfoBuilder() = default;

        explicit ConfigInfoBuilder(EA::servlet::ConfigInfo *info);

        /**
         * @brief set up ConfigInfo pointer for building
         * @param info
         */
        void set_info(EA::servlet::ConfigInfo *info);

        /**
         * @brief load ConfigInfo from json format string.
         * @param json_str json format string
         * @return
         */
        turbo::Status build_from_json(const std::string &json_str);

        /**
         * @brief load ConfigInfo from json format string that read from file.
         * @param json_path file path of json string
         * @return status.ok() if success else return the reason of parse fail.
         */
        turbo::Status build_from_json_file(const std::string &json_path);

        /**
         * @brief build ConfigInfo by parameters, the config store in the file
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of parse fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const EA::servlet::Version &version,
                                      const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);

        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const EA::servlet::Version &version,
                                      const std::string &type = "json");

        /**
         *  @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const std::string &version,
                                      const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);

        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param file [input] config content data file.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_file(const std::string &name, const std::string &file, const std::string &version,
                                      const std::string &type = "json");

        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_content(const std::string &name, const std::string &content, const EA::servlet::Version &version,
                                         const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);
        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_content(const std::string &name, const std::string &content, const EA::servlet::Version &version,
                                         const std::string &type = "json");
        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [CF_JSON|CF_GFLAGS|CF_TEXT|CF_TOML|CF_XML|CF_YAML|CF_INI].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status build_from_content(const std::string &name, const std::string &content, const std::string &version,
                                         const EA::servlet::ConfigType &type = EA::servlet::CF_JSON);
        /**
         * @brief build ConfigInfo by parameters, the config store in content
         * @param name [input] config name
         * @param content [input] config content.
         * @param version [input] config version, format is major.minor.patch eg "1.2.3"
         * @param type [input] config type, can be any one of this [json|toml|yaml|xml|gflags|text|ini].
         * @return status.ok() if success else return the reason of build fail.
         */
        turbo::Status
        build_from_content(const std::string &name, const std::string &content, const std::string &version,
                           const std::string &type = "json");

    private:
        EA::servlet::ConfigInfo *_info{nullptr};
    };
}  // namespace client

#endif  // EA_CLIENT_CONFIG_INFO_BUILDER_H_
