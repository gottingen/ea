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
// Created by jeff on 23-11-30.
//

#ifndef EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_
#define EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_

#include "eapi/servlet/servlet.struct.pb.h"
#include "turbo/base/status.h"

namespace EA::client {

    /**
     * @ingroup ea_proto
     * @brief ConfigInfoBuilder is helper class for build ConfigInfo object,
     *        it does not hold the ConfigInfo object memory, and not thread safe. user should guard
     *        that the object is usable and make sure it is thread safe call. eg.
     * @code
     *        EA::servlet::ConfigInfo info;
     *        ServletInstanceBuilder builder(&info);
     *        std::string json_str = "{}"
     *        auto status = builder.build_from_json(json_str);
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class ServletInstanceBuilder {
    public:
        ServletInstanceBuilder() = default;

        explicit ServletInstanceBuilder(EA::servlet::ServletInstance *ins);

        ~ServletInstanceBuilder() = default;

    public:

        /**
         *
         * @param ins the ConfigInfo object to build
         */
        void set(EA::servlet::ServletInstance *ins);

        /**
         *
         * @param json_str [input] the json string to build ConfigInfo object.
         * @return Status::OK if the ConfigInfo object was built successfully. Otherwise, an error status is returned.
         */
        turbo::Status build_from_json(const std::string &json_str);

        /**
         *
         * @param json_path [input] the json file path to build ConfigInfo object.
         * @return Status::OK if the ConfigInfo object was built successfully. Otherwise, an error status is returned.
         */
        turbo::Status build_from_json_file(const std::string &json_path);

        /**
         *
         * @param namespace_name [input] the namespace name to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_namespace(const std::string &namespace_name);

        /**
         *
         * @param zone [input] the zone to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_zone(const std::string &zone);

        /**
         *
         * @param servlet [input] the servlet to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_servlet(const std::string &servlet);


        /**
         *
         * @param color [input] the color to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_color(const std::string &color);

        /**
         *
         * @param env [input] the env to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_env(const std::string &env);

        /**
         *
         * @param user [input] the user to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_user(const std::string &user);

        /**
         *
         * @param passwd [input] the passwd to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_passwd(const std::string &passwd);

        /**
         *
         * @param status [input] the status to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_status(const std::string &status);


        /**
         *
         * @param address [input] the address to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_address(const std::string &address);

        /**
         *
         * @param status [input] the status to set.
         * @return the ServletInstanceBuilder.
         */        
        ServletInstanceBuilder &set_status(const EA::servlet::Status &status);

        /**
         *
         * @param weight [input] the weight to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_weight(int weight);

        /**
         *
         * @param time [input] the time to set.
         * @return the ServletInstanceBuilder.
         */
        ServletInstanceBuilder &set_time(int time);

    private:
        EA::servlet::ServletInstance *_instance;
    };
}  // namespace EA::client

#endif  // EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_
