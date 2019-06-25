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

set_property(GLOBAL APPEND PROPERTY ALL_ALPS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/address_space.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/address_space_options.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/helper_functions.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/lfs_options.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/lfs_region_file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/lfs_topology.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mm.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/multi_region_file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/pegasthread.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/pegasus_options.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/pegasus.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/region.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/region_file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/region_file_factory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/segmap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tmpfs_options.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tmpfs_region_file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tmpfs_topology.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/topology.cc
)
