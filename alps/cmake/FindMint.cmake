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

# Find the holodeck mint library.
# Output variables:
#  MINT_INCLUDE_DIR : e.g., /usr/include/.
#  MINT_LIBRARY     : Library path of mint library
#  MINT_FOUND       : True if found.

set(HOLODECK_DIR /home/volos/workspace/holodeck) 

find_path(MINT_INCLUDE_DIR NAME mint/mint.h
  HINTS $ENV{HOME}/local/include /opt/local/include /usr/local/include /usr/include ${HOLODECK_DIR}/include/holodeck
)

find_library(MINT_LIBRARY NAME mint
  HINTS $ENV{HOME}/local/lib64 $ENV{HOME}/local/lib /usr/local/lib64 /usr/local/lib /opt/local/lib64 /opt/local/lib /usr/lib64 /usr/lib ${HOLODECK_DIR}/build/src/mint
)

if (MINT_INCLUDE_DIR AND MINT_LIBRARY)
    set(MINT_FOUND TRUE)
    message(STATUS "Found mint library: inc=${MINT_INCLUDE_DIR}, lib=${MINT_LIBRARY}")
else ()
    set(NUMA_FOUND FALSE)
    message(STATUS "WARNING: mint library not found.")
endif ()
