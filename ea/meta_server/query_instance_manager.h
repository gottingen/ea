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

//
// Created by jeff on 23-11-29.
//

#ifndef EA_META_SERVER_QUERY_INSTANCE_MANAGER_H_
#define EA_META_SERVER_QUERY_INSTANCE_MANAGER_H_

#include "ea/meta_server/instance_manager.h"

namespace EA::servlet {

    class QueryInstanceManager {
    public:
        static QueryInstanceManager *get_instance() {
            static QueryInstanceManager ins;
            return &ins;
        }

        ~QueryInstanceManager() = default;

        ///
        /// \param request
        /// \param response
        void query_instance(const EA::servlet::QueryRequest *request, EA::servlet::QueryResponse *response);

        ///
        /// \param request
        /// \param response
        void query_instance_flatten(const EA::servlet::QueryRequest *request, EA::servlet::QueryResponse *response);

    public:
        static void instance_info_to_query(const EA::servlet::ServletInstance &sinstance, EA::servlet::QueryInstance &ins);
    };
}  // namespace EA::servlet

#endif  // EA_META_SERVER_QUERY_INSTANCE_MANAGER_H_
