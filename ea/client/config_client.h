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
//
// Created by jeff on 23-11-25.
//

#ifndef EA_CLIENT_CONFIG_CLIENT_H_
#define EA_CLIENT_CONFIG_CLIENT_H_

#include "turbo/container/flat_hash_map.h"
#include <map>
#include "turbo/base/status.h"
#include "turbo/module/module_version.h"
#include "eapi/servlet/servlet.interface.pb.h"
#include <functional>
#include "ea/client/base_message_sender.h"
#include "ea/client/meta.h"
#include "ea/base/bthread.h"

namespace EA::client {

    /**
     * @ingroup meta_client
     */
    struct ConfigCallbackData {
        std::string config_name;
        turbo::ModuleVersion current_version;
        turbo::ModuleVersion new_version;
        std::string new_content;
        std::string type;
    };

    /**
     *
     */
    typedef std::function<void(const ConfigCallbackData &config_name)> ConfigCallback;

    ///
    struct ConfigEventListener {
        ConfigCallback on_new_config;
        ConfigCallback on_new_version;
    };
    struct ConfigWatchEntity {
        turbo::ModuleVersion notice_version;
        ConfigEventListener listener;
    };

    class ConfigClient {
    public:
        static ConfigClient *get_instance() {
            static ConfigClient ins;
            return &ins;
        }

        /**
         *
         * @return
         */
        turbo::Status init();

        ///
        void stop();

        ///
        void join();

        /**
         *
         * @param config_name
         * @param version
         * @param content
         * @param type
         * @return
         */
        turbo::Status
        get_config(const std::string &config_name, const std::string &version, std::string &content,
                   std::string *type = nullptr);

        /**
         *
         * @param config_name
         * @param content
         * @param version
         * @param type
         * @return
         */
        turbo::Status get_config(const std::string &config_name, std::string &content, std::string *version = nullptr,
                                 std::string *type = nullptr);

        /**
         *
         * @param config_name
         * @param listener
         * @return
         */
        turbo::Status watch_config(const std::string &config_name, const ConfigEventListener &listener);

        /**
         *
         * @param config_name
         * @return
         */
        turbo::Status unwatch_config(const std::string &config_name);

        /**
         *
         * @param config_name
         * @return
         */
        turbo::Status remove_config(const std::string &config_name);

        /**
         *
         * @param config_name
         * @param version
         * @return
         */
        turbo::Status remove_config(const std::string &config_name, const std::string &version);

        /**
         *
         * @param config_name
         * @param version
         * @return
         */
        turbo::Status apply(const std::string &config_name, const turbo::ModuleVersion &version);

        /**
         *
         * @param config_name
         * @param version
         * @return
         */
        turbo::Status apply(const std::string &config_name, const std::string &version);

        /**
         *
         * @param config_name
         * @return
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
