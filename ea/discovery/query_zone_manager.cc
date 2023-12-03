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


#include "ea/discovery/query_zone_manager.h"
#include "ea/base/tlog.h"

namespace EA::discovery {
    void QueryZoneManager::get_zone_info(const EA::discovery::DiscoveryQueryRequest *request,
                                                 EA::discovery::DiscoveryQueryResponse *response) {
        ZoneManager *manager = ZoneManager::get_instance();
        BAIDU_SCOPED_LOCK(manager->_zone_mutex);
        if (!request->has_zone()) {
            for (auto &zone_info: manager->_zone_info_map) {
                *(response->add_zone_infos()) = zone_info.second;
            }
        } else {
            std::string namespace_name = request->namespace_name();
            std::string zone = namespace_name + "\001" + request->zone();
            if (manager->_zone_id_map.find(zone) != manager->_zone_id_map.end()) {
                int64_t id = manager->_zone_id_map[zone];
                *(response->add_zone_infos()) = manager->_zone_info_map[id];
            } else {
                response->set_errmsg("zone not exist");
                response->set_errcode(EA::INPUT_PARAM_ERROR);
                TLOG_ERROR("namespace: {} zone: {} not exist", namespace_name, zone);
            }
        }
    }

}  // namespace EA::discovery
