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



#ifndef EA_CLI_PROTO_HELP_H_
#define EA_CLI_PROTO_HELP_H_

#include "eapi/discovery/discovery.interface.pb.h"
#include "turbo/base/result_status.h"

namespace EA::cli {

    std::string config_type_to_string(EA::discovery::ConfigType type);

    turbo::ResultStatus<EA::discovery::ConfigType> string_to_config_type(const std::string &str);

    std::string get_op_string(EA::discovery::OpType type);

    std::string get_op_string(EA::RaftControlOp type);

    std::string get_op_string(EA::discovery::QueryOpType type);

    turbo::Status string_to_version(const std::string &str, EA::discovery::Version*v);

    std::string version_to_string(const EA::discovery::Version &v);


}  // namespace EA::cli

#endif  // EA_CLI_PROTO_HELP_H_
