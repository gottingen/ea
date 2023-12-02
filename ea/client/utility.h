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
     * @ingroup ea_proto_g
     */

    /**
     *
     * @param type
     * @return
     */
    std::string config_type_to_string(EA::servlet::ConfigType type);

    /**
     *
     * @param str
     * @return
     */
    turbo::ResultStatus<EA::servlet::ConfigType> string_to_config_type(const std::string &str);

    /**
     *
     * @param str
     * @param v
     * @return
     */
    turbo::Status string_to_version(const std::string &str, EA::servlet::Version*v);

    /**
     *
     * @param str
     * @param v
     * @return
     */
    turbo::Status string_to_module_version(const std::string &str, turbo::ModuleVersion *v);

    /**
     *
     * @param v
     * @return
     */
    std::string version_to_string(const EA::servlet::Version &v);

    /**
     *
     * @param v
     * @return
     */
    std::string module_version_to_string(const turbo::ModuleVersion &v);

    /**
     *
     * @param ns
     * @return
     */
    [[nodiscard]] turbo::Status CheckValidNameType(std::string_view ns);

}  // namespace EA::client

#endif // EA_CLIENT_UTILITY_H_
