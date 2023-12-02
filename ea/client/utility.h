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


#ifndef EA_CLIENT_UTILITY_H_
#define EA_CLIENT_UTILITY_H_

#include "turbo/base/status.h"
#include "turbo/base/result_status.h"
#include <string>
#include "eapi/servlet/servlet.interface.pb.h"
#include "turbo/module/module_version.h"

namespace EA::client {

    /**
     * @ingroup ea_proto
     * @brief config_type_to_string is used to convert a ConfigType to a string.
     * @param type [input] is the ConfigType to convert.
     * @return a string representation of the ConfigType.
     */
    std::string config_type_to_string(EA::servlet::ConfigType type);

    /**
     * @ingroup ea_proto
     * @brief string_to_config_type is used to convert a string to a ConfigType.
     * @param str [input] is the string to convert.
     * @return a ConfigType if the string is valid. Otherwise, an error status is returned.
     */
    turbo::ResultStatus<EA::servlet::ConfigType> string_to_config_type(const std::string &str);

    /**
     * @ingroup ea_proto
     * @brief string_to_version is used to convert a string to a Version.
     * @param str [input] is the string to convert.
     * @param v [output] is the Version received from the string.
     * @return Status::OK if the string was converted successfully. Otherwise, an error status is returned.
     */
    turbo::Status string_to_version(const std::string &str, EA::servlet::Version*v);

    /**
     * @ingroup ea_proto
     * @brief string_to_module_version is used to convert a string to a ModuleVersion.
     * @param str [input] is the string to convert.
     * @param v [output] is the ModuleVersion received from the string.
     * @return Status::OK if the string was converted successfully. Otherwise, an error status is returned.
     */
    turbo::Status string_to_module_version(const std::string &str, turbo::ModuleVersion *v);

    /**
     * @ingroup ea_proto
     * @brief version_to_string is used to convert a Version to a string.
     * @param v [input] is the Version to convert.
     * @return a string representation of the Version.
     */
    std::string version_to_string(const EA::servlet::Version &v);

    /**
     * @ingroup ea_proto
     * @brief module_version_to_string is used to convert a ModuleVersion to a string.
     * @param v [input] is the ModuleVersion to convert.
     * @return a string representation of the ModuleVersion.
     */
    std::string module_version_to_string(const turbo::ModuleVersion &v);


    /**
     * @ingroup ea_proto
     * @brief string_to_config_info is used to convert a string to a ConfigInfo.
     * @param name [input] is the name of the ConfigInfo to convert.
     * @return a ConfigInfo if the string is valid. Otherwise, an error status is returned.
     */
    [[nodiscard]] turbo::Status CheckValidNameType(std::string_view name);

}  // namespace EA::client

#endif // EA_CLIENT_UTILITY_H_
