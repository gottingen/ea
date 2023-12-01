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
     */
    class Dumper {
    public:

        /**
         *
         * @param path
         * @param message
         * @return
         */
        static turbo::Status dump_proto_to_file(const std::string &path, const google::protobuf::Message &message);

        /**
         *
         * @param message
         * @param content
         * @return
         */
        static turbo::Status dump_proto(const google::protobuf::Message &message, std::string &content);
    };
}  // namespace EA::client

#endif  // EA_CLIENT_DUMPER_H_
