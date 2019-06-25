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
include_directories(${PROJECT_SOURCE_DIR}/third-party/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${NUMA_INCLUDE_DIR})
include_directories(${XATTR_INCLUDE_DIR})

add_definitions(-DHAVE_BOOST_FILESYSTEM)
add_definitions(${ARCH_DEFS})

# Collect all cc files in the main source folder.
set_property(GLOBAL PROPERTY ALL_ALPS_SRC "")

# common library
add_subdirectory(common)

# persistent global address space library
add_subdirectory(pegasus)

# persistent global heap library
add_subdirectory(globalheap)

get_property(TMP_ALL_ALPS_SRC GLOBAL PROPERTY ALL_ALPS_SRC)
add_library(alps SHARED ${TMP_ALL_ALPS_SRC})

target_link_libraries(alps
	${NUMA_LIBRARY}
	${XATTR_LIBRARY}
	${Boost_SERIALIZATION_LIBRARY}
	${Boost_LOG_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SERIALIZATION_LIBRARY}
	backtrace
	yaml-cpp
)
target_link_libraries(alps ${ARCH_LIBS})
 
list (APPEND TARGETS alps)

install (TARGETS ${TARGETS} DESTINATION lib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/alps" DESTINATION include)
