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

function (add_globalheap_test targetname) 
  add_alps_test(${targetname})
  target_link_libraries(${targetname} alps ${Boost_THREAD_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
endfunction()

add_globalheap_test(test_bitmap)
add_globalheap_test(test_extent)
add_globalheap_test(test_extentmap)
add_globalheap_test(test_extentheap)
add_globalheap_test(test_freespacemap)
add_globalheap_test(test_globalheap)
add_globalheap_test(test_memattrib)
add_globalheap_test(test_root)
add_globalheap_test(test_slab)
add_globalheap_test(test_slab_heap)
add_globalheap_test(test_stress)
add_globalheap_test(test_thread_slab_heap)
add_globalheap_test(test_zone)
add_globalheap_test(test_zone_heap)
#add_globalheap_test(test_large)
