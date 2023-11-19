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


#include <string>
#include <fstream>
#include <brpc/server.h>
#include <gflags/gflags.h>
#include "ea/dict/dict_server.h"
#include "ea/rdb/storage.h"
#include "ea/base/memory_profile.h"
#include "turbo/files/filesystem.h"
#include "turbo/strings/str_split.h"
#include "ea/base/tlog.h"
#include "ea/gflags/dict.h"

int main(int argc, char **argv) {
    google::SetCommandLineOption("flagfile", "conf/eadict_gflags.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);
    turbo::filesystem::path remove_path("init.success");
    turbo::filesystem::remove_all(remove_path);
    // Initail log
    if (!EA::init_tlog()) {
        fprintf(stderr, "log init failed.");
        return -1;
    }
    TLOG_INFO("log file load success");

    auto dir = turbo::filesystem::path(EA::FLAGS_dict_db_path).parent_path();
    std::error_code ec;
    if(!turbo::filesystem::exists(dir, ec)) {
        turbo::filesystem::create_directories(dir);
    }

    auto rs = EA::rdb::Storage::get_instance()->init(EA::FLAGS_dict_db_path);
    if (rs < 0) {
        TLOG_ERROR("rocksdb init fail");
        return -1;
    }

    //add service
    brpc::Server server;
    //将raft加入到baidu-rpc server中
    if (0 != braft::add_service(&server, EA::FLAGS_dict_listen.c_str())) {
        TLOG_ERROR("Fail to init raft");
        return -1;
    }
    TLOG_INFO("add raft to baidu-rpc server success");

    int ret = 0;
    //this step must be before server.Start
    std::vector<braft::PeerId> peers;
    std::vector<std::string> instances;
    bool completely_deploy = false;
    bool use_bns = false;
    //指定的是ip:port的形式
    std::vector<std::string> list_raft_peers = turbo::StrSplit(EA::FLAGS_dict_server_bns, ',');
    for (auto &raft_peer: list_raft_peers) {
        TLOG_INFO("raft_peer:{}", raft_peer.c_str());
        braft::PeerId peer(raft_peer);
        peers.push_back(peer);
    }

    EA::dict::DictServer *service_server = EA::dict::DictServer::get_instance();
    //注册处理service 逻辑的service服务
    if (0 != server.AddService(service_server, brpc::SERVER_DOESNT_OWN_SERVICE)) {
        TLOG_ERROR("Fail to Add dict service");
        return -1;
    }
    //启动端口
    if (server.Start(EA::FLAGS_dict_listen.c_str(), nullptr) != 0) {
        TLOG_ERROR("Fail to start server");
        return -1;
    }
    TLOG_INFO("baidu-rpc server start");
    if (service_server->init(peers) != 0) {
        TLOG_ERROR("service server init fail");
        return -1;
    }
    EA::MemoryGCHandler::get_instance()->init();
    if (!completely_deploy && use_bns) {
        // 循环等待数据加载成功, ip_list配置区分不了全新/更新部署
        while (!service_server->have_data()) {
            bthread_usleep(1000 * 1000);
        }
        std::ofstream init_fs("init.success", std::ofstream::out | std::ofstream::trunc);
    }
    TLOG_INFO("service server init success");
    //server.RunUntilAskedToQuit(); 这个方法会先把端口关了，导致丢请求
    while (!brpc::IsAskedToQuit()) {
        bthread_usleep(1000000L);
    }
    TLOG_INFO("recevie kill signal, begin to quit");
    service_server->shutdown_raft();
    service_server->close();
    EA::MemoryGCHandler::get_instance()->close();
    EA::rdb::Storage::get_instance()->close();
    TLOG_INFO("raft shut down, rocksdb close");
    server.Stop(0);
    server.Join();
    TLOG_INFO("service server quit success");
    return 0;
}

