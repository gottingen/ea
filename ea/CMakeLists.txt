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
file(GLOB ENGINE engine/*.cc)
file(GLOB BASE base/*.cc)

set(EA_COMMON
        ${BASE}
        ${ENGINE}
        ${RAFT}
        ${RPC}
        )

add_definitions(
        -D_GNU_SOURCE
        -D__STDC_FORMAT_MACROS
        -D__STDC_LIMIT_MACROS
        -D__STDC_CONSTANT_MACROS
        -D__const__=unused
        -DBRPC_WITH_GLOG=OFF
)

file(GLOB EA_FLAGS flags/*.cc)
carbin_cc_library(
        NAMESPACE ea
        NAME flags
        SOURCES
        ${EA_FLAGS}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        ${CARBIN_DEPS_LINK}
        PUBLIC
)

carbin_cc_library(
        NAMESPACE ea
        NAME common
        SOURCES
        ${EA_COMMON}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eapi::eapi
        ea::flags
        ${CARBIN_DEPS_LINK}
        PUBLIC
)

file(GLOB EA_CLIENT client/*.cc)

carbin_cc_library(
        NAMESPACE ea
        NAME client
        SOURCES
        ${EA_CLIENT}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eapi::eapi
        ea::common
        ea::flags
        ${CARBIN_DEPS_LINK}
        PUBLIC
)


file(GLOB META_SERVER discovery/*.cc)

carbin_cc_binary(
        NAMESPACE ea
        NAME eadiscovery
        SOURCES
        ${META_SERVER}
        COPTS
        ${USER_CXX_FLAGS}
        DEPS
        eapi::eapi
        ea::common
        ea::flags
        ea::client
        ${CARBIN_DEPS_LINK}
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
        ea::client
        eapi::eapi
        PUBLIC
)
