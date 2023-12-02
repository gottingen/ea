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

#ifndef EA_CLIENT_BASE_MESSAGE_SENDER_H_
#define EA_CLIENT_BASE_MESSAGE_SENDER_H_

#include "turbo/base/status.h"
#include "eapi/servlet/servlet.interface.pb.h"

/**
 * * @defgroup meta_client
 */
namespace EA::client {

    /**
     * @ingroup meta_client
     */
    class BaseMessageSender {
    public:
        virtual ~BaseMessageSender() = default;

        /**
         *
         * @param request
         * @param response
         * @param retry_times
         * @return
         */
        virtual turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                                                     EA::servlet::MetaManagerResponse &response, int retry_times) = 0;
        /**
         *
         * @param request
         * @param response
         * @return
         */
        virtual turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                           EA::servlet::MetaManagerResponse &response) = 0;

        /**
         *
         * @param request
         * @param response
         * @param retry_times
         * @return
         */
        virtual turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                                    EA::servlet::QueryResponse &response, int retry_times) = 0;
        /**
         *
         * @param request
         * @param response
         * @return
         */
        virtual turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                         EA::servlet::QueryResponse &response) = 0;
    };
}  // namespace EA::client

#endif  // EA_CLIENT_BASE_MESSAGE_SENDER_H_
