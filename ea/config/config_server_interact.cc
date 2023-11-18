// Copyright 2023 The Elastic AI Search Authors.
//
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


#include "ea/config/config_server_interact.h"

namespace EA::config {

    int ConfigServerInteract::init(bool is_backup) {
        if (is_backup) {
            if (!FLAGS_config_backup_server_bns.empty()) {
                return init_internal(FLAGS_config_backup_server_bns);
            }
        } else {
            return init_internal(FLAGS_config_server_bns);
        }
        return 0;
    }

    int ConfigServerInteract::init_internal(const std::string &file_bns) {
        _master_leader_address.ip = butil::IP_ANY;
        _master_leader_address.port = 0;
        _connect_timeout = FLAGS_config_connect_timeout;
        _request_timeout = FLAGS_config_request_timeout;
        //初始化channel，但是该channel是file_server的 bns pool，大部分时间用不到
        brpc::ChannelOptions channel_opt;
        channel_opt.timeout_ms = FLAGS_config_request_timeout;
        channel_opt.connect_timeout_ms = FLAGS_config_connect_timeout;
        std::string config_server_addr = file_bns;
        //bns
        if (file_bns.find(":") == std::string::npos) {
            config_server_addr = std::string("bns://") + file_bns;
        } else {
            config_server_addr = std::string("list://") + file_bns;
        }
        if (_bns_channel.Init(config_server_addr.c_str(), "rr", &channel_opt) != 0) {
            TLOG_ERROR("file server bns pool init fail. bns_name:{}", config_server_addr);
            return -1;
        }
        _is_inited = true;
        return 0;
    }
}  // namespace EA::config

