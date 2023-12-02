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

#ifndef EA_CLIENT_LOADER_H_
#define EA_CLIENT_LOADER_H_

#include "turbo/base/status.h"
#include <string>
#include <google/protobuf/descriptor.h>
#include "eapi/servlet/servlet.interface.pb.h"

/**
 * @defgroup ea_proto_g proto  proto operators
 *
 */

namespace EA::client {

    /**
     * @ingroup ea_proto_g
     * @brief Loader is a helper class for proto convert from json to protobuf.
     *        do not ignore the result of function, it is recommend use like:
     * @code
     *        auto json_config = "R{
     *             "name": "example",
     *             "version": {
     *               "major": 1,
     *               "minor": 2,
     *               "patch": 3
     *             },
     *             "content": "{\"servlet\":\"sug\",\"zone\":{\"instance\":[\"192.168.1.2\",\"192.168.1.3\","
     *                          "\"192.168.1.3\"],\"name\":\"ea_search\",\"user\":\"jeff\"}}",
     *             "type": "CF_JSON",
     *             "time": 1701477509
     *           }"
     *        EA::servlet::ConfigInfo info;
     *        auto status = Loader::load_proto(json_config, info);
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class Loader {
    public:

        /**
         * @brief load a json format string to the message
         * @param content [input] json format string.
         * @param message [output] result of parse the json format string.
         * @return status.ok() if success else return the reason of parse fail.
         */
        static turbo::Status load_proto(const std::string &content, google::protobuf::Message &message);

        /**
         *
         * @param path [input]file path, the file content must be json format.
         * @param message [output] result of parse the json format string.
         * @return status.ok() if success else return the reason of parse fail.
         */
        static turbo::Status load_proto_from_file(const std::string &path, google::protobuf::Message &message);
    };
}  // namespace EA::client

#endif  // EA_CLIENT_LOADER_H_
