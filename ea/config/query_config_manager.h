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

#ifndef EA_CONFIG_QUERY_CONFIG_MANAGER_H_
#define EA_CONFIG_QUERY_CONFIG_MANAGER_H_

#include "eaproto/ops/ops.interface.pb.h"

namespace EA::config {

    class QueryConfigManager {
    public:
        static QueryConfigManager *get_instance() {
            static QueryConfigManager ins;
            return &ins;
        }

        void
        get_config(const ::EA::proto::QueryOpsServiceRequest *request, ::EA::proto::QueryOpsServiceResponse *response);

        void
        list_config(const ::EA::proto::QueryOpsServiceRequest *request, ::EA::proto::QueryOpsServiceResponse *response);

        void
        list_config_version(const ::EA::proto::QueryOpsServiceRequest *request,
                            ::EA::proto::QueryOpsServiceResponse *response);
    };
}  // namespace EA::config

#endif  // EA_CONFIG_QUERY_CONFIG_MANAGER_H_
