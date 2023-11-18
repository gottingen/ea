# Copyright (c) 2020-present Baidu, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

INCLUDE(ExternalProject)

SET(BLUEBIRD_SOURCES_DIR ${THIRD_PARTY_PATH}/bluebird)
SET(BLUEBIRD_INSTALL_DIR ${THIRD_PARTY_PATH}/install/bluebird)
SET(BLUEBIRD_INCLUDE_DIR "${BLUEBIRD_INSTALL_DIR}/include" CACHE PATH "bluebird include directory." FORCE)
SET(BLUEBIRD_LIBRARIES "${BLUEBIRD_INSTALL_DIR}/lib/libbits.a" CACHE FILEPATH "bluebird library." FORCE)

ExternalProject_Add(
        extern_bluebird
        ${EXTERNAL_PROJECT_LOG_ARGS}
        GIT_REPOSITORY "https://github.com/gottingen/bluebird.git"
        GIT_TAG "v0.3.0"
        PREFIX ${BLUEBIRD_SOURCES_DIR}
        UPDATE_COMMAND ""
        CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
        -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
        -DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
        -DCMAKE_INSTALL_PREFIX=${BLUEBIRD_INSTALL_DIR}
        -DCMAKE_INSTALL_LIBDIR=${BLUEBIRD_INSTALL_DIR}/lib
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCARBIN_BUILD_TEST=OFF
        -DCARBIN_BUILD_BENCHMARK=OFF
        -DCARBIN_BUILD_EXAMPLES=OFF
        -DCARBIN_USE_CXX11_ABI=ON
        -DBUILD_SHARED_LIBRARY=OFF
        -DBUILD_STATIC_LIBRARY=ON
        -DCMAKE_BUILD_TYPE=${THIRD_PARTY_BUILD_TYPE}
        ${EXTERNAL_OPTIONAL_ARGS}
        CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${BLUEBIRD_INSTALL_DIR}
        -DCMAKE_INSTALL_LIBDIR:PATH=${BLUEBIRD_INSTALL_DIR}/lib
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
        -DCMAKE_BUILD_TYPE:STRING=${THIRD_PARTY_BUILD_TYPE}
)

ADD_LIBRARY(bluebird STATIC IMPORTED GLOBAL)
SET_PROPERTY(TARGET bluebird PROPERTY IMPORTED_LOCATION ${BLUEBIRD_LIBRARIES})
ADD_DEPENDENCIES(bluebird extern_bluebird)
