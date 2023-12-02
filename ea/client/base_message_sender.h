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


#ifndef EA_CLIENT_BASE_MESSAGE_SENDER_H_
#define EA_CLIENT_BASE_MESSAGE_SENDER_H_

#include "turbo/base/status.h"
#include "eapi/discovery/discovery.interface.pb.h"

namespace EA::client {

    /**
     * @ingroup ea_rpc
     * @brief BaseMessageSender is the interface for sending messages to the meta server.
     *        It is used by the DiscoveryClient to send messages to the meta server. The DiscoveryClient will
     *        implement this interface. 
     */
    class BaseMessageSender {
    public:
        virtual ~BaseMessageSender() = default;

        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                                                     EA::discovery::DiscoveryManagerResponse &response, int retry_times) = 0;
        /**
         * @brief discovery_manager is used to send a DiscoveryManagerRequest to the meta server.
         * @param request [input] is the DiscoveryManagerRequest to send.
         * @param response [output] is the MetaManagerResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */        
        virtual turbo::Status discovery_manager(const EA::discovery::DiscoveryManagerRequest &request,
                                           EA::discovery::DiscoveryManagerResponse &response) = 0;

        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @param retry_times [input] is the number of times to retry sending the request.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned.
         */
        virtual turbo::Status discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                                    EA::discovery::DiscoveryQueryResponse &response, int retry_times) = 0;
        /**
         * @brief discovery_query is used to send a DiscoveryQueryRequest to the meta server.
         * @param request [input] is the DiscoveryQueryRequest to send.
         * @param response [output] is the DiscoveryQueryResponse received from the meta server.
         * @return Status::OK if the request was sent successfully. Otherwise, an error status is returned. 
         */
        virtual turbo::Status discovery_query(const EA::discovery::DiscoveryQueryRequest &request,
                                         EA::discovery::DiscoveryQueryResponse &response) = 0;
    };
}  // namespace EA::client

#endif  // EA_CLIENT_BASE_MESSAGE_SENDER_H_
