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

include_directories(${PROJECT_SOURCE_DIR}/third-party/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
#SET(CMAKE_INSTALL_RPATH "build/src/alps")
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

add_definitions(${ARCH_DEFS})

add_subdirectory(unit)
#add_subdirectory(integration)


#add_executable(test-util ${CMAKE_CURRENT_SOURCE_DIR}/util.cc)
#target_link_libraries(test-util pthread ${GTEST_LIBRARY} alps boost_program_options gtest)

#install (TARGETS test-util DESTINATION bin)
