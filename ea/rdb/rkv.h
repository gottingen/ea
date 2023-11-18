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

#ifndef EA_RDB_RKV_H_
#define EA_RDB_RKV_H_

#include <string>
#include <vector>
#include "turbo/base/status.h"
#include "ea/rdb/storage.h"
#include <functional>

namespace EA::rdb {

    class Rkv {
    public:
        typedef std::function<bool(const std::string &key, const std::string &value)> scan_func;

        Rkv() = default;

        void init(const std::string&ns);

        turbo::Status put(const std::string &key, const std::string &value);

        turbo::Status mput(const std::vector<std::string> &keys, const std::vector<std::string> &values);

        turbo::Status get(const std::string &key, std::string *value);

        turbo::Status remove(const std::string &keys);

        turbo::Status mremove(const std::vector<std::string> &keys);

        turbo::Status scan(const scan_func &func);
    private:
        std::string make_key(const std::string& user_key);
    private:
        std::string                  _namespace;
        Storage                     *_storage{nullptr};
        rocksdb::ColumnFamilyHandle *_handle{nullptr};
    };
}  // namespace EA::rdb

#endif  // EA_RDB_RKV_H_
