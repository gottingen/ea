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
//
// Created by jeff on 23-11-19.
//

#ifndef EA_RAFT_PARSE_PATH_H_
#define EA_RAFT_PARSE_PATH_H_

#include <string>
#include <cstdint>

namespace EA {
    int64_t parse_snapshot_index_from_path(const std::string &snapshot_path, bool use_dirname);
}
#endif  // EA_RAFT_PARSE_PATH_H_
