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
#include "ea/discovery/query_instance_manager.h"

namespace EA::discovery {

    void QueryInstanceManager::query_instance(const EA::discovery::DiscoveryQueryRequest *request, EA::discovery::DiscoveryQueryResponse *response) {
        if(!request->has_instance_address()) {
            response->set_errcode(EA::INPUT_PARAM_ERROR);
            response->set_errmsg("no instance address");
            return;
        }
        auto manager = InstanceManager::get_instance();
        BAIDU_SCOPED_LOCK(manager->_instance_mutex);
        auto it = manager->_instance_info.find(request->instance_address());
        if(it == manager->_instance_info.end()) {
            response->set_errcode(EA::INPUT_PARAM_ERROR);
            response->set_errmsg("instance not exists");
            return;
        }
        *response->add_instance() = it->second;
        response->set_errcode(EA::SUCCESS);
        response->set_errmsg("success");
    }

    void QueryInstanceManager::query_instance_flatten(const EA::discovery::DiscoveryQueryRequest *request, EA::discovery::DiscoveryQueryResponse *response) {
        auto manager = InstanceManager::get_instance();
        if(!request->has_namespace_name() || request->namespace_name().empty()) {
            BAIDU_SCOPED_LOCK(manager->_instance_mutex);
            for(auto &it : manager->_instance_info) {
                EA::discovery::QueryInstance ins;
                instance_info_to_query(it.second, ins);
                *response->add_flatten_instances() = std::move(ins);
            }
            response->set_errcode(EA::SUCCESS);
            response->set_errmsg("success");
            return;
        }
        if(!request->has_zone() || request->zone().empty()) {
            BAIDU_SCOPED_LOCK(manager->_instance_mutex);
            auto it = manager->_namespace_instance.find(request->namespace_name());
            if(it == manager->_namespace_instance.end()) {
                response->set_errcode(EA::INPUT_PARAM_ERROR);
                auto msg = turbo::format("no instance in namespace {}", request->namespace_name());
                response->set_errmsg(msg);
                return;
            }
            for(auto address : it->second) {
                EA::discovery::QueryInstance ins;
                auto &info = manager->_instance_info[address];
                instance_info_to_query(info, ins);
                *response->add_flatten_instances() = std::move(ins);
            }
            response->set_errcode(EA::SUCCESS);
            response->set_errmsg("success");
            return;
        }
        if(!request->has_servlet() || request->servlet().empty()) {
            BAIDU_SCOPED_LOCK(manager->_instance_mutex);
            auto zone_key = ZoneManager::make_zone_key(request->namespace_name(), request->zone());
            auto it = manager->_zone_instance.find(zone_key);
            if(it == manager->_zone_instance.end()) {
                response->set_errcode(EA::INPUT_PARAM_ERROR);
                auto msg = turbo::format("no instance in namespace {}.{}", request->namespace_name(), request->zone());
                response->set_errmsg(msg);
                return;
            }
            for(auto address : it->second) {
                EA::discovery::QueryInstance ins;
                auto &info = manager->_instance_info[address];
                instance_info_to_query(info, ins);
                *response->add_flatten_instances() = std::move(ins);
            }
            response->set_errcode(EA::SUCCESS);
            response->set_errmsg("success");
            return;
        }

        BAIDU_SCOPED_LOCK(manager->_instance_mutex);
        auto servlet_key = ServletManager::make_servlet_key(request->namespace_name(), request->zone(), request->servlet());
        auto it = manager->_servlet_instance.find(servlet_key);
        if(it == manager->_servlet_instance.end()) {
            response->set_errcode(EA::INPUT_PARAM_ERROR);
            auto msg = turbo::format("no instance in {}.{}.{}", request->namespace_name(), request->zone(), request->servlet());
            response->set_errmsg(msg);
            return;
        }
        for(auto address : it->second) {
            EA::discovery::QueryInstance ins;
            auto &info = manager->_instance_info[address];
            instance_info_to_query(info, ins);
            *response->add_flatten_instances() = std::move(ins);
        }
        response->set_errcode(EA::SUCCESS);
        response->set_errmsg("success");
    }

    void QueryInstanceManager::instance_info_to_query(const EA::discovery::ServletInstance &sinstance, EA::discovery::QueryInstance &ins) {
        ins.set_namespace_name(sinstance.namespace_name());
        ins.set_zone_name(sinstance.zone_name());
        ins.set_servlet_name(sinstance.servlet_name());
        ins.set_env(sinstance.env());
        ins.set_color(sinstance.color());
        ins.set_version(sinstance.version());
        ins.set_status(sinstance.status());
        ins.set_address(sinstance.address());
    }
}  // namespace EA::discovery
