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

#include "ea/meta_server/privilege_manager.h"

namespace EA::servlet {
    class QueryPrivilegeManager {
    public:
        static QueryPrivilegeManager *get_instance() {
            static QueryPrivilegeManager instance;
            return &instance;
        }

        ~QueryPrivilegeManager() = default;

        ///
        /// \param request
        /// \param response
        void get_user_info(const EA::servlet::QueryRequest *request,
                           EA::servlet::QueryResponse *response);

        ///
        /// \param request
        /// \param response
        void
        get_flatten_servlet_privilege(const EA::servlet::QueryRequest *request, EA::servlet::QueryResponse *response);


    private:
        QueryPrivilegeManager() {}

        ///
        /// \param user_privilege
        /// \param namespace_privileges
        static void construct_query_response_for_servlet_privilege(const EA::servlet::UserPrivilege &user_privilege,
                                                            std::map<std::string, std::multimap<std::string, EA::servlet::QueryUserPrivilege>> &namespace_privileges);
    };
}  // namespace EA::servlet
