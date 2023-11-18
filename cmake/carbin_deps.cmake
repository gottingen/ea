#
# Copyright 2023 The Carbin Authors.
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

if (CARBIN_BUILD_TEST)
    enable_testing()
    #include(require_gtest)
    #include(require_gmock)
endif (CARBIN_BUILD_TEST)

if (CARBIN_BUILD_BENCHMARK)
    include(require_benchmark)
endif ()

set(CARBIN_SYSTEM_DYLINK)
if (APPLE)
    find_library(CoreFoundation CoreFoundation)
    list(APPEND CARBIN_SYSTEM_DYLINK ${CoreFoundation} pthread)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND CARBIN_SYSTEM_DYLINK rt dl pthread)
endif ()




SET(THIRD_PARTY_PATH ${PROJECT_SOURCE_DIR}/third-deps)
SET(THIRD_PARTY_BUILD_TYPE Release)
SET(EXTERNAL_PROJECT_LOG_ARGS
        LOG_DOWNLOAD 0
        LOG_UPDATE 1
        LOG_CONFIGURE 0
        LOG_BUILD 0
        LOG_TEST 1
        LOG_INSTALL 0)

include(ProcessorCount)
ProcessorCount(NUM_OF_PROCESSOR)
message(NUM_OF_PROCESSOR: ${NUM_OF_PROCESSOR})
#thread
include(FindThreads)

#openssl
find_package(OpenSSL REQUIRED)

message(STATUS "ssl:" ${OPENSSL_SSL_LIBRARY})
message(STATUS "crypto:" ${OPENSSL_CRYPTO_LIBRARY})

ADD_LIBRARY(ssl SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET ssl PROPERTY IMPORTED_LOCATION ${OPENSSL_SSL_LIBRARY})

ADD_LIBRARY(crypto SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_CRYPTO_LIBRARY})

#zlib
if (NOT CARBIN_WITH_SYSTEM_LIBS)
    include(zlib)
else ()
    ADD_LIBRARY(zlib SHARED IMPORTED GLOBAL)
    SET(ZLIB_LIBRARIES z)
endif ()

#bluebird
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(TURBO_INCLUDE_DIR NAMES turbo/version.h)
    find_library(TURBO_LIBRARIES NAMES libturbo.a turbo)
    if ((NOT TURBO_INCLUDE_DIR) OR (NOT TURBO_LIBRARIES))
        message(FATAL_ERROR "Fail to find turbo")
    endif ()
    ADD_LIBRARY(turbo SHARED IMPORTED GLOBAL)
else ()
    include(turbo)
endif ()

#bluebird
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(BLUEBIRD_INCLUDE_DIR NAMES bluebird/bits/bitmap.h)
    find_library(BLUEBIRD_LIBRARIES NAMES libbits.a bits)
    if ((NOT BLUEBIRD_INCLUDE_DIR) OR (NOT BLUEBIRD_LIBRARIES))
        message(FATAL_ERROR "Fail to find bluebird")
    endif ()
    ADD_LIBRARY(bluebird SHARED IMPORTED GLOBAL)
else ()
    include(bluebird)
endif ()

#gflags
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(GFLAGS_INCLUDE_DIR NAMES gflags/gflags.h)
    find_library(GFLAGS_LIBRARIES NAMES gflags)
    if ((NOT GFLAGS_INCLUDE_DIR) OR (NOT GFLAGS_LIBRARIES))
        message(FATAL_ERROR "Fail to find gflags")
    endif ()
    ADD_LIBRARY(gflags SHARED IMPORTED GLOBAL)
else ()
    include(gflags)
endif ()

#snappy
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(SNAPPY_INCLUDE_DIR NAMES snappy.h)
    find_library(SNAPPY_LIBRARIES NAMES snappy)
    if ((NOT SNAPPY_INCLUDE_DIR) OR (NOT SNAPPY_LIBRARIES))
        message(FATAL_ERROR "Fail to find snappy")
    endif ()
    ADD_LIBRARY(snappy SHARED IMPORTED GLOBAL)
else ()
    include(snappy)
endif ()
#re2
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(RE2_INCLUDE_DIR NAMES re2/re2.h)
    find_library(RE2_LIBRARIES NAMES re2)
    if ((NOT RE2_INCLUDE_DIR) OR (NOT RE2_LIBRARIES))
        message(FATAL_ERROR "Fail to find re2")
    endif ()
    ADD_LIBRARY(re2 SHARED IMPORTED GLOBAL)
else ()
    include(re2)
endif ()
#protobuf
find_package(Protobuf REQUIRED)

#include(require_turbo)
#rocksdb
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(ROCKSDB_INCLUDE_DIR NAMES rocksdb/db.h)
    find_library(ROCKSDB_LIBRARIES NAMES rocksdb)
    if ((NOT ROCKSDB_INCLUDE_DIR) OR (NOT ROCKSDB_LIBRARIES))
        message(FATAL_ERROR "Fail to find rocksdb")
    endif ()
    ADD_LIBRARY(rocksdb SHARED IMPORTED GLOBAL)
else ()
    include(rocksdb)
endif ()
#[[
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(MARIADB_INCLUDE_DIR NAMES mariadb/mysql.h)
    if (MARIADB_INCLUDE_DIR)
        set(MARIADB_INCLUDE_DIR ${MARIADB_INCLUDE_DIR}/mariadb)
    else ()
        find_path(MARIADB_INCLUDE_DIR NAMES mysql.h)
    endif ()
    find_library(MARIADB_LIBRARIES NAMES mariadbclient)
    if ((NOT MARIADB_INCLUDE_DIR) OR (NOT MARIADB_LIBRARIES))
        message(FATAL_ERROR "Fail to find mariadbclient")
    endif ()
    ADD_LIBRARY(mariadb SHARED IMPORTED GLOBAL)
else ()
    include(mariadb)
endif ()
message(mysqlclient: ${MARIADB_INCLUDE_DIR}, ${MARIADB_LIBRARIES})
]]
#brpc
if (CARBIN_WITH_SYSTEM_LIBS)
    #leveldb(for brpc)
    find_library(LEVELDB_LIBRARIES NAMES leveldb)
    if (NOT LEVELDB_LIBRARIES)
        message(FATAL_ERROR "Fail to find leveldb")
    endif ()

    find_path(BRPC_INCLUDE_DIR NAMES brpc/server.h)
    find_library(BRPC_LIBRARIES NAMES libbrpc.a brpc)
    if ((NOT BRPC_INCLUDE_DIR) OR (NOT BRPC_LIBRARIES))
        message(FATAL_ERROR "Fail to find brpc")
    endif ()
else ()
    include(leveldb)
    include(brpc)
endif ()
message(BRPC:${BRPC_INCLUDE_DIR}, ${BRPC_LIBRARIES})
#braft
if (CARBIN_WITH_SYSTEM_LIBS)
    find_path(BRAFT_INCLUDE_DIR NAMES braft/raft.h)
    find_library(BRAFT_LIBRARIES NAMES libbraft.a brpc)
    if ((NOT BRAFT_INCLUDE_DIR) OR (NOT BRAFT_LIBRARIES))
        message(FATAL_ERROR "Fail to find braft")
    endif ()
else ()
    include(braft)
endif ()
message(braft lib : ${BRAFT_LIBRARIES})

include_directories(${ROCKSDB_INCLUDE_DIR})

#message(STATUS "ssl:" ${OPENSSL_SSL_LIBRARY})
#message(STATUS "crypto:" ${OPENSSL_CRYPTO_LIBRARY})
find_package(eaproto REQUIRED)
SET(DEP_INC
        ${CMAKE_CURRENT_BINARY_DIR}/proto

        ${OPENSSL_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        #        ${BZ2_INCLUDE_DIR}
        ${BLUEBIRD_INCLUDE_DIR}

        ${PROTOBUF_INCLUDE_DIR}
        ${GFLAGS_INCLUDE_DIR}
        ${SNAPPY_INCLUDE_DIR}
        ${RE2_INCLUDE_DIR}
        ${TURBO_INCLUDE_DIR}
        ${ROCKSDB_INCLUDE_DIR}
        ${MARIADB_INCLUDE_DIR}
        ${BRPC_INCLUDE_DIR}
        ${BRAFT_INCLUDE_DIR}

        ${GPERFTOOLS_INCLUDE_DIR}
        )
include_directories(${DEP_INC})
set(CARBIN_DEPS_LINK
        ${TURBO_LIBRARIES}
        ${BRAFT_LIBRARIES}
        ${BRPC_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
        ${LEVELDB_LIBRARIES}
        ${ROCKSDB_LIBRARIES}
        ${SNAPPY_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${GFLAGS_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${RE2_LIBRARIES}
        ${BLUEBIRD_LIBRARIES}
        ${LEVELDB_LIBRARIES}
        ${CARBIN_SYSTEM_DYLINK}
        )






