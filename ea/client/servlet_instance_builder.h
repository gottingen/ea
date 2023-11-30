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
// Created by jeff on 23-11-30.
//

#ifndef EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_
#define EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_

#include "eapi/servlet/servlet.struct.pb.h"
#include "turbo/base/status.h"

namespace EA::client {

    class ServletInstanceBuilder {
    public:
        ServletInstanceBuilder() = default;

        explicit ServletInstanceBuilder(EA::servlet::ServletInstance *ins);

        ~ServletInstanceBuilder() = default;

    public:
        ///
        /// \param ins
        void set(EA::servlet::ServletInstance *ins);

        ///
        /// \param json_str
        /// \return
        turbo::Status build_from_json(const std::string &json_str);

        ///
        /// \param json_path
        /// \return
        turbo::Status build_from_json_file(const std::string &json_path);

        ///
        /// \param namespace_name
        /// \return
        ServletInstanceBuilder &set_namespace(const std::string &namespace_name);

        ///
        /// \param zone
        /// \return
        ServletInstanceBuilder &set_zone(const std::string &zone);

        ///
        /// \param servlet
        /// \return
        ServletInstanceBuilder &set_servlet(const std::string &servlet);

        ///
        /// \param color
        /// \return
        ServletInstanceBuilder &set_color(const std::string &color);

        ///
        /// \param env
        /// \return
        ServletInstanceBuilder &set_env(const std::string &env);

        ///
        /// \param user
        /// \return
        ServletInstanceBuilder &set_user(const std::string &user);

        ///
        /// \param passwd
        /// \return
        ServletInstanceBuilder &set_passwd(const std::string &passwd);

        ///
        /// \param s
        /// \return
        ServletInstanceBuilder &set_status(const std::string &s);

        ///
        /// \param address
        /// \return
        ServletInstanceBuilder &set_address(const std::string &address);

        ///
        /// \param s
        /// \return
        ServletInstanceBuilder &set_status(const EA::servlet::Status &s);

        ///
        /// \param weight
        /// \return
        ServletInstanceBuilder &set_weight(int weight);

        ///
        /// \param time
        /// \return
        ServletInstanceBuilder &set_time(int time);

    private:
        EA::servlet::ServletInstance *_instance;
    };
}  // namespace EA::client

#endif  // EA_CLIENT_SERVLET_INSTANCE_BUILDER_H_
