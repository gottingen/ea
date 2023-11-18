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

#ifndef EA_CONFIG_CONFIG_RKV_H_
#define EA_CONFIG_CONFIG_RKV_H_

#include "ea/rdb/rkv.h"

namespace EA::config {

    class ConfigRkv {
    public:
        static ConfigRkv *get_instance() {
            static ConfigRkv ins;
            return &ins;
        }

        static EA::rdb::Rkv *get_rkv() {
            return &get_instance()->_rkv;
        }

    private:
        ConfigRkv();

    private:
        EA::rdb::Rkv _rkv;
    };

    inline ConfigRkv::ConfigRkv(){
        _rkv.init(std::string(1, 0x01));
    }
}  // namespace EA::config

#endif // EA_CONFIG_CONFIG_RKV_H_
