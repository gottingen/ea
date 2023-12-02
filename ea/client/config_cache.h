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

#ifndef EA_CLIENT_CONFIG_CACHE_H_
#define EA_CLIENT_CONFIG_CACHE_H_

#include "turbo/container/flat_hash_map.h"
#include <map>
#include <mutex>
#include "turbo/base/status.h"
#include "turbo/module/module_version.h"
#include "eapi/servlet/servlet.struct.pb.h"
#include "ea/base/bthread.h"

namespace EA::client {

    /**
     * @ingroup config_client
     * @brief ConfigCache is used to cache the config files downloaded from the meta server.
     *        It is used by the MetaClient to cache the config files downloaded from the meta server.
     */
    class ConfigCache {
    public:
        static ConfigCache *get_instance() {
            static ConfigCache ins;
            return &ins;
        }

        /**
         * @brief init is used to initialize the ConfigCache. It must be called before using the ConfigCache.
         * @return Status::OK if the ConfigCache was initialized successfully. Otherwise, an error status is returned. 
         */ 
        turbo::Status init();

        /**
         * @brief add_config is used to add a config to the ConfigCache.
         * @param config [input] is the config to add to the ConfigCache.
         * @return Status::OK if the config was added successfully. Otherwise, an error status is returned. 
         */
        turbo::Status add_config(const EA::servlet::ConfigInfo &config);

        /**
         * @brief get_config is used to get a config that matches the name and version from the ConfigCache.
         * @param name [input] is the name of the config to get.
         * @param version [input] is the version of the config to get.
         * @param config [output] is the config received from the ConfigCache.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config(const std::string &name, const turbo::ModuleVersion &version, EA::servlet::ConfigInfo &config);

        /**
         * @brief get_config is used to get the latest version of a config from the ConfigCache.
         * @param name [input] is the name of the config to get the latest version for.
         * @param config is the config received from the ConfigCache.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status get_config(const std::string &name, EA::servlet::ConfigInfo &config);

        /**
         * @brief get_config_list is used to get a list of config names from the ConfigCache.
         * @param name [output] is the list of config names received from the ConfigCache.
         * @return Status::OK if the config names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status get_config_list(std::vector<std::string> &name);

        /**
         * @brief get_config_version_list is used to get a list of config versions from the ConfigCache.
         * @param config_name [input] is the name of the config to get the versions for.
         * @param versions [output] is the list of config versions received from the ConfigCache.
         * @return Status::OK if the config versions were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config_version_list(const std::string &config_name, std::vector<turbo::ModuleVersion> &versions);

        /**
         * @brief remove_config is used to remove a config from the ConfigCache.
         * @param config_name [input] is the name of the config to remove.
         * @param version [input] is the version of the config to remove.
         * @return Status::OK if the config names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status remove_config(const std::string &config_name, const turbo::ModuleVersion &version);

        /**
         * @brief remove_config is used to remove a config from the ConfigCache.
         * @param config_name [input] is the name of the config to remove.
         * @param version [input] is the version of the config to remove.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned.
         */
        turbo::Status remove_config(const std::string &config_name, const std::vector<turbo::ModuleVersion> &version);

        /**
         * @brief remove_config_less_than is used to remove a config from the ConfigCache. All versions less than the specified version will be removed.
         * @param config_name [input] is the name of the config to remove all versions less than.
         * @param version [input] is the version of the config to remove all versions less than.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned.
         */
        turbo::Status remove_config_less_than(const std::string &config_name, const turbo::ModuleVersion &version);

        /**
         * @brief remove_config is used to remove a config from the ConfigCache with all versions.
         * @param config_name [input] is the name of the config to remove.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned.
         */
        turbo::Status remove_config(const std::string &config_name);

    private:

        /**
         *
         * @param dir
         * @param config
         * @return
         */
        turbo::Status write_config_file(const std::string &dir, const EA::servlet::ConfigInfo &config);

        /**
         *
         * @param dir
         * @param config
         * @return
         */
        turbo::Status remove_config_file(const std::string &dir, const EA::servlet::ConfigInfo &config);

        /**
         *
         * @param dir
         * @param config
         * @return
         */
        std::string make_cache_file_path(const std::string &dir, const EA::servlet::ConfigInfo &config);

        /**
         *
         * @param config
         */
        void do_add_config(const EA::servlet::ConfigInfo &config);

    private:
        typedef turbo::flat_hash_map<std::string, std::map<turbo::ModuleVersion, EA::servlet::ConfigInfo>> CacheType;
        std::mutex _cache_mutex;
        CacheType _cache_map;
        std::string _cache_dir;
        bool        _init{false};
    };
}  // namespace EA::client

#endif  // EA_CLIENT_CONFIG_CACHE_H_
