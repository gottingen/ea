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
// Created by jeff on 23-11-25.
//

#ifndef EA_CLIENT_CONFIG_CLIENT_H_
#define EA_CLIENT_CONFIG_CLIENT_H_

#include "turbo/container/flat_hash_map.h"
#include <map>
#include "turbo/base/status.h"
#include "turbo/module/module_version.h"
#include "eapi/discovery/discovery.interface.pb.h"
#include <functional>
#include "ea/client/base_message_sender.h"
#include "ea/client/discovery.h"
#include "ea/base/bthread.h"

namespace EA::client {

    /**
     * @ingroup config_client
     * @brief ConfigCallbackData is used to pass data to the ConfigCallback.
     */
    struct ConfigCallbackData {
        std::string config_name;
        turbo::ModuleVersion current_version;
        turbo::ModuleVersion new_version;
        std::string new_content;
        std::string type;
    };

    /**
     * @brief ConfigCallback is used to notify the user when a config is updated.
     */
    typedef std::function<void(const ConfigCallbackData &config_name)> ConfigCallback;

    
    /**
     * @brief ConfigEventListener is used to store the callback function for a config.
     */
    struct ConfigEventListener {
        ConfigCallback on_new_config;
        ConfigCallback on_new_version;
    };

    /**
     * @brief ConfigWatchEntity It is used by the ConfigClient to store the config watch information.      
    */
    struct ConfigWatchEntity {
        turbo::ModuleVersion notice_version;
        ConfigEventListener listener;
    };

    /**
     * @brief ConfigClient is used to download config files from the discovery server.
     *        It is used by the DiscoveryClient to download config files from the discovery server.
     */
    class ConfigClient {
    public:
        static ConfigClient *get_instance() {
            static ConfigClient ins;
            return &ins;
        }

        /**
         * @brief init is used to initialize the ConfigClient. It must be called before using the ConfigClient.
         * @return Status::OK if the ConfigClient was initialized successfully. Otherwise, an error status is returned. 
         */
        turbo::Status init();

        /**
         * @brief start is used to start the ConfigClient.
         * @return void
         * @note It must be called after init.
         */
        void stop();

        /**
         * @brief join is used to wait for the ConfigClient to stop.
         * @return void
         * @note It must be called after stop.
         */
        void join();

        /**
         * @brief get_config is used to get a config from the discovery server, and add it to the ConfigCache.
         * @param config_name [input] is the name of the config to get. it can not be empty.
         * @param version [input] is the version of the config to get. it can not be empty.
         * @param content [out] is the content of the config received from the discovery server.
         * @param type [out] if not null, it is the type of the config received from the discovery server.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config(const std::string &config_name, const std::string &version, std::string &content,
                   std::string *type = nullptr);

        /**
         * @brief get_config is used to get a config from the discovery server, and add it to the ConfigCache.
         * @param config_name [input] is the name of the config to get. it can not be empty.
         * @param content [out] is the content of the config received from the discovery server and added to the ConfigCache.
         * @param version [out] if not null, it is the version of the config received from the discovery server and added to the ConfigCache.
         * @param type [out] if not null, it is the type of the config received from the discovery server and added to the ConfigCache.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */ 
        turbo::Status get_config(const std::string &config_name, std::string &content, std::string *version = nullptr,
                                 std::string *type = nullptr);

        /**
         * @brief watch_config is used to watch a config. When the config is updated, the callback function will be called.
         * @param config_name [input] is the name of the config to watch. it can not be empty.
         * @param listener [input] is the callback function to call when the config is updated.
         * @return Status::OK if the config was watched successfully. Otherwise, an error status is returned. 
         */
        turbo::Status watch_config(const std::string &config_name, const ConfigEventListener &listener);

        /**
         * @brief unwatch_config is used to unwatch a config.
         * @param config_name [input] is the name of the config to unwatch. it can not be empty.
         * @return Status::OK if the config was unwatched successfully. Otherwise, an error status is returned. 
         */
        turbo::Status unwatch_config(const std::string &config_name);

        /**
         * @brief remove_config is used to remove a config from config cache.
         * @param config_name [input] is the name of the config to remove. it can not be empty.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned. 
         */ 
        turbo::Status remove_config(const std::string &config_name);


        /**
         * @brief remove_config is used to remove a config from config cache.
         * @param config_name [input] is the name of the config to remove. it can not be empty.
         * @param version [input] is the version of the config to remove. it can not be empty.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned. 
         */
        turbo::Status remove_config(const std::string &config_name, const std::string &version);

        /**
         * @brief apply is used to apply a config to the application.
         * @param config_name [input] is the name of the config to apply. it can not be empty.
         * @param version [input] is the version of the config to apply. it can not be empty.
         * @return Status::OK if the config was applied successfully. Otherwise, an error status is returned. 
         */
        turbo::Status apply(const std::string &config_name, const turbo::ModuleVersion &version);

        /**
         * @brief apply is used to apply a config to the application.
         * @param config_name [input] is the name of the config to apply. it can not be empty.
         * @param version [input] is the version of the config to apply. it can not be empty.
         * @return Status::OK if the config was applied successfully. Otherwise, an error status is returned. 
         */
        turbo::Status apply(const std::string &config_name, const std::string &version);

        /**
         * @brief unapply is used to unapply a config from the application.
         * @param config_name [input] is the name of the config to unapply. it can not be empty.
         * @return Status::OK if the config was unapplied successfully. Otherwise, an error status is returned. 
         */
        turbo::Status unapply(const std::string &config_name);

    private:
        ///
        void period_check();

        /**
         *
         * @param config_name
         * @param version
         * @return
         */
        turbo::Status do_apply(const std::string &config_name, const turbo::ModuleVersion &version);

        /**
         *
         * @param config_name
         * @return
         */
        turbo::Status do_unapply(const std::string &config_name);

        /**
         *
         * @param config_name
         * @return
         */
        turbo::Status do_unwatch_config(const std::string &config_name);
    private:
        turbo::flat_hash_map<std::string, turbo::ModuleVersion> _apply_version TURBO_GUARDED_BY(_watch_mutex);
        std::mutex _watch_mutex;
        turbo::flat_hash_map<std::string, ConfigWatchEntity> _watches TURBO_GUARDED_BY(_watch_mutex);
        EA::Bthread _bth;
        bool _shutdown{false};
        bool _init{false};
    };
}  // namespace EA::client

#endif  // EA_CLIENT_CONFIG_CLIENT_H_
