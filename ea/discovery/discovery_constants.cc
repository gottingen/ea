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


#include "ea/discovery/discovery_constants.h"

namespace EA::discovery {

    const std::string DiscoveryConstants::SCHEMA_IDENTIFY(1, 0x01);

    const std::string DiscoveryConstants::MAX_ID_SCHEMA_IDENTIFY(1, 0x01);
    const std::string DiscoveryConstants::NAMESPACE_SCHEMA_IDENTIFY(1, 0x02);
    const std::string DiscoveryConstants::ZONE_SCHEMA_IDENTIFY(1, 0x09);
    const std::string DiscoveryConstants::SERVLET_SCHEMA_IDENTIFY(1, 0x0A);

    const std::string DiscoveryConstants::PRIVILEGE_IDENTIFY(1, 0x02);

    const std::string DiscoveryConstants::CONFIG_IDENTIFY(1, 0x04);

    const std::string DiscoveryConstants::DISCOVERY_IDENTIFY(1, 0x03);
    const std::string DiscoveryConstants::DISCOVERY_MAX_ID_IDENTIFY(1, 0x01);
    const std::string DiscoveryConstants::DISCOVERY_INSTANCE_IDENTIFY(1, 0x03);
    const std::string DiscoveryConstants::INSTANCE_PARAM_CLUSTER_IDENTIFY(1, 0x04);


    const std::string DiscoveryConstants::MAX_IDENTIFY(1, 0xFF);

    /// for schema
    const std::string DiscoveryConstants::MAX_NAMESPACE_ID_KEY = "max_namespace_id";
    const std::string DiscoveryConstants::MAX_ZONE_ID_KEY = "max_zone_id";
    const std::string DiscoveryConstants::MAX_SERVLET_ID_KEY = "max_servlet_id";
    const std::string DiscoveryConstants::MAX_INSTANCE_ID_KEY = "max_instance_id";

    const int DiscoveryConstants::DiscoveryMachineRegion = 0;
    const int DiscoveryConstants::AutoIDMachineRegion = 1;
    const int DiscoveryConstants::TsoMachineRegion = 2;
}  // namespace EA::discovery
