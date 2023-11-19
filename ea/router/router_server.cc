// Copyright (c) 2020 Baidu, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ea/router/router_service.h"
#include "ea/restful/config_server.h"
#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include <brpc/server.h>
#include "ea/base/tlog.h"
#include "ea/rpc/config_server_interact.h"
#include "ea/gflags/router.h"

namespace EA {

}  // namespace EA

int main(int argc, char**argv) {
    google::SetCommandLineOption("flagfile", "conf/router_gflags.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);

    if (!EA::init_tlog()) {
        fprintf(stderr, "log init failed.");
        return -1;
    }
    TLOG_INFO("log file load success");
    // init meta interact
    auto r =EA::rpc::ConfigServerInteract::get_instance()->init();
    if(!r.ok()) {
        return -1;
    }

    brpc::Server server;
    EA::router::RouterServiceImpl router;
    if (0 != server.AddService(&router, brpc::SERVER_DOESNT_OWN_SERVICE)) {
        TLOG_ERROR("Fail to Add router service");
        return -1;
    }
    EA::restful::ConfigServer config_restful;
    if(EA::FLAGS_enable_restful) {
        if (0 != server.AddService(&config_restful, brpc::SERVER_DOESNT_OWN_SERVICE,
                                   "config/create => create_config,"
                                   "config/remove => remove_config,"
                                   "config/get => get_config,"
                                   "config/list => get_config_list,"
                                   "config/lv => get_config_version_list")) {
            TLOG_ERROR("Fail to Add config restful service");
            return -1;
        }
    }

    if (server.Start(EA::FLAGS_router_listen.c_str(), nullptr) != 0) {
        TLOG_ERROR("Fail to start server");
        return -1;
    }
    while (!brpc::IsAskedToQuit()) {
        bthread_usleep(1000000L);
    }
    TLOG_INFO("got kill signal, begin to quit");
    TLOG_INFO("router shut down");
    server.Stop(0);
    server.Join();
    TLOG_INFO("router server quit success");
    return 0;
}