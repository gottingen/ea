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

#include "eapi/discovery/discovery.interface.pb.h"
#include "ea/discovery/discovery_state_machine.h"

namespace EA::discovery {

    class DiscoveryManager {
    public:
        static DiscoveryManager *get_instance() {
            static DiscoveryManager instance;
            return &instance;
        }

        ~DiscoveryManager() {}

        ///
        /// \param controller
        /// \param request
        /// \param response
        /// \param done
        void process_schema_info(google::protobuf::RpcController *controller,
                                 const EA::discovery::DiscoveryManagerRequest *request,
                                 EA::discovery::DiscoveryManagerResponse *response,
                                 google::protobuf::Closure *done);

        ///
        /// \param user_privilege
        /// \return
        int check_and_get_for_privilege(EA::discovery::UserPrivilege &user_privilege);

        ///
        /// \param instance
        /// \return
        int check_and_get_for_instance(EA::discovery::ServletInstance &instance);

        int load_snapshot();

        ///
        /// \param discovery_state_machine
        void set_discovery_state_machine(DiscoveryStateMachine *discovery_state_machine) {
            _discovery_state_machine = discovery_state_machine;
        }

    private:
        DiscoveryManager() = default;

        int load_max_id_snapshot(const std::string &max_id_prefix,
                                 const std::string &key,
                                 const std::string &value);

        DiscoveryStateMachine *_discovery_state_machine;
    }; //class

}  // namespace EA::discovery
