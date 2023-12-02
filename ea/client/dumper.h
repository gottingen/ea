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

#ifndef EA_CLIENT_DUMPER_H_
#define EA_CLIENT_DUMPER_H_

#include "turbo/base/status.h"
#include <string>
#include <google/protobuf/descriptor.h>
#include "eapi/servlet/servlet.interface.pb.h"

namespace EA::client {

    /**
     * @ingroup meta_client
     * @details Dumper is a helper class for protobuf object convert to json string.
     *        do not ignore the result of function, it is recommend use like:
     * @code
     *        const std::string json_content = "R{
     *             "name": "example",
     *             "version": {
     *               "major": 1,
     *               "minor": 2,
     *               "patch": 3
     *             },
     *             "content": "{\"servlet\":\"sug\",\"zone\":{\"instance\":"
     *                        "[\"192.168.1.2\",\"192.168.1.3\",\"192.168.1.3\"]"
     *                        ",\"name\":\"ea_search\",\"user\":\"jeff\"}}",
     *             "type": "CF_JSON",
     *             "time": 1701477509
     *           }"
     *
     *        EA::servlet::ConfigInfo info;
     *        info.set_name("example");
     *        info.mutable_version()->set_major(1);
     *        info.mutable_version()->set_minor(2);
     *        info.mutable_version()->set_path(3);
     *        info.set_content("{\"servlet\":\"sug\",\"zone\":{\"instance\":[\"192.168.1.2\","
     *                          "\"192.168.1.3\",\"192.168.1.3\"],\"name\":\"ea_search\",\"user\":\"jeff\"}}");
     *        info.set_type(EA::servlet::CF_JSON);
     *        info.set_time(1701477509);
     *        std::string parsed_string;
     *        auto status = Loader::dump_proto(info, parsed_string);
     *        if(!status.ok) {
     *          handler_error();
     *         }
     *         ...
     *         handler_success();
     * @endcode
     */
    class Dumper {
    public:

        /**
         *
         * @param path
         * @param message
         * @return status.ok() if success else return the reason of format fail.
         */
        static turbo::Status dump_proto_to_file(const std::string &path, const google::protobuf::Message &message);

        /**
         * @brief dump a pb object message to json format string
         * @param message [input] a pb object message
         * @param content [output] string to store json format result
         * @return status.ok() if success else return the reason of format fail.
         */
        static turbo::Status dump_proto(const google::protobuf::Message &message, std::string &content);
    };
}  // namespace EA::client

#endif  // EA_CLIENT_DUMPER_H_
