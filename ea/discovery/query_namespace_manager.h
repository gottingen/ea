// Copyright 2023 The Elastic AI Search Authors.
//
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


#pragma once

#include "ea/discovery/namespace_manager.h"

namespace EA::discovery {
    class QueryNamespaceManager {
    public:
        ~QueryNamespaceManager() = default;

        static QueryNamespaceManager *get_instance() {
            static QueryNamespaceManager instance;
            return &instance;
        }

        ///
        /// \param request
        /// \param response
        void get_namespace_info(const EA::discovery::DiscoveryQueryRequest *request, EA::discovery::DiscoveryQueryResponse *response);

    private:
        QueryNamespaceManager() {}
    };
} // namespace EA::discovery
