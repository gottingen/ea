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

#ifndef EA_BASE_FILE_UTIL_H_
#define EA_BASE_FILE_UTIL_H_

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include <functional>
#include <sstream>
#include <string>

namespace EA {

    ssize_t ea_pread(int fd, void* data, size_t len, off_t offset);

    ssize_t ea_pwrite(int fd, const void* data, size_t len, off_t offset);

}  // namespace EA
#endif  // EA_BASE_FILE_UTIL_H_
