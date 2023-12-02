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

#include <unordered_map>
#include <bthread/mutex.h>
#include "eapi/discovery/discovery.interface.pb.h"
#include "ea/discovery/discovery_state_machine.h"
#include "ea/discovery/discovery_constants.h"

namespace EA::discovery {
    class PrivilegeManager {
    public:
        friend class QueryPrivilegeManager;

        ///
        /// \return
        static PrivilegeManager *get_instance() {
            static PrivilegeManager instance;
            return &instance;
        }

        ~PrivilegeManager() {
            bthread_mutex_destroy(&_user_mutex);
        }

        ///
        /// \brief privilege proxy called by discovery state machine
        /// \param controller
        /// \param request
        /// \param response
        /// \param done
        void process_user_privilege(google::protobuf::RpcController *controller,
                                    const EA::discovery::DiscoveryManagerRequest *request,
                                    EA::discovery::DiscoveryManagerResponse *response,
                                    google::protobuf::Closure *done);

        ///
        /// \brief create a user
        /// \param request
        /// \param done
        void create_user(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief remove user privilege for namespace db and table
        /// \param request
        /// \param done
        void drop_user(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief add privilege for user, user should be created
        /// \param request
        /// \param done
        void add_privilege(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \brief drop privilege for user, user should be created
        /// \param request
        /// \param done
        void drop_privilege(const EA::discovery::DiscoveryManagerRequest &request, braft::Closure *done);

        ///
        /// \return
        int load_snapshot();

        ///
        /// \param discovery_state_machine
        void set_discovery_state_machine(DiscoveryStateMachine *discovery_state_machine) {
            _discovery_state_machine = discovery_state_machine;
        }

    private:
        PrivilegeManager() {
            bthread_mutex_init(&_user_mutex, nullptr);
        }

        ///
        /// \param username
        /// \return
        static std::string construct_privilege_key(const std::string &username) {
            return DiscoveryConstants::PRIVILEGE_IDENTIFY + username;
        }

        ///
        /// \param privilege_zone
        /// \param mem_privilege
        static void insert_zone_privilege(const EA::discovery::PrivilegeZone &privilege_zone,
                                       EA::discovery::UserPrivilege &mem_privilege);

        ///
        /// \param privilege_servlet
        /// \param mem_privilege
        static void insert_servlet_privilege(const EA::discovery::PrivilegeServlet &privilege_servlet,
                                   EA::discovery::UserPrivilege &mem_privilege);

        ///
        /// \param ip
        /// \param mem_privilege
        static void insert_ip(const std::string &ip, EA::discovery::UserPrivilege &mem_privilege);

        ///
        /// \param privilege_zone
        /// \param mem_privilege
        static void delete_zone_privilege(const EA::discovery::PrivilegeZone &privilege_zone,
                                       EA::discovery::UserPrivilege &mem_privilege);
        ///
        /// \param privilege_servlet
        /// \param mem_privilege
        static void delete_servlet_privilege(const EA::discovery::PrivilegeServlet &privilege_servlet,
                                    EA::discovery::UserPrivilege &mem_privilege);

        ///
        /// \param ip
        /// \param mem_privilege
        static void delete_ip(const std::string &ip, EA::discovery::UserPrivilege &mem_privilege);

        bthread_mutex_t _user_mutex;
        std::unordered_map<std::string, EA::discovery::UserPrivilege> _user_privilege;

        DiscoveryStateMachine *_discovery_state_machine;
    };//class
}  // namespace EA::discovery
