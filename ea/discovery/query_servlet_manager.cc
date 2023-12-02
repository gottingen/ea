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


#include "ea/discovery/query_servlet_manager.h"
#include "ea/base/tlog.h"

namespace EA::discovery {
    void QueryServletManager::get_servlet_info(const EA::discovery::DiscoveryQueryRequest *request,
                                                 EA::discovery::DiscoveryQueryResponse *response) {
        ServletManager *manager = ServletManager::get_instance();
        BAIDU_SCOPED_LOCK(manager->_servlet_mutex);
        if (!request->has_servlet()) {
            for (auto &servlet_info: manager->_servlet_info_map) {
                *(response->add_servlet_infos()) = servlet_info.second;
            }
        } else {
            std::string namespace_name = request->namespace_name();
            std::string zone = namespace_name + "\001" + request->zone();
            std::string servlet = zone + "\001" + request->servlet();
            if (manager->_servlet_id_map.find(servlet) != manager->_servlet_id_map.end()) {
                int64_t id = manager->_servlet_id_map[servlet];
                *(response->add_servlet_infos()) = manager->_servlet_info_map[id];
            } else {
                response->set_errmsg("servlet not exist");
                response->set_errcode(EA::discovery::INPUT_PARAM_ERROR);
                TLOG_ERROR("namespace: {} zone: {} servlet: {} not exist", namespace_name, zone, servlet);
            }
        }
    }

}  // namespace EA::discovery
