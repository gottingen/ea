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

#include "ea/discovery/parse_path.h"
#include "turbo/strings/str_split.h"
#include "butil/file_util.h"

namespace EA::discovery {
    int64_t parse_snapshot_index_from_path(const std::string &snapshot_path, bool use_dirname) {
        butil::FilePath path(snapshot_path);
        std::string tmp_path;
        if (use_dirname) {
            tmp_path = path.DirName().BaseName().value();
        } else {
            tmp_path = path.BaseName().value();
        }
        std::vector<std::string> split_vec;
        std::vector<std::string> snapshot_index_vec;
        split_vec = turbo::str_split(tmp_path, '/', turbo::skip_empty());
        snapshot_index_vec = turbo::str_split(split_vec.back(), '_', turbo::skip_empty());
        int64_t snapshot_index = 0;
        if (snapshot_index_vec.size() == 2) {
            snapshot_index = atoll(snapshot_index_vec[1].c_str());
        }
        return snapshot_index;
    }

}  // namespace EA::discovery
