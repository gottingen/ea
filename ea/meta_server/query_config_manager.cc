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

#include "ea/meta_server/query_config_manager.h"
#include "ea/meta_server/config_manager.h"

namespace EA::servlet {

    void QueryConfigManager::get_config(const ::EA::servlet::QueryRequest *request,
                                        ::EA::servlet::QueryResponse *response) {
        if (!request->has_config_name()) {
            response->set_errmsg("config name not set");
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            return;
        }
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        auto &name = request->config_name();
        auto it = configs.find(name);
        if (it == configs.end() || it->second.empty()) {
            response->set_errmsg("config not exist");
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            return;
        }
        turbo::ModuleVersion version;

        if (!request->has_config_version()) {
            // use newest
            // version = it->second.rend()->first;
            auto cit = it->second.rbegin();
            *(response->add_config_infos()) = cit->second;
            response->set_errmsg("success");
            response->set_errcode(EA::servlet::SUCCESS);
            return;
        }
        auto &request_version = request->config_version();
        version = turbo::ModuleVersion(request_version.major(), request_version.minor(), request_version.patch());

        auto cit = it->second.find(version);
        if (cit == it->second.end()) {
            /// not exists
            response->set_errmsg("config not exist");
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            return;
        }

        *(response->add_config_infos()) = cit->second;
        response->set_errmsg("success");
        response->set_errcode(EA::servlet::SUCCESS);
    }

    void QueryConfigManager::list_config(const ::EA::servlet::QueryRequest *request,
                                         ::EA::servlet::QueryResponse *response) {
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        response->mutable_config_infos()->Reserve(configs.size());
        EA::servlet::ConfigInfo config;
        for (auto it = configs.begin(); it != configs.end(); ++it) {
            config.set_name(it->first);
            *(response->add_config_infos()) = config;
        }
        response->set_errmsg("success");
        response->set_errcode(EA::servlet::SUCCESS);
    }

    void QueryConfigManager::list_config_version(const ::EA::servlet::QueryRequest *request,
                                                 ::EA::servlet::QueryResponse *response) {
        if (!request->has_config_name()) {
            response->set_errmsg("config name not set");
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            return;
        }
        auto &name = request->config_name();
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        auto it = configs.find(name);
        if (it == configs.end()) {
            response->set_errmsg("config not exist");
            response->set_errcode(EA::servlet::INPUT_PARAM_ERROR);
            return;
        }
        response->mutable_config_infos()->Reserve(it->second.size());
        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            *(response->add_config_infos()) = vit->second;
        }
        response->set_errmsg("success");
        response->set_errcode(EA::servlet::SUCCESS);
    }

}  // namespace EA::servlet
