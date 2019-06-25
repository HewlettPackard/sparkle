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

include_directories(${PROJECT_SOURCE_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/test/unit)

function (__build_and_install_alps_test targetname srcname binname)
  add_executable(${targetname} ${CMAKE_CURRENT_SOURCE_DIR}/${srcname}.cc)
  set_target_properties(${targetname} PROPERTIES OUTPUT_NAME ${binname})
  target_link_libraries(${targetname} pthread gtest alps)
  install (TARGETS ${targetname} DESTINATION bin/test/unit COMPONENT test)
endfunction()

function (__add_alps_test targetname srcname binname)
  __build_and_install_alps_test(${targetname} ${targetname} ${targetname})
  add_test(NAME tmpfs_${targetname} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${binname})
  #add_test(NAME lfs_${targetname} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${binname} --test_dir=/lfs)
endfunction()

function (__add_alps_test_tmpfs_only targetname srcname binname)
  __build_and_install_alps_test(${targetname} ${targetname} ${targetname})
  add_test(NAME tmpfs_${targetname} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${binname})
endfunction()

function (__add_alps_test_lfs_only targetname srcname binname)
  __build_and_install_alps_test(${targetname} ${targetname} ${targetname})
  add_test(NAME lfs_${targetname} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${binname} --test_dir=/lfs)
endfunction()

function (add_alps_test targetname)
  __add_alps_test(${targetname} ${targetname} ${targetname})
endfunction()

function (add_alps_test_tmpfs_only targetname)
  __add_alps_test_tmpfs_only(${targetname} ${targetname} ${targetname})
endfunction()

function (add_alps_test_lfs_only targetname)
  __add_alps_test_lfs_only(${targetname} ${targetname} ${targetname})
endfunction()


add_subdirectory(common)
#add_subdirectory(pegasus)
add_subdirectory(layers)
