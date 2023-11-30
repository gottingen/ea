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


#include "ea/meta_server/query_namespace_manager.h"
#include "ea/base/tlog.h"

namespace EA::servlet {
    void QueryNamespaceManager::get_namespace_info(const EA::servlet::QueryRequest *request,
                                                   EA::servlet::QueryResponse *response) {
        NamespaceManager *manager = NamespaceManager::get_instance();
        BAIDU_SCOPED_LOCK(manager->_namespace_mutex);
        if (!request->has_namespace_name()) {
            for (auto &namespace_info: manager->_namespace_info_map) {
                *(response->add_namespace_infos()) = namespace_info.second;
            }
        } else {
            std::string namespace_name = request->namespace_name();
            if (manager->_namespace_id_map.find(namespace_name) != manager->_namespace_id_map.end()) {
                int64_t id = manager->_namespace_id_map[namespace_name];
                *(response->add_namespace_infos()) = manager->_namespace_info_map[id];
            } else {
                response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
                response->set_errmsg("namespace not exist");
                TLOG_ERROR("namespace: {}  not exist", namespace_name.c_str());
            }
        }
    }

}  // namespace EA::servlet
