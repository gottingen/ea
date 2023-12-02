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


#ifndef EA_DISCOVERY_QUERY_CONFIG_MANAGER_H_
#define EA_DISCOVERY_QUERY_CONFIG_MANAGER_H_

#include "eapi/discovery/discovery.interface.pb.h"

namespace EA::discovery {

    class QueryConfigManager {
    public:
        ///
        /// \return
        static QueryConfigManager *get_instance() {
            static QueryConfigManager ins;
            return &ins;
        }

        ///
        /// \param request
        /// \param response
        void get_config(const ::EA::discovery::DiscoveryQueryRequest *request, ::EA::discovery::DiscoveryQueryResponse *response);

        ///
        /// \param request
        /// \param response
        void list_config(const ::EA::discovery::DiscoveryQueryRequest *request, ::EA::discovery::DiscoveryQueryResponse *response);

        ///
        /// \param request
        /// \param response
        void list_config_version(const ::EA::discovery::DiscoveryQueryRequest *request,
                                 ::EA::discovery::DiscoveryQueryResponse *response);
    };
}  // namespace EA::discovery

#endif  // EA_DISCOVERY_QUERY_CONFIG_MANAGER_H_
