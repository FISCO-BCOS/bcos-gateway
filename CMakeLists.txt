#------------------------------------------------------------------------------
# Top-level CMake file for bcos-gateway
# ------------------------------------------------------------------------------
# Copyright (C) 2021 bcos-gateway
# SPDX-License-Identifier: Apache-2.0
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

include(FetchContent)
if (NOT DEFINED URL_BASE)
    set(URL_BASE "github.com")
    # set(URL_BASE "github.com.cnpmjs.org")
endif()
set(BCOS_CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bcos-cmake-scripts)
FetchContent_Declare(bcos-cmake-scripts
    GIT_REPOSITORY https://${URL_BASE}/FISCO-BCOS/bcos-cmake-scripts.git
    GIT_TAG dev
    SOURCE_DIR ${BCOS_CMAKE_SCRIPTS_DIR}
)
FetchContent_MakeAvailable(bcos-cmake-scripts)

set(BCOS_GATEWAY_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "The cmake path for the library")
list(APPEND CMAKE_MODULE_PATH ${BCOS_CMAKE_SCRIPTS_DIR})
list(APPEND CMAKE_MODULE_PATH ${BCOS_GATEWAY_CMAKE_DIR})
#init hunter
include(HunterGate)
HunterGate(
    URL "https://${URL_BASE}/ywy2090/hunter/archive/eff2760a468cdb92c4fb133fcecf16d358820a5a.tar.gz"
    SHA1 "277a9dd8af36a0bd1212d8b2f2964934c711e487"
    FILEPATH "${CMAKE_SOURCE_DIR}/cmake/config.cmake"
)
project(bcos-gateway VERSION "3.0.0")
set(VERSION_SUFFIX "")

# basic settings
include(Options)
configure_project()
include(CompilerSettings)

include_directories(${CMAKE_INSTALL_INCLUDEDIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# install dependencies
include(InstallBcosFrameworkDependencies)
include(InstallBcosCryptoDependencies)

set(NETWORK_TARGET "network")
set(P2P_TARGET "p2p")
set(BCOS_GATEWAY_TARGET "bcos-gateway")

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

hunter_add_package(jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)

add_subdirectory(bcos-gateway)

# ut
if (TESTS)
    hunter_add_package(bcos-front)
    find_package(bcos-front CONFIG REQUIRED)

    enable_testing()
    set(CTEST_OUTPUT_ON_FAILURE TRUE)
    add_subdirectory(test)
endif()

include(InstallConfig)

# install bcos gateway target
install(
    TARGETS ${BCOS_GATEWAY_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    TARGETS ${NETWORK_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    TARGETS ${P2P_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# install the include files for hash
include(InstallInclude)

# for doxygen
include(BuildDocs)

# for code coverage
if (COVERAGE)
    include(Coverage)
    config_coverage("gateway-coverage" "'/usr*' '${CMAKE_CURRENT_SOURCE_DIR}/bcos-cmake-scripts*' '${CMAKE_SOURCE_DIR}/test/mock**' '${CMAKE_SOURCE_DIR}/test/main**'")
endif ()
