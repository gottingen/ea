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


#include "ea/discovery/query_privilege_manager.h"
#include "turbo/strings/str_trim.h"


namespace EA::discovery {
    void QueryPrivilegeManager::get_user_info(const EA::discovery::DiscoveryQueryRequest *request,
                                              EA::discovery::DiscoveryQueryResponse *response) {
        PrivilegeManager *manager = PrivilegeManager::get_instance();
        BAIDU_SCOPED_LOCK(manager->_user_mutex);
        if (!request->has_user_name()) {
            for (auto &user_info: manager->_user_privilege) {
                auto privilege = response->add_user_privilege();
                *privilege = user_info.second;
            }
        } else {
            std::string user_name = request->user_name();
            if (manager->_user_privilege.find(user_name) != manager->_user_privilege.end()) {
                auto privilege = response->add_user_privilege();
                *privilege = manager->_user_privilege[user_name];
            } else {
                response->set_errcode(EA::discovery::INPUT_PARAM_ERROR);
                response->set_errmsg("username not exist");
            }
        }
    }

    void QueryPrivilegeManager::get_flatten_servlet_privilege(const EA::discovery::DiscoveryQueryRequest *request,
                                                      EA::discovery::DiscoveryQueryResponse *response) {
        PrivilegeManager *manager = PrivilegeManager::get_instance();
        BAIDU_SCOPED_LOCK(manager->_user_mutex);
        std::string user_name = request->user_name();
        turbo::Trim(&user_name);
        std::string namespace_name = request->namespace_name();
        turbo::Trim(&namespace_name);
        std::map<std::string, std::multimap<std::string, EA::discovery::QueryUserPrivilege>> namespace_privileges;
        if (user_name.empty() && namespace_name.empty()) {
            for (auto &privilege_info: manager->_user_privilege) {
                construct_query_response_for_servlet_privilege(privilege_info.second, namespace_privileges);
            }
        }
        if (!user_name.empty()
            && manager->_user_privilege.find(user_name) != manager->_user_privilege.end()) {
            construct_query_response_for_servlet_privilege(manager->_user_privilege[user_name], namespace_privileges);
        }
        if (!namespace_name.empty()) {
            for (auto &privilege_info: manager->_user_privilege) {
                if (privilege_info.second.namespace_name() != namespace_name) {
                    continue;
                }
                construct_query_response_for_servlet_privilege(privilege_info.second, namespace_privileges);
            }
        }
        for (auto &namespace_privilege: namespace_privileges) {
            for (auto &user_privilege: namespace_privilege.second) {
                EA::discovery::QueryUserPrivilege *privilege_info = response->add_flatten_privileges();
                *privilege_info = user_privilege.second;
            }
        }
    }


    void QueryPrivilegeManager::construct_query_response_for_servlet_privilege(const EA::discovery::UserPrivilege &user_privilege,
                                                                       std::map<std::string, std::multimap<std::string, EA::discovery::QueryUserPrivilege>> &namespace_privileges) {
        std::string namespace_name = user_privilege.namespace_name();
        std::string username = user_privilege.username();
        for (auto &privilege_zone: user_privilege.privilege_zone()) {
            EA::discovery::QueryUserPrivilege flatten_privilege;
            flatten_privilege.set_username(username);
            flatten_privilege.set_namespace_name(namespace_name);
            flatten_privilege.set_servlet_rw(privilege_zone.zone_rw());
            flatten_privilege.set_privilege(privilege_zone.zone() + ".*");
            std::multimap<std::string, EA::discovery::QueryUserPrivilege> user_privilege_map;
            if (namespace_privileges.find(namespace_name) != namespace_privileges.end()) {
                user_privilege_map = namespace_privileges[namespace_name];
            }
            user_privilege_map.insert(std::pair<std::string, EA::discovery::QueryUserPrivilege>(username, flatten_privilege));
            namespace_privileges[namespace_name] = user_privilege_map;
        }
        for (auto &privilege_servlet: user_privilege.privilege_servlet()) {
            EA::discovery::QueryUserPrivilege flatten_privilege;
            flatten_privilege.set_username(username);
            flatten_privilege.set_namespace_name(namespace_name);
            flatten_privilege.set_servlet_rw(privilege_servlet.servlet_rw());
            flatten_privilege.set_privilege(privilege_servlet.zone() + "." + privilege_servlet.servlet_name());
            std::multimap<std::string, EA::discovery::QueryUserPrivilege> user_privilege_map;
            if (namespace_privileges.find(namespace_name) != namespace_privileges.end()) {
                user_privilege_map = namespace_privileges[namespace_name];
            }
            user_privilege_map.insert(std::pair<std::string, EA::discovery::QueryUserPrivilege>(username, flatten_privilege));
            namespace_privileges[namespace_name] = user_privilege_map;
        }
    }

}  // namespace EA::discovery
