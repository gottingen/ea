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


#ifndef EA_META_SERVER_META_CONSTANTS_H_
#define EA_META_SERVER_META_CONSTANTS_H_

#include <string>

namespace EA::servlet {

    struct MetaConstants {

        static const std::string SCHEMA_IDENTIFY;
        static const std::string MAX_ID_SCHEMA_IDENTIFY;
        static const std::string NAMESPACE_SCHEMA_IDENTIFY;
        static const std::string ZONE_SCHEMA_IDENTIFY;
        static const std::string SERVLET_SCHEMA_IDENTIFY;

        static const std::string PRIVILEGE_IDENTIFY;

        static const std::string DISCOVERY_IDENTIFY;
        static const std::string DISCOVERY_MAX_ID_IDENTIFY;
        static const std::string DISCOVERY_INSTANCE_IDENTIFY;
        static const std::string INSTANCE_PARAM_CLUSTER_IDENTIFY;

        static const std::string CONFIG_IDENTIFY;

        static const std::string MAX_IDENTIFY;

        /// for schema
        static const std::string MAX_NAMESPACE_ID_KEY;
        static const std::string MAX_ZONE_ID_KEY;
        static const std::string MAX_SERVLET_ID_KEY;
        static const std::string MAX_INSTANCE_ID_KEY;

        static const int MetaMachineRegion;
        static const int AutoIDMachineRegion;
        static const int TsoMachineRegion;
    };

    namespace tso {
        constexpr int64_t update_timestamp_interval_ms = 50LL; // 50ms
        constexpr int64_t update_timestamp_guard_ms = 1LL; // 1ms
        constexpr int64_t save_interval_ms = 3000LL;  // 3000ms
        constexpr int64_t base_timestamp_ms = 1577808000000LL; // 2020-01-01 12:00:00
        constexpr int logical_bits = 18;
        constexpr int64_t max_logical = 1 << logical_bits;

        inline int64_t clock_realtime_ms() {
            struct timespec tp;
            ::clock_gettime(CLOCK_REALTIME, &tp);
            return tp.tv_sec * 1000ULL + tp.tv_nsec / 1000000ULL - base_timestamp_ms;
        }

        inline uint32_t get_timestamp_internal(int64_t offset) {
            return ((offset >> 18) + base_timestamp_ms) / 1000;
        }

    } // namespace tso

}  // namespace EA::servlet

#endif  // EA_META_SERVER_META_CONSTANTS_H_
