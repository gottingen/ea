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


#include "ea/flags/client.h"
#include "gflags/gflags.h"

namespace EA {
    DEFINE_string(config_cache_dir, "./config_cache", "config cache dir");
    DEFINE_int32(config_watch_interval_ms, 1, "config watch sleep between two watch config");
    DEFINE_int32(config_watch_interval_round_s, 30, "every x(s) to fetch and get config for a round");
}  // namespace EA