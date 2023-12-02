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
     * @brief BaseMessageSender is the interface for sending messages to the meta server.
     *        It is used by the MetaClient to send messages to the meta server. The MetaClient will
     *        implement this interface. 
     */
    class BaseMessageSender {
    public:
        virtual ~BaseMessageSender() = default;

        /**
         * @brief meta_manager is used to send a MetaManagerRequest to the meta server.
         * @param request [input] is the MetaManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                                                     EA::servlet::MetaManagerResponse &response, int retry_times) = 0;
        /**
         * @brief meta_manager is used to send a MetaManagerRequest to the meta server.
         * @param request [input] is the MetaManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */        
        virtual turbo::Status meta_manager(const EA::servlet::MetaManagerRequest &request,
                                           EA::servlet::MetaManagerResponse &response) = 0;

        /**
         * @brief meta_query is used to send a QueryRequest to the meta server.
         * @param [input] request is the QueryRequest to send.
         * @param response [output] is the QueryResponse received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                                    EA::servlet::QueryResponse &response, int retry_times) = 0;
        /**
         * @brief meta_query is used to send a QueryRequest to the meta server.
         * @param request [input] is the QueryRequest to send.
         * @param response [output] is the QueryResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status meta_query(const EA::servlet::QueryRequest &request,
                                         EA::servlet::QueryResponse &response) = 0;
    };
}  // namespace EA::client

#endif  // EA_CLIENT_BASE_MESSAGE_SENDER_H_
