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

function (add_layers_test targetname) 
  add_alps_test(${targetname})
  target_link_libraries(${targetname} alps boost_thread boost_filesystem)
endfunction()

add_layers_test(test_bitmap)
add_layers_test(test_extentheap)
add_layers_test(test_extentinterval)
add_layers_test(test_extentmap)
add_layers_test(test_freespacemap)
add_layers_test(test_hybridheap)
add_layers_test(test_pointer)
add_layers_test(test_slab)
add_layers_test(test_slabheap)
