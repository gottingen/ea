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


#include "ea/client/config_client.h"
#include "ea/client/utility.h"
#include "ea/client/config_cache.h"
#include "ea/flags/client.h"

namespace EA::client {

    turbo::Status ConfigClient::init() {
        if(_init) {
            return turbo::OkStatus();
        }
        auto rs = ConfigCache::get_instance()->init();
        if (!rs.ok()) {
            TLOG_ERROR("config cache init error:{}", rs.message());
            return rs;
        }

        _shutdown = false;
        _bth.run([this] {
            period_check();
        });
        _init = true;
        return turbo::OkStatus();
    }

    void ConfigClient::stop() {
        _shutdown = true;
    }

    ///
    void ConfigClient::join() {
        _bth.join();
    }

    turbo::Status
    ConfigClient::get_config(const std::string &config_name, const std::string &version, std::string &content,
                             std::string *type) {
        turbo::ModuleVersion mv;
        auto rs = string_to_module_version(version, &mv);
        if (!rs.ok()) {
            return rs;
        }
        EA::servlet::ConfigInfo config_pb;
        rs = ConfigCache::get_instance()->get_config(config_name, mv, config_pb);
        if (rs.ok()) {
            content = config_pb.content();
            if (type) {
                *type = config_type_to_string(config_pb.type());
            }
            return turbo::OkStatus();
        }

        rs = MetaClient::get_instance()->get_config(config_name, version, config_pb);
        if (!rs.ok()) {
            return rs;
        }
        content = config_pb.content();
        if (type) {
            *type = config_type_to_string(config_pb.type());
        }
        rs = ConfigCache::get_instance()->add_config(config_pb);
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::get_config(const std::string &config_name, std::string &content, std::string *version,
                                           std::string *type) {
        turbo::ModuleVersion mv;
        EA::servlet::ConfigInfo config_pb;
        auto rs = ConfigCache::get_instance()->get_config(config_name, config_pb);
        if (rs.ok()) {
            content = config_pb.content();
            if (type) {
                *type = config_type_to_string(config_pb.type());
            }
            if (version) {
                *version = version_to_string(config_pb.version());
            }
            return turbo::OkStatus();
        }

        rs = MetaClient::get_instance()->get_config_latest(config_name, config_pb);
        if (!rs.ok()) {
            return rs;
        }
        content = config_pb.content();
        if (type) {
            *type = config_type_to_string(config_pb.type());
        }
        if (version) {
            *version = version_to_string(config_pb.version());
        }
        rs = ConfigCache::get_instance()->add_config(config_pb);
        return rs;
    }

    turbo::Status ConfigClient::watch_config(const std::string &config_name, const ConfigEventListener &listener) {
        turbo::ModuleVersion module_version;
        std::unique_lock lock(_watch_mutex);
        if (_watches.find(config_name) != _watches.end()) {
            return turbo::AlreadyExistsError("");
        }
        auto ait = _apply_version.find(config_name);
        if (ait != _apply_version.end()) {
            module_version = ait->second;
        }
        _watches[config_name] = ConfigWatchEntity{module_version, listener};
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::unwatch_config(const std::string &config_name) {
        std::unique_lock lock(_watch_mutex);
        return do_unwatch_config(config_name);
    }

    turbo::Status ConfigClient::do_unwatch_config(const std::string &config_name) {
        if (_watches.find(config_name) == _watches.end()) {
            return turbo::NotFoundError("");
        }
        _watches.erase(config_name);
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::remove_config(const std::string &config_name) {
        return ConfigCache::get_instance()->remove_config(config_name);
    }

    turbo::Status ConfigClient::remove_config(const std::string &config_name, const std::string &version) {
        turbo::ModuleVersion module_version;
        auto rs = string_to_module_version(version, &module_version);
        if (!rs.ok()) {
            return rs;
        }
        return ConfigCache::get_instance()->remove_config(config_name, module_version);
    }

    turbo::Status ConfigClient::apply(const std::string &config_name, const turbo::ModuleVersion &version) {
        std::unique_lock lock(_watch_mutex);
        return do_apply(config_name, version);
    }

    turbo::Status ConfigClient::apply(const std::string &config_name, const std::string &version) {
        turbo::ModuleVersion mv;
        auto rs = string_to_module_version(version, &mv);
        if (!rs.ok()) {
            return rs;
        }
        return apply(config_name, mv);
    }

    turbo::Status ConfigClient::unapply(const std::string &config_name) {
        std::unique_lock lock(_watch_mutex);
        auto rs = do_unwatch_config(config_name);
        TURBO_UNUSED(rs);
        return do_unapply(config_name);
    }

    turbo::Status ConfigClient::do_unapply(const std::string &config_name) {
        auto it = _apply_version.find(config_name);
        if (it == _apply_version.end()) {
            return turbo::NotFoundError("not found config:{}", config_name);
        }
        _apply_version.erase(config_name);
        return turbo::OkStatus();
    }

    turbo::Status ConfigClient::do_apply(const std::string &config_name, const turbo::ModuleVersion &version) {
        _apply_version[config_name] = version;
        return turbo::OkStatus();
    }

    void ConfigClient::period_check() {
        static turbo::ModuleVersion kZero;
        std::vector<std::pair<std::string, turbo::ModuleVersion>> updates;
        turbo::flat_hash_map<std::string, ConfigWatchEntity> watches;
        int sleep_step_us = FLAGS_config_watch_interval_ms * 1000;
        int sleep_round = FLAGS_config_watch_interval_round_s * 1000 * 1000;
        TLOG_INFO("start config watch background");
        while(!_shutdown) {
            updates.clear();
            watches.clear();
            {
                std::unique_lock lock(_watch_mutex);
                watches = _watches;
            }
            TLOG_INFO("new round watch size:{}", watches.size());
            for(auto &it : watches) {
                EA::servlet::ConfigInfo info;
                turbo::ModuleVersion current_version = it.second.notice_version;
                auto rs = MetaClient::get_instance()->get_config_latest(it.first, info);
                if(!rs.ok()) {
                    TLOG_WARN_IF(kZero != it.second.notice_version, "get config fail:{}", rs.message());
                    continue;
                }
                TLOG_INFO("get config {} version:{}.{}.{}",info.name(),info.version().major(), info.version().minor(), info.version().patch());
                rs = ConfigCache::get_instance()->add_config(info);
                if(!rs.ok() && !turbo::IsAlreadyExists(rs)) {
                    TLOG_WARN("add config to cache fail:{}", rs.message());
                }
                turbo::ModuleVersion new_view(info.version().major(), info.version().minor(), info.version().patch());
                if(current_version == kZero) {
                    if(it.second.listener.on_new_config) {
                        TLOG_INFO("call new config callback:{}",info.name());
                        ConfigCallbackData data{info.name(), kZero,new_view,info.content(), config_type_to_string(info.type())};
                        it.second.listener.on_new_config(data);
                    } else {
                        TLOG_INFO("call new config callback:{} but no call backer",info.name());
                    }
                } else if(current_version < new_view) {
                    if(it.second.listener.on_new_version) {
                        TLOG_INFO("call new config version, callback:{}",info.name());
                        ConfigCallbackData data{info.name(), kZero,new_view, info.content(), config_type_to_string(info.type())};
                        it.second.listener.on_new_version(data);
                    } else {
                        TLOG_INFO("call new config callback:{} but no call backer",info.name());
                    }
                }
                updates.emplace_back(info.name(), new_view);
                bthread_usleep(sleep_step_us);
            }

            {
                std::unique_lock lock(_watch_mutex);
                for(auto &item : updates) {
                    auto it = _watches.find(item.first);
                    if(it == _watches.end()) {
                        continue;
                    }
                    it->second.notice_version = item.second;
                }
            }
            bthread_usleep(sleep_round);

        }
        TLOG_INFO("config watch background stop...");
    }
}  // namespace EA::client

