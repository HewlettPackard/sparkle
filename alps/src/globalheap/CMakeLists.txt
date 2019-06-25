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
    ${CMAKE_CURRENT_SOURCE_DIR}/extentheap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/extentmap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/freespacemap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/globalheap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/globalheap_internal.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/helper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/lease.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/memattrib_heap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/process_slab_heap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/size_class.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/slab.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/slab_heap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_slab_heap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/zone.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/zone_heap.cc
)

add_executable(globalheap-util ${CMAKE_CURRENT_SOURCE_DIR}/util.cc)
target_link_libraries(globalheap-util pthread ${GTEST_LIBRARY} alps ${Boost_PROGRAM_OPTIONS_LIBRARY})

install (TARGETS globalheap-util DESTINATION bin)
