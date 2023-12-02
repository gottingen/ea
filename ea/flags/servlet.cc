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

#include "ea/flags/servlet.h"
#include "gflags/gflags.h"

namespace EA {

    DEFINE_string(servlet_listen, "0.0.0.0:8898", "router default ip port");
    DEFINE_string(servlet_namespace, "default", "servlet namespace");
    DEFINE_string(servlet_zone, "default", "servlet namespace");
    DEFINE_string(servlet_name, "default", "servlet namespace");
    DEFINE_string(servlet_physical, "default", "servlet namespace");
    DEFINE_string(servlet_resource_tag, "default", "servlet namespace");
    DEFINE_string(servlet_env, "default", "servlet namespace");
}  // namespace EA
