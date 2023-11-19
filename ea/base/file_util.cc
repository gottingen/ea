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

#include "ea/base/file_util.h"

namespace EA {

    ssize_t ea_pread(int fd, void* data, size_t len, off_t offset) {
        size_t size = len;
        ssize_t left = size;
        char* wdata = reinterpret_cast<char*>(data);
        while (left > 0) {
            ssize_t written = pread(fd, reinterpret_cast<void*>(wdata), static_cast<size_t>(left), offset);
            if (written >= 0) {
                offset += written;
                left -= written;
                wdata = wdata + written;
            } else if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        return size - left;
    }

    ssize_t ea_pwrite(int fd, const void* data, size_t len, off_t offset) {
        size_t size = len;
        ssize_t left = size;
        const char* wdata = (const char*) data;
        while (left > 0) {
            ssize_t written = pwrite(fd, (const void*) wdata, (size_t) left, offset);
            if (written >= 0) {
                offset += written;
                left -= written;
                wdata = wdata + written;
            } else if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        return size - left;
    }

}