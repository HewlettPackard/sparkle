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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ALPS_ROOT}/src)
include_directories(${ALPS_ROOT}/include/alps)
include_directories(${ALPS_ROOT}/third-party/yaml-cpp-0.5.2/include)

## need to change which package's include, boost?
include_directories(/usr/local/include)

set_property(GLOBAL APPEND PROPERTY ALL_SHMMANAGEMENT_SRC
     ${CMAKE_CURRENT_SOURCE_DIR}/SharedMemoryManager.cc
)

get_property(TMP_ALL_SHMMANAGEMENT_SRC GLOBAL PROPERTY ALL_SHMMANAGEMENT_SRC)
add_library(shm_management SHARED ${TMP_ALL_SHMMANAGEMENT_SRC})

## set_target_properties (shm_management PROPERTIES VERSION 1.0)

## default to be installed to /usr/local/lib
install (TARGETS shm_management DESTINATION lib)

## later, to find automatically the following shared libraries.
target_link_libraries(shm_management ${ALPS_LIBRARY} glog)
