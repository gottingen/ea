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


#ifndef EA_CLIENT_DISCOVERY_H_
#define EA_CLIENT_DISCOVERY_H_

#include "turbo/base/status.h"
#include <butil/endpoint.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <brpc/controller.h>
#include <string>
#include "ea/base/tlog.h"
#include "ea/flags/discovery.h"
#include <google/protobuf/descriptor.h>
#include "eapi/discovery/discovery.interface.pb.h"
#include "turbo/module/module_version.h"
#include "ea/client/base_message_sender.h"


namespace EA::client {

    /**
     * @ingroup discovery_client
     * @brief DiscoveryClient is used by the ConfigClient to communicate with the discovery server by sender.
     *       It does not support asynchronous calls. run it in a bthread by yourself. farther more, it is not thread safe.
     *       It does not hold the ownership of the sender. The sender must be valid during the lifetime of the DiscoveryClient.
     *       It is a proxy interface of the discovery server.
     * @attention before using the DiscoveryClient, you must call init to initialize it
     * @code
     *      BaseMessageSender *sender = new RouterSender();
     *      DiscoveryClient::get_instance()->init(sender);
     *      auto rs = DiscoveryClient::get_instance()->create_config("test", "test", "1.0.0");
     *      if (!rs.ok()) {
     *          TLOG_ERROR("create config error:{}", rs.message());
     *          return -1;
     *      }
     *      std::vector<std::string> configs;
     *      rs = DiscoveryClient::get_instance()->list_config(configs);
     *      if (!rs.ok()) {
     *          TLOG_ERROR("list config error:{}", rs.message());
     *          return rs;
     *      }
     *      for (auto &config : configs) {
     *          TLOG_INFO("config:{}", config);
     *          std::vector<std::string> versions;
     *          rs = DiscoveryClient::get_instance()->list_config_version(config, versions);
     *          if (!rs.ok()) {
     *               TLOG_ERROR("list config version error:{}", rs.message());
     *               return rs;
     *           }
     *      }
     *      ...
     *      delete sender;
     *      return 0;
     * @endcode
     */
    class DiscoveryClient {
    public:
        static DiscoveryClient *get_instance() {
            static DiscoveryClient ins;
            return &ins;
        }

        DiscoveryClient() = default;

    public:

        /**
         * @brief init is used to initialize the DiscoveryClient. It must be called before using the DiscoveryClient.
         * @param sender [input] is the sender used to communicate with the discovery server, it can be a RouterSender or a ConfigSender.
         * @return Status::OK if the DiscoveryClient was initialized successfully. Otherwise, an error status is returned.
         */         
        turbo::Status init(BaseMessageSender *sender);


        static turbo::Status check_config(const std::string &json_content);

        /**
         *
         * @param config_path
         * @return
         */
        static turbo::Status check_config_file(const std::string &config_path);

        /**
         * @brief dump_config_file is used to dump a ConfigInfo to a file.
         * @param config_path [input] is the path of the file to dump the ConfigInfo to.
         * @param config [input] is the ConfigInfo to dump.
         * @return Status::OK if the ConfigInfo was dumped successfully. Otherwise, an error status is returned. 
         */
        static turbo::Status dump_config_file(const std::string &config_path, const EA::discovery::ConfigInfo &config);

        /**
         * @brief create_config is used to create a config by parameters, it is a synchronous call.
         * @param config_name [input] is the name of the config to create.
         * @param content [input] is the content of the config to create.
         * @param version [input] is the version of the config to create.
         * @param config_type [input] is the type of the config to create default is json.
         * @param retry_time [input] is the retry times of the create config.
         * @return Status::OK if the config was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_config(const std::string &config_name,
                                    const std::string &content,
                                    const std::string &version,
                                    const std::string &config_type = "json",
                                    int *retry_time = nullptr);

        /**
         * @brief create_config is used to create a config by ConfigInfo, it is a synchronous call.
         * @param request [input] is the ConfigInfo to create.
         * @param retry_times [input] is the retry times of the create config.
         * @return Status::OK if the config was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_config(const EA::discovery::ConfigInfo &request, int *retry_times = nullptr);


        /**
         * @brief create_config_by_json is used to create a config by json string, it is a synchronous call.
         * @param config_name [input] is the name of the config to create.
         * @param path [input] is the path of the json file to read the config from.
         * @param config_type [input] is the type of the config to create default is json.
         * @param version [input] is the version of the config to create.
         * @param retry_time [input] is the retry times of the create config.
         * @return Status::OK if the config was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_config_by_file(const std::string &config_name,
                                            const std::string &path,
                                            const std::string &config_type = "json", const std::string &version = "",
                                            int *retry_time = nullptr);

        /**
         * @brief create_config_by_json is used to create a config by json string, it is a synchronous call.
         * @param json_path [input] is the path of the json file to read the config from.
         * @param retry_time [input] is the retry times of the create config.
         * @return Status::OK if the config was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_config_by_json(const std::string &json_path, int *retry_time = nullptr);

        /**
         * @brief list_config is used to list all config names from the discovery server, it is a synchronous call.
         * @param configs [output] is the config names received from the discovery server.
         * @param retry_time [input] is the retry times of the list config.
         * @return Status::OK if the config names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_config(std::vector<std::string> &configs, int *retry_time = nullptr);

        /**
         * @brief list_config_version is used to list all config versions of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to list the versions for.
         * @param versions [output] is the config versions received from the discovery server.
         * @param retry_time [input] is the retry times of the list config version.
         * @return Status::OK if the config versions were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_config_version(const std::string &config_name, std::vector<std::string> &versions,
                                          int *retry_time = nullptr);

        /**
         * @brief list_config_version is used to list all config versions of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to list the versions for.
         * @param versions [output] is the config versions received from the discovery server.
         * @param retry_time [input] is the retry times of the list config version.
         * @return Status::OK if the config versions were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_config_version(const std::string &config_name, std::vector<turbo::ModuleVersion> &versions,
                                          int *retry_time = nullptr);

        /**
         * @brief get_config is used to get a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get.
         * @param version [input] is the version of the config to get.
         * @param config [output] is the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config(const std::string &config_name, const std::string &version, EA::discovery::ConfigInfo &config,
                   int *retry_time = nullptr);

        /**
         * @brief get_config is used to get a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get.
         * @param version [input] is the version of the config to get.
         * @param config [output] is the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @param type [output] is the type of the config received from the discovery server.
         * @param time [output] is the time of the config received from the discovery server.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status get_config(const std::string &config_name, const std::string &version, std::string &config,
                                 int *retry_time = nullptr,
                                 std::string *type = nullptr, uint32_t *time = nullptr);

        /**
         * @brief save_config is used to save a config to local file, it is a synchronous call.
         * @param config_name [input] is the name of the config to save.
         * @param version [input] is the version of the config to save.
         * @param path [input] is the path of the file to save the config to.
         * @param retry_time [input] is the retry times of the save config.
         * @return Status::OK if the config was saved successfully. Otherwise, an error status is returned. 
         */
        turbo::Status save_config(const std::string &config_name, const std::string &version, std::string &path,
                                  int *retry_time = nullptr);

        /**
         * @brief save_config is used to save a config to local file, it is a synchronous call.
         * @param config_name [input] is the name of the config to save.
         * @param version [input] is the version of the config to save.
         * @param retry_time [input] is the retry times of the save config.
         * @return Status::OK if the config was saved successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        save_config(const std::string &config_name, const std::string &version, int *retry_time = nullptr);

        /**
         * @brief get_config is used to get the latest version of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param config [output] is the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config_latest(const std::string &config_name, EA::discovery::ConfigInfo &config,
                          int *retry_time = nullptr);

        /**
         * @brief get_config is used to get the latest version of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param config [output] is the config received from the discovery server.
         * @param version [output] is the version of the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config_latest(const std::string &config_name, std::string &config, std::string &version,
                          int *retry_time = nullptr);

        /**
         * @brief get_config is used to get the latest version of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param version [output] is the version of the config received from the discovery server.
         * @param config [output] is the config received from the discovery server.
         * @param type [output] is the type of the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config_latest(const std::string &config_name, std::string &config, std::string &version, std::string &type,
                          int *retry_time = nullptr);

        /**
         * @brief get_config is used to get the latest version of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param config [output] is the config received from the discovery server.
         * @param version [output] is the version of the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config_latest(const std::string &config_name, std::string &config, turbo::ModuleVersion &version,
                          int *retry_time = nullptr);

        /**
         * @brief get_config is used to get the latest version of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param config [output] is the config received from the discovery server.
         * @param version [output] is the version of the config received from the discovery server.
         * @param type [output] is the type of the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned.
         */
        turbo::Status
        get_config_latest(const std::string &config_name, std::string &config, turbo::ModuleVersion &version,
                          std::string &type,
                          int *retry_time = nullptr);

        /**
         * @brief get_config is used to get the latest version of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param config [output] is the config received from the discovery server.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_config_latest(const std::string &config_name, std::string &config, int *retry_time = nullptr);

        /**
         * @brief remove_config is used to remove a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to get the latest version for.
         * @param version [input] is the version of the config to remove.
         * @param retry_time [input] is the retry times of the get config.
         * @return Status::OK if the config was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        remove_config(const std::string &config_name, const std::string &version, int *retry_time = nullptr);

        /**
         * @brief remove_config is used to remove a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to remove.
         * @param version [input] is the version of the config to remove.
         * @param retry_time [input] is the retry times of the remove config.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        remove_config(const std::string &config_name, const turbo::ModuleVersion &version, int *retry_time = nullptr);

        /**
         * @brief remove_config_all_version is used to remove all versions of a config from the discovery server, it is a synchronous call.
         * @param config_name [input] is the name of the config to remove all versions for.
         * @param retry_time [input] is the retry times of the remove config.
         * @return Status::OK if the config was removed successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        remove_config_all_version(const std::string &config_name, int *retry_time = nullptr);

        /**
         * @brief create_namespace is used to create a namespace by NameSpaceInfo, it is a synchronous call.
         * @param info [input] is the NameSpaceInfo to create.
         * @param retry_time [input] is the retry times of the create namespace.
         * @return Status::OK if the namespace was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_namespace(EA::discovery::NameSpaceInfo &info, int *retry_time = nullptr);

   
        /**
         * @brief create_namespace is used to create a namespace by parameters, it is a synchronous call.
         * @param ns [input] is the name of the namespace to create.
         * @param quota [input] is the quota of the namespace to create.
         * @param retry_time [input] is the retry times of the create namespace.
         * @return Status::OK if the namespace was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_namespace(const std::string &ns, int64_t quota = 0, int *retry_time = nullptr);

        /**
         * @brief create_namespace_by_json is used to create a namespace by json string, it is a synchronous call.
         * @param json_str [input] is the json string to create the namespace from, it contains the namespace name, quota and status.
         * @param retry_time [input] is the retry times of the create namespace.
         * @return Status::OK if the namespace was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_namespace_by_json(const std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief create_namespace_by_file is used to create a namespace by json file, it is a synchronous call.
         * @param path [input] is the path of the json file to read the namespace from.
         * @param retry_time [input] is the retry times of the create namespace.
         * @return Status::OK if the namespace was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_namespace_by_file(const std::string &path, int *retry_time = nullptr);

        /**
         * remove_namespace is used to remove a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the name of the namespace to remove.
         * @param retry_time [input] is the retry times of the remove namespace.
         * @return Status::OK if the namespace was removed successfully. Otherwise, an error status is returned. 
         */
        turbo::Status remove_namespace(const std::string &ns, int *retry_time = nullptr);

        /**
         * @brief modify_namespace is used to modify a namespace by parameters, it is a synchronous call.
         * @param ns_info [input] is the NameSpaceInfo to modify.
         * @param retry_time [input] is the retry times of the modify namespace.
         * @return Status::OK if the namespace was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_namespace(EA::discovery::NameSpaceInfo &ns_info, int *retry_time = nullptr);

        /**
         * @brief modify_namespace_by_json is used to modify a namespace by json string, it is a synchronous call.
         * @param json_str [input] is the json string to modify the namespace from, it contains the namespace name, quota and status.
         * @param retry_time [input] is the retry times of the modify namespace.
         * @return Status::OK if the namespace was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_namespace_by_json(const std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief modify_namespace_by_file is used to modify a namespace by json file, it is a synchronous call.
         * @param path [input] is the path of the json file to read the namespace from.
         * @param retry_time [input] is the retry times of the modify namespace.
         * @return Status::OK if the namespace was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_namespace_by_file(const std::string &path, int *retry_time = nullptr);

        /**
         * @brief list_namespace is used to list all namespace names from the discovery server, it is a synchronous call.
         * @param ns_list [output] is the namespace names received from the discovery server.
         * @param retry_time [input] is the retry times of the list namespace.
         * @return Status::OK if the namespace names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_namespace(std::vector<std::string> &ns_list, int *retry_time = nullptr);

        /**
         * @brief list_namespace is used to list all namespace names from the discovery server, it is a synchronous call.
         * @param ns_list [output] is the namespace names received from the discovery server.
         * @param retry_time [input] is the retry times of the list namespace.
         * @return Status::OK if the namespace names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_namespace(std::vector<EA::discovery::NameSpaceInfo> &ns_list, int *retry_time = nullptr);

        /**
         * @brief list_namespace_to_json is used to list all namespace names from the discovery server, it is a synchronous call.
         * @param ns_list [output] is the namespace names received from the discovery server.
         * @param retry_time [input] is the retry times of the list namespace.
         * @return Status::OK if the namespace names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_namespace_to_json(std::vector<std::string> &ns_list, int *retry_time = nullptr);

        /**
         * @brief list_namespace_to_file is used to list all namespace names from the discovery server, it is a synchronous call.
         * @param save_path [input] is the path of the file to save the namespace names to.
         * @param retry_time [input] is the retry times of the list namespace.
         * @return Status::OK if the namespace names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_namespace_to_file(const std::string &save_path, int *retry_time = nullptr);

        /**
         * @brief get_namespace is used to get a namespace from the discovery server, it is a synchronous call.
         * @param ns_name [input] is the name of the namespace to get.
         * @param ns_pb [output] is the namespace received from the discovery server.
         * @param retry_time [input] is the retry times of the get namespace.
         * @return Status::OK if the namespace was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_namespace(const std::string &ns_name, EA::discovery::NameSpaceInfo &ns_pb, int *retry_time = nullptr);

        /**
         * @brief get_namespace is used to get a namespace from the discovery server, it is a synchronous call.
         * @param ns_name [input] is the name of the namespace to get.
         * @param json_str [output] is the namespace received from the discovery server.
         * @param retry_time [input] is the retry times of the get namespace.
         * @return Status::OK if the namespace was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status get_namespace_json(const std::string &ns_name, std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief save_namespace_json is used to save a namespace to local file, it is a synchronous call.
         * @param ns_name [input] is the name of the namespace to save.
         * @param json_path [input] is the path of the file to save the namespace to.
         * @param retry_time [input] is the retry times of the save namespace.
         * @return Status::OK if the namespace was saved successfully. Otherwise, an error status is returned.
         */
        turbo::Status
        save_namespace_json(const std::string &ns_name, const std::string &json_path, int *retry_time = nullptr);

        /**
         * @brief create_zone is used to create a zone by ZoneInfo, it is a synchronous call.
         * @param info [input] is the ZoneInfo to create.
         * @param retry_time [input] is the retry times of the create zone.
         * @return Status::OK if the zone was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_zone(EA::discovery::ZoneInfo &info, int *retry_time = nullptr);

        /**
         * @brief create_zone is used to create a zone by parameters, it is a synchronous call.
         * @param ns [input] is the namespace name of the zone to create.
         * @param zone [input] is the zone name of the zone to create.
         * @param quota [input] is the quota of the zone to create.
         * @param retry_time [input] is the retry times of the create zone.
         * @return Status::OK if the zone was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        create_zone(const std::string &ns, const std::string &zone, int64_t quota = 0, int *retry_time = nullptr);

        /**
         * @brief create_zone_by_json is used to create a zone by json string, it is a synchronous call.
         * @param json_str [input] is the json string to create the zone from, it contains the zone name, namespace name, quota and status.
         * @param retry_time [input] is the retry times of the create zone.
         * @return Status::OK if the zone was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_zone_by_json(const std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief create_zone_by_file is used to create a zone by json file, it is a synchronous call.
         * @param path [input] is the path of the json file to read the zone from.
         * @param retry_time [input] is the retry times of the create zone.
         * @return Status::OK if the zone was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_zone_by_file(const std::string &path, int *retry_time = nullptr);

        /**
         * @brief remove_zone is used to remove a zone from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the zone to remove.
         * @param zone [input] is the zone name of the zone to remove.
         * @param retry_time [input] is the retry times of the remove zone.
         * @return Status::OK if the zone was removed successfully. Otherwise, an error status is returned. 
         */
        turbo::Status remove_zone(const std::string &ns, const std::string &zone, int *retry_time = nullptr);

        /**
         * @brief modify_zone is used to modify a zone by parameters, it is a synchronous call.
         * @param zone_info [input] is the ZoneInfo to modify.
         * @param retry_time [input] is the retry times of the modify zone.
         * @return Status::OK if the zone was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_zone(EA::discovery::ZoneInfo &zone_info, int *retry_time = nullptr);

        /**
         * @brief modify_zone_by_json is used to modify a zone by json string, it is a synchronous call.
         * @param json_str [input] is the json string to modify the zone from, it contains the zone name, namespace name, quota and status.
         * @param retry_time [input] is the retry times of the modify zone.
         * @return Status::OK if the zone was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_zone_by_json(const std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief modify_zone_by_file is used to modify a zone by json file, it is a synchronous call.
         * @param path [input] is the path of the json file to read the zone from.
         * @param retry_time [input] is the retry times of the modify zone.
         * @return Status::OK if the zone was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_zone_by_file(const std::string &path, int *retry_time = nullptr);

        /**
         * @brief list_zone is used to list all zone names from the discovery server, it is a synchronous call.
         * @param zone_list [output] is the zone names received from the discovery server.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zone names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_zone(std::vector<EA::discovery::ZoneInfo> &zone_list, int *retry_time = nullptr);

        /**
         * @brief list_zone is used to list all zone names of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the zones to list.
         * @param zone_list [output] is the zone names received from the discovery server.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zone names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_zone(const std::string &ns, std::vector<EA::discovery::ZoneInfo> &zone_list, int *retry_time = nullptr);

        /**
         * @brief list_zone is used to list all zone names from the discovery server, it is a synchronous call.
         * @param zone_list [output] is the zone names received from the discovery server.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zone names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_zone(std::vector<std::string> &zone_list, int *retry_time = nullptr);

        /**
         * @brief list_zone is used to list all zone names of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the zones to list.
         * @param zone_list [output] is the zone names received from the discovery server.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zone names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_zone(std::string &ns, std::vector<std::string> &zone_list, int *retry_time = nullptr);

        /**
         * @brief list_zone_to_json is used to list all zone names from the discovery server, it is a synchronous call.
         * @param zone_list [output] is the zone names received from the discovery server.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zone names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_zone_to_json(std::vector<std::string> &zone_list, int *retry_time = nullptr);

        /**
         * @brief list_zone_to_json is used to list all zone names of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the zones to list.
         * @param zone_list [output] is the zone names received from the discovery server.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zone names were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_zone_to_json(const std::string &ns, std::vector<std::string> &zone_list, int *retry_time = nullptr);

        /**
         * @brief list_zone_to_file is used to list all zone names from the discovery server, it is a synchronous call.
         * @param save_path [input] is the path of the file to save the zones to.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zones were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_zone_to_file(const std::string &save_path, int *retry_time = nullptr);

        /**
         * @brief list_zone_to_file is used to list all zone names of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the zones to list.
         * @param save_path [input] is the path of the file to save the zones to.
         * @param retry_time [input] is the retry times of the list zone.
         * @return Status::OK if the zones were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_zone_to_file(const std::string &ns, const std::string &save_path, int *retry_time = nullptr);

        /**
         * @brief get_zone is used to get a zone from the discovery server, it is a synchronous call.
         * @param ns_name [input] is the namespace name of the zone to get.
         * @param zone_name [input] is the zone name of the zone to get.
         * @param zone_pb [output] is the zone received from the discovery server.
         * @param retry_time [input] is the retry times of the get zone.
         * @return Status::OK if the zone was received successfully. Otherwise, an error status is returned. 
         */     
        turbo::Status get_zone(const std::string &ns_name, const std::string &zone_name, EA::discovery::ZoneInfo &zone_pb,
                               int *retry_time = nullptr);

        /**
         * @brief get_zone_json is used to get a zone from the discovery server, it is a synchronous call.
         * @param ns_name [input] is the namespace name of the zone to get.
         * @param zone_name [input] is the zone name of the zone to get.
         * @param json_str [output] is the zone received from the discovery server.
         * @param retry_time [input] is the retry times of the get zone.
         * @return Status::OK if the zone was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status get_zone_json(const std::string &ns_name, const std::string &zone_name, std::string &json_str,
                                    int *retry_time = nullptr);

        /**
         * @brief save_zone_json is used to save a zone to local file, it is a synchronous call.
         * @param ns_name [input] is the namespace name of the zone to save.
         * @param zone_name [input] is the zone name of the zone to save.
         * @param json_path [input] is the path of the file to save the zone to.
         * @param retry_time [input] is the retry times of the save zone.
         * @return Status::OK if the zone was saved successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        save_zone_json(const std::string &ns_name, const std::string &zone_name, const std::string &json_path,
                       int *retry_time = nullptr);

        /**
         * @brief create_servlet is used to create a servlet by parameters, it is a synchronous call.
         * @param servlet_info [input] is the servlet to create.
         * @param retry_time [input] is the retry times of the create servlet.
         * @return Status::OK if the servlet was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_servlet(EA::discovery::ServletInfo &servlet_info, int *retry_time = nullptr);

        /**
         * @brief create_servlet is used to create a servlet by parameters, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlet to create.
         * @param zone [input] is the zone name of the servlet to create.
         * @param servlet [input] is the servlet name to create.
         * @param retry_time [input] is the retry times of the create servlet.
         * @return Status::OK if the servlet was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        create_servlet(const std::string &ns, const std::string &zone, const std::string &servlet,
                       int *retry_time = nullptr);

        /**
         * @brief create_servlet_by_json is used to create a servlet by json string, it is a synchronous call.
         * @param json_str [input] is the json string to create the servlet from, it contains the servlet name, content, version and type.
         * @param retry_time [input] is the retry times of the create servlet.
         * @return Status::OK if the servlet was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_servlet_by_json(const std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief create_servlet_by_file is used to create a servlet by json file, it is a synchronous call.
         * @param path [input] is the path of the json file to read the servlet from.
         * @param retry_time [input] is the retry times of the create servlet.
         * @return Status::OK if the servlet was created successfully. Otherwise, an error status is returned. 
         */
        turbo::Status create_servlet_by_file(const std::string &path, int *retry_time = nullptr);


        /**
         * @brief remove_servlet is used to remove a servlet from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlet to remove.
         * @param zone [input] is the zone name of the servlet to remove.
         * @param servlet [input] is the servlet name to remove.
         * @param retry_time [input] is the retry times of the remove servlet.
         * @return Status::OK if the servlet was removed successfully. Otherwise, an error status is returned. 
         */
        turbo::Status remove_servlet(const std::string &ns, const std::string &zone, const std::string &servlet,
                                     int *retry_time = nullptr);

        /**
         * @brief Modify a servlet by parameters, it is a synchronous call.
         * @param servlet_info [input] is the servlet to remove.
         * @param retry_time [input] is the retry times of the modify servlet.
         * @return Status::OK if the servlet was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_servlet(const EA::discovery::ServletInfo &servlet_info, int *retry_time = nullptr);

        /**
         * @brief modify_servlet_by_json is used to modify a servlet by json string, it is a synchronous call.
         * @param json_str [input] is the json string to modify the servlet from, it contains the servlet name, content, version and type.
         * @param retry_time [input] is the retry times of the modify servlet.
         * @return Status::OK if the servlet was modified successfully. Otherwise, an error status is returned. 
         */
        turbo::Status modify_servlet_by_json(const std::string &json_str, int *retry_time = nullptr);

        /**
         * @brief modify_servlet_by_file is used to modify a servlet by json file, it is a synchronous call.
         * @param path [input] is the path of the json file to read the servlet from.
         * @param retry_time [input] is the retry times of the modify servlet.
         * @return Status::OK if the servlet was modified successfully. Otherwise, an error status is returned. 
         */ 
        turbo::Status modify_servlet_by_file(const std::string &path, int *retry_time = nullptr);

        /**
         * @brief list_servlet is used to list all servlets from the discovery server, it is a synchronous call.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_servlet(std::vector<EA::discovery::ServletInfo> &servlet_list, int *retry_time = nullptr);

        /**
         * @brief list_servlet is used to list all servlets of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_servlet(const std::string &ns, std::vector<EA::discovery::ServletInfo> &servlet_list,
                     int *retry_time = nullptr);


        /**
         * @brief list_servlet is used to list all servlets of a zone from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param zone [input] is the zone name of the servlets to list.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_servlet(const std::string &ns, const std::string &zone,
                     std::vector<EA::discovery::ServletInfo> &servlet_list, int *retry_time = nullptr);

        /**
         * @brief list_servlet is used to list all servlets from the discovery server, it is a synchronous call.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_servlet(std::vector<std::string> &servlet_list, int *retry_time = nullptr);

        /**
         * @brief list_servlet is used to list all servlets of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_servlet(const std::string &ns, std::vector<std::string> &servlet_list, int *retry_time = nullptr);

        /**
         * @brief list_servlet is used to list all servlets of a zone from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param zone [input] is the zone name of the servlets to list.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
         
        turbo::Status
        list_servlet(const std::string &ns, const std::string &zone, std::vector<std::string> &servlet_list,
                     int *retry_time = nullptr);

        /**
         * @brief list_servlet_to_json is used to list all servlets from the discovery server, it is a synchronous call.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_servlet_to_json(std::vector<std::string> &servlet_list, int *retry_time = nullptr);

        /**
         * @brief list_servlet_to_json is used to list all servlets of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_servlet_to_json(const std::string &ns, std::vector<std::string> &servlet_list, int *retry_time = nullptr);

        /**
         * @brief list_servlet_to_json is used to list all servlets of a zone from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param zone [input] is the zone name of the servlets to list.
         * @param servlet_list [output] is the servlets received from the discovery server.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        list_servlet_to_json(const std::string &ns, const std::string &zone, std::vector<std::string> &servlet_list,
                             int *retry_time = nullptr);


        /**
         * @brief list_servlet_to_file is used to list all servlets from the discovery server, it is a synchronous call.
         * @param save_path [input] is the path of the file to save the servlets to.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_servlet_to_file(const std::string &save_path, int *retry_time = nullptr);

        /**
         * @brief list_servlet_to_file is used to list all servlets of a namespace from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param save_path [input] is the path of the file to save the servlets to.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
         
        turbo::Status
        list_servlet_to_file(const std::string &ns, const std::string &save_path, int *retry_time = nullptr);

        /**
         * @brief list_servlet_to_file is used to list all servlets of a zone from the discovery server, it is a synchronous call.
         * @param ns [input] is the namespace name of the servlets to list.
         * @param zone [input] is the zone name of the servlets to list.
         * @param save_path [input] is the path of the file to save the servlets to.
         * @param retry_time [input] is the retry times of the list servlet.
         * @return Status::OK if the servlets were received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status list_servlet_to_file(const std::string &ns, const std::string &zone, const std::string &save_path,
                                           int *retry_time = nullptr);

        /**
         * @brief get_servlet is used to get a servlet from the discovery server, it is a synchronous call.
         * @param ns_name [input] is the namespace name of the servlet to get.
         * @param zone_name [input] is the zone name of the servlet to get.
         * @param servlet [input] is the servlet to get.
         * @param servlet_pb [output] is the servlet received from the discovery server.
         * @param retry_time [input] is the retry times of the get servlet.
         * @return Status::OK if the servlet was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status get_servlet(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                                  EA::discovery::ServletInfo &servlet_pb,
                                  int *retry_time = nullptr);

        /**
         * @brief get_servlet is used to get a servlet from the discovery server, it is a synchronous call.
         * @param ns_name [input] is the namespace name of the servlet to get.
         * @param zone_name [input] is the zone name of the servlet to get.
         * @param servlet [input] is the servlet to get.
         * @param json_str [output] is the servlet received from the discovery server.
         * @param retry_time [input] is the retry times of the get servlet.
         * @return Status::OK if the servlet was received successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        get_servlet_json(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                         std::string &json_str,
                         int *retry_time = nullptr);

        /**
         * @brief save_servlet_json is used to save a servlet to local file, it is a synchronous call.
         * @param ns_name [input] is the namespace name of the servlet to save.
         * @param zone_name [input] is the zone name of the servlet to save.
         * @param servlet [input] is the servlet to save.
         * @param json_path [input] is the path of the file to save the servlet to.
         * @param retry_time [input] is the retry times of the save servlet.
         * @return Status::OK if the servlet was saved successfully. Otherwise, an error status is returned. 
         */
        turbo::Status
        save_servlet_json(const std::string &ns_name, const std::string &zone_name, const std::string &servlet,
                          const std::string &json_path,
                          int *retry_time = nullptr);

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the discovery server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the DiscoveryManagerResponse received from the discovery server.
         * @param retry_time [input] is the retry times of the discovery manager.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                   EA::discovery::DiscoveryManagerResponse &response, int *retry_time);

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the discovery server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the DiscoveryManagerResponse received from the discovery server.
         * @param retry_time [input] is the retry times of the discovery manager.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        turbo::Status discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                 EA::discovery::DiscoveryQueryResponse &response, int *retry_time);

    private:
        BaseMessageSender *_sender;
    };

    inline turbo::Status DiscoveryClient::discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                                  EA::discovery::DiscoveryManagerResponse &response, int *retry_time) {
        if (!retry_time) {
            return _sender->discovery_manager(request, response);
        }
        return _sender->discovery_manager(request, response, *retry_time);
    }

    inline turbo::Status DiscoveryClient::discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                                EA::discovery::DiscoveryQueryResponse &response, int *retry_time) {
        if (!retry_time) {
            return _sender->discovery_query(request, response);
        }
        return _sender->discovery_query(request, response, *retry_time);
    }

}  // namespace EA::client

#endif // EA_CLIENT_DISCOVERY_H_
