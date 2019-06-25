# 
# (c) Copyright 2016 Hewlett Packard Enterprise Development LP
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
#

cmake_minimum_required(VERSION 3.0.2)
project(alps)
set(ALPS_PROJECT_VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(cmake/internal_utils.cmake)

# compiler flags for all build types
set(CMAKE_CXX_FLAGS "-std=c++11")

# compiler flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo
MinSizeRel."
       FORCE )
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message(STATUS "Configuring build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


# configuration based on target architecture
if(NOT TARGET_ARCH_MEM)
	set(TARGET_ARCH_MEM "CC-NUMA" CACHE STRING "" FORCE)
	message(STATUS "WARNING: Undefined target memory architecture.")
    message(STATUS "WARNING: You can define target memory architecture through command line option: -DTARGET_ARCH_MEM=target_arch_mem")
	message(STATUS "WARNING: Defaulting to: " ${TARGET_ARCH_MEM})
endif()
set_arch_conf(${TARGET_ARCH_MEM} ARCH_LIBS ARCH_DEFS)

# check for dependencies 
find_package(Boost 1.36.0 REQUIRED COMPONENTS serialization log system program_options filesystem thread)
find_package(Numa REQUIRED)
find_package(XATTR REQUIRED)

# check for target architecture dependencies
if(${TARGET_ARCH_MEM} MATCHES "NV-NCC-FAM")
	find_package(FamAtomic REQUIRED)
endif()

set(TARGETS)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# third-party libraries
set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/third-party/gtest)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(third-party)

include_directories(${PROJECT_SOURCE_DIR}/third-party/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/yaml-cpp-0.5.2/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/libbacktrace)

# library source 
add_subdirectory(src)

# examples source
add_subdirectory(examples)

# testing
enable_testing()
add_subdirectory(test)

###############################################################################
#
#                           Install headers/libraries
#
###############################################################################

set(CPACK_PACKAGE_VERSION ${ALPS_PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME "alps")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Haris Volos -- haris.volos@hpe.com")
set(CPACK_PACKAGE_VENDOR "Hewlett Packard Enterprise")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_COMPONENTS_ALL unspecified tests)
include(CPack)

feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES WHAT ALL)
