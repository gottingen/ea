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


#ifndef EA_FLAGS_LOG_H_
#define EA_FLAGS_LOG_H_

#include "gflags/gflags_declare.h"

namespace EA {

    /// for log
    DECLARE_bool(enable_console_log);
    DECLARE_string(log_root);
    DECLARE_int32(log_rotation_hour);
    DECLARE_int32(log_rotation_minute);
    DECLARE_string(log_base_name);
    DECLARE_int32(log_save_days);

}  // namespace EA
#endif  // EA_FLAGS_LOG_H_
