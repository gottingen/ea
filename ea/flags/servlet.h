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


#ifndef EA_FLAGS_SERVLET_H_
#define EA_FLAGS_SERVLET_H_

#include "gflags/gflags_declare.h"

namespace EA {

    DECLARE_string(servlet_listen);
    DECLARE_string(servlet_namespace);
    DECLARE_string(servlet_zone);
    DECLARE_string(servlet_name);
    DECLARE_string(servlet_physical);
    DECLARE_string(servlet_resource_tag);
    DECLARE_string(servlet_env);
}  // namespace EA

#endif  // EA_FLAGS_SERVLET_H_
