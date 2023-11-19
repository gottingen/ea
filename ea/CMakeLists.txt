#
# Copyright 2023 The titan-search Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

file(COPY ${PROJECT_SOURCE_DIR}/conf DESTINATION ${PROJECT_BINARY_DIR})
file(GLOB RDB_SRC rdb/*.cc)
file(GLOB BASE base/*.cc)
file(GLOB GFLAGS gflags/*.cc)
file(GLOB RAFT raft/*.cc)
#file(GLOB RAFT raft/*.cc)

set(EA_BASE
        ${RDB_SRC}
        ${GFLAGS}
        ${BASE}
        ${RAFT}
        )

add_definitions(
        -D_GNU_SOURCE
        -D__STDC_FORMAT_MACROS
        -D__STDC_LIMIT_MACROS
        -D__STDC_CONSTANT_MACROS
        -D__const__=unused
        -DBRPC_WITH_GLOG=OFF
)

carbin_cc_library(
        NAMESPACE ea
        NAME base
        SOURCES
        ${EA_BASE}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eaproto::eaproto_STATIC
        ${CARBIN_DEPS_LINK}
        PUBLIC
)
file(GLOB EA_RPC rpc/*.cc)
carbin_cc_library(
        NAMESPACE ea
        NAME rpc
        SOURCES
        ${EA_RPC}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eaproto::eaproto_STATIC
        ea::base
        ${CARBIN_DEPS_LINK}
        PUBLIC
)

file(GLOB EA_RESTFUL restful/*.cc)
carbin_cc_library(
        NAMESPACE ea
        NAME restful
        SOURCES
        ${EA_RESTFUL}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eaproto::eaproto_STATIC
        ea::base
        ea::rpc
        ${CARBIN_DEPS_LINK}
        PUBLIC
)

file(GLOB_RECURSE CONFIG_SERVER config/*.cc)
carbin_cc_binary(
        NAMESPACE ea
        NAME config_server
        SOURCES
        ${CONFIG_SERVER}
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        ea::base
        ea::rpc
        eaproto::eaproto_STATIC
        PUBLIC
)

file(GLOB ROUTER router/*.cc)
carbin_cc_binary(
        NAMESPACE ea
        NAME router_server
        SOURCES
        ${ROUTER}
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        ea::base
        ea::rpc
        ea::restful
        eaproto::eaproto_STATIC
        PUBLIC
)

file(GLOB CLI_SRC cli/*.cc)
carbin_cc_binary(
        NAMESPACE ea
        NAME eacli
        SOURCES
        ${CLI_SRC}
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        eaproto::eaproto_STATIC
        ea::rpc
        PUBLIC
)

#[[
file(GLOB META_SERVER meta_server/*.cc)

carbin_cc_library(
        NAMESPACE elasticann
        NAME mete_server
        SOURCES
        ${META_SERVER}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eaproto::eaproto_STATIC
        elasticann::common
        ${CARBIN_DEPS_LINK}
)

file(GLOB_RECURSE CONFIG_SERVER ops/config/*.cc)
file(GLOB_RECURSE PLUGIN_SERVER ops/plugin/*.cc)
file(GLOB_RECURSE DICT_SERVER ops/dict/*.cc)
file(GLOB_RECURSE DISCOVERY_SERVER ops/servlet/*.cc)
set(OPS_SERVER
        ${CONFIG_SERVER}
        ${PLUGIN_SERVER}
        ${DICT_SERVER}
        ${DISCOVERY_SERVER}
        ${PROJECT_SOURCE_DIR}/elasticann/ops/constants.cc
        ${PROJECT_SOURCE_DIR}/elasticann/ops/file_util.cc
        ${PROJECT_SOURCE_DIR}/elasticann/ops/service_rocksdb.cc
)
carbin_cc_library(
        NAMESPACE elasticann
        NAME ops_server
        SOURCES
        ${OPS_SERVER}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eaproto::eaproto_STATIC
        elasticann::common
        ${CARBIN_DEPS_LINK}
)

carbin_cc_binary(
        NAMESPACE elasticann
        NAME eameta
        SOURCES
        ${META_SERVER}
        ${RAFT_META}
        server/eameta.cc
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        eaproto::eaproto_STATIC
        PUBLIC
)

carbin_cc_binary(
        NAMESPACE elasticann
        NAME eaconfig
        SOURCES
        ${META_SERVER}
        ${RAFT_META}
        server/eaconfig.cc
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        elasticann::ops_server
        eaproto::eaproto_STATIC
        PUBLIC
)

carbin_cc_binary(
        NAMESPACE elasticann
        NAME eaplugin
        SOURCES
        ${META_SERVER}
        ${RAFT_META}
        server/eaplugin.cc
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        elasticann::ops_server
        eaproto::eaproto_STATIC
        PUBLIC
)

carbin_cc_binary(
        NAMESPACE elasticann
        NAME eadict
        SOURCES
        ${META_SERVER}
        ${RAFT_META}
        server/eadict.cc
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        elasticann::ops_server
        eaproto::eaproto_STATIC
        PUBLIC
)

file(GLOB STORE_SERVER raft_store/*.cc)
carbin_cc_binary(
        NAMESPACE elasticann
        NAME eastore
        SOURCES
        ${STORE_SERVER}
        server/eastore.cc
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        eaproto::eaproto_STATIC
        PUBLIC
)



file(GLOB CLIENT_SRC client/*.cc)
carbin_cc_binary(
        NAMESPACE elasticann
        NAME eacli
        SOURCES
        ${CLIENT_SRC}
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        eaproto::eaproto_STATIC
        PUBLIC
)


file(GLOB ROUTER_SERVER raft_dummy/*.cc)
carbin_cc_binary(
        NAMESPACE elasticann
        NAME eadb
        SOURCES
        ${ROUTER_SERVER}
        server/db_server.cc
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        eaproto::eaproto_STATIC
        PUBLIC
)

file(GLOB EA_CLIENT client/*.cc)
carbin_cc_library(
        NAMESPACE elasticann
        NAME eaclient
        SOURCES
        ${EA_CLIENT}
        COPTS
        ${USER_CXX_FLAGS}
        "-Wno-strict-aliasing"
        DEPS
        ${CARBIN_DEPS_LINK}
        elasticann::common
        elasticann::proto-object
)]]