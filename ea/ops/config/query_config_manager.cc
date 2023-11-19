// Copyright 2023 The Elastic-AI Authors.
// part of Elastic AI Search
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
#include "ea/ops/config/query_config_manager.h"
#include "ea/ops/config/config_manager.h"

namespace EA::config {

    void QueryConfigManager::get_config(const ::EA::proto::QueryOpsServiceRequest *request,
                                        ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        auto &get_request = request->query_config();
        auto &name = get_request.name();
        auto it = configs.find(name);
        if (it == configs.end() || it->second.empty()) {
            response->set_errmsg("config not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        turbo::ModuleVersion version;

        if (!get_request.has_version()) {
            // use newest
            // version = it->second.rend()->first;
            auto cit = it->second.rbegin();
            *(response->mutable_config_response()->mutable_config()) = cit->second;
            response->set_errmsg("success");
            response->set_errcode(proto::SUCCESS);
            return;
        }

        version = turbo::ModuleVersion(get_request.version().major(), get_request.version().minor(),
                                       get_request.version().patch());

        auto cit = it->second.find(version);
        if (cit == it->second.end()) {
            /// not exists
            response->set_errmsg("config not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }

        *(response->mutable_config_response()->mutable_config()) = cit->second;
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryConfigManager::list_config(const ::EA::proto::QueryOpsServiceRequest *request,
                                         ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        response->mutable_config_response()->mutable_config_list()->Reserve(configs.size());
        for (auto it = configs.begin(); it != configs.end(); ++it) {
            response->mutable_config_response()->add_config_list(it->first);
        }
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

    void QueryConfigManager::list_config_version(const ::EA::proto::QueryOpsServiceRequest *request,
                                                 ::EA::proto::QueryOpsServiceResponse *response) {
        response->set_op_type(request->op_type());
        auto &get_request = request->query_config();
        BAIDU_SCOPED_LOCK( ConfigManager::get_instance()->_config_mutex);
        auto configs = ConfigManager::get_instance()->_configs;
        auto &name = get_request.name();
        auto it = configs.find(name);
        if (it == configs.end()) {
            response->set_errmsg("config not exist");
            response->set_errcode(proto::INPUT_PARAM_ERROR);
            return;
        }
        response->mutable_config_response()->mutable_versions()->Reserve(it->second.size());
        for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
            *(response->mutable_config_response()->add_versions()) = vit->second.version();
        }
        response->set_errmsg("success");
        response->set_errcode(proto::SUCCESS);
    }

}  // namespace EA::config
