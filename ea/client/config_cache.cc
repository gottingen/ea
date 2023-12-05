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


#include "ea/client/config_cache.h"
#include "ea/flags/client.h"
#include "ea/client/utility.h"
#include "turbo/files/filesystem.h"
#include "turbo/files/sequential_write_file.h"
#include "turbo/files/sequential_read_file.h"
#include "ea/client/loader.h"
#include "ea/client/dumper.h"

namespace EA::client {

    turbo::Status ConfigCache::init() {
        if(_init) {
            return turbo::ok_status();
        }
        _cache_dir = EA::FLAGS_config_cache_dir;
        if(_cache_dir.empty()) {
            return turbo::ok_status();
        }
        std::error_code ec;
        if(!turbo::filesystem::exists(_cache_dir, ec)) {
            if(ec) {
                return turbo::unknown_error(ec.message());
            }
            turbo::filesystem::create_directories(_cache_dir);
            return turbo::ok_status();
        }
        turbo::filesystem::directory_iterator dir_itr(_cache_dir);
        turbo::filesystem::directory_iterator end;
        for(;dir_itr != end; ++dir_itr) {
            if(dir_itr->path() == "." || dir_itr->path() == "..") {
                continue;
            }
            auto file_path = dir_itr->path().string();
            EA::discovery::ConfigInfo info;
            auto rs = Loader::load_proto_from_file(file_path, info);
            if(!rs.ok()) {
                return rs;
            }
            do_add_config(info);
            TLOG_INFO("loading config cache file:{}", file_path);
        }
        _init = true;
        return turbo::ok_status();
    }
    turbo::Status ConfigCache::add_config(const EA::discovery::ConfigInfo &config) {
        {
            std::unique_lock lock(_cache_mutex);
            if (_cache_map.find(config.name()) == _cache_map.end()) {
                _cache_map[config.name()] = std::map<turbo::ModuleVersion, EA::discovery::ConfigInfo>();
            }
            auto it = _cache_map.find(config.name());
            auto version = turbo::ModuleVersion(config.version().major(), config.version().minor(),
                                                config.version().patch());
            if (it->second.find(version) != it->second.end()) {
                return turbo::already_exists_error("");
            }
            it->second[version] = config;
        }
        return write_config_file(_cache_dir, config);
    }

    void ConfigCache::do_add_config(const EA::discovery::ConfigInfo &config) {
            if (_cache_map.find(config.name()) == _cache_map.end()) {
                _cache_map[config.name()] = std::map<turbo::ModuleVersion, EA::discovery::ConfigInfo>();
            }
            auto it = _cache_map.find(config.name());
            auto version = turbo::ModuleVersion(config.version().major(), config.version().minor(),
                                                config.version().patch());
            it->second[version] = config;
    }

    turbo::Status ConfigCache::get_config(const std::string &name, const turbo::ModuleVersion &version,
                                          EA::discovery::ConfigInfo &config) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config.name());
        if (it != _cache_map.end()) {
            auto vit = it->second.find(version);
            if (vit != it->second.end()) {
                config = vit->second;
                return turbo::ok_status();
            }
        }
        return turbo::not_found_error("");
    }

    ///
    /// \brief get latest version of config
    /// \param name
    /// \param config
    /// \return
    turbo::Status ConfigCache::get_config(const std::string &name, EA::discovery::ConfigInfo &config) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config.name());
        if (it != _cache_map.end()) {
            auto vit = it->second.rbegin();
            if (vit != it->second.rend()) {
                config = vit->second;
                return turbo::ok_status();
            }
        }
        return turbo::not_found_error("");
    }

    ///
    /// \param name
    /// \return
    turbo::Status ConfigCache::get_config_list(std::vector<std::string> &configs) {
        std::unique_lock lock(_cache_mutex);
        for (auto it = _cache_map.begin(); it != _cache_map.end(); ++it) {
            configs.push_back(it->first);
        }
        return turbo::ok_status();
    }

    ///
    /// \param config_name
    /// \param versions
    /// \return
    turbo::Status
    ConfigCache::get_config_version_list(const std::string &config_name, std::vector<turbo::ModuleVersion> &versions) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            for (auto vit = it->second.begin(); vit != it->second.end(); ++vit) {
                versions.push_back(vit->first);
            }
            return turbo::ok_status();
        }
        return turbo::not_found_error("");
    }

    ///
    /// \brief remove signal version of config
    /// \param config_name
    /// \param version
    /// \return
    turbo::Status ConfigCache::remove_config(const std::string &config_name, const turbo::ModuleVersion &version) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            auto vit = it->second.find(version);
            if (vit != it->second.end()) {
                auto rs = remove_config_file(_cache_dir, vit->second);
                TURBO_UNUSED(rs);
                it->second.erase(vit);
                if (it->second.empty()) {
                    _cache_map.erase(it);
                }
                return turbo::ok_status();
            }
        }
        return turbo::not_found_error("");
    }

    ///
    /// \param config_name
    /// \param version
    /// \return
    turbo::Status
    ConfigCache::remove_config(const std::string &config_name, const std::vector<turbo::ModuleVersion> &versions) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            for (auto &version: versions) {
                auto vit = it->second.find(version);
                if (vit != it->second.end()) {
                    auto rs = remove_config_file(_cache_dir, vit->second);
                    TURBO_UNUSED(rs);
                    it->second.erase(vit);
                }
            }
            if (it->second.empty()) {
                _cache_map.erase(it);
            }
            return turbo::ok_status();
        }
        return turbo::not_found_error("");
    }

    ///
    /// \brief remove config versions less than the given version
    /// \param config_name
    /// \param version
    /// \return
    turbo::Status
    ConfigCache::remove_config_less_than(const std::string &config_name, const turbo::ModuleVersion &version) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            auto vit = it->second.lower_bound(version);
            for (auto rit = it->second.begin(); rit != vit; ++rit) {
                auto rs = remove_config_file(_cache_dir, rit->second);
                TURBO_UNUSED(rs);
            }
            it->second.erase(it->second.begin(), vit);
            if (it->second.empty()) {
                _cache_map.erase(it);
            }
            return turbo::ok_status();
        }
        return turbo::not_found_error("");
    }

    ///
    /// \brief remove all version of config
    /// \param config_name
    /// \return
    turbo::Status ConfigCache::remove_config(const std::string &config_name) {
        std::unique_lock lock(_cache_mutex);
        auto it = _cache_map.find(config_name);
        if (it != _cache_map.end()) {
            for (auto &vit: it->second) {
                auto rs = remove_config_file(_cache_dir, vit.second);
                TURBO_UNUSED(rs);
            }
            _cache_map.erase(it);
            return turbo::ok_status();
        }
        return turbo::not_found_error("");
    }

    turbo::Status ConfigCache::write_config_file(const std::string &dir, const EA::discovery::ConfigInfo &config) {
        if (dir.empty()) {
            return turbo::ok_status();
        }
        auto file_path = make_cache_file_path(dir, config);
        return EA::client::Dumper::dump_proto_to_file(file_path, config);
    }

    turbo::Status ConfigCache::remove_config_file(const std::string &dir, const EA::discovery::ConfigInfo &config) {
        if (dir.empty()) {
            return turbo::ok_status();
        }
        auto file_path = make_cache_file_path(dir, config);
        turbo::filesystem::remove(file_path);
        return turbo::ok_status();
    }

    std::string ConfigCache::make_cache_file_path(const std::string &dir, const EA::discovery::ConfigInfo &config) {
        return turbo::Format("{}/{}-{}.{}.{}.{}", dir,
                             config.name(),
                             config.version().minor(),
                             config.version().minor(),
                             config.version().patch(),
                             config_type_to_string(config.type()));
    }
}  // namespace EA::client