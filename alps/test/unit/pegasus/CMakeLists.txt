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

function (add_pegasus_test targetname)
  add_alps_test(${targetname})
endfunction()

function (add_pegasus_test_tmpfs_only targetname)
  add_alps_test_tmpfs_only(${targetname})
endfunction()

##add_alps_test(test_inverted_table)
add_pegasus_test(test_multi_region_file)
add_pegasus_test(test_pegas)
add_pegasus_test(test_pointer)
add_pegasus_test(test_region_file)
add_pegasus_test_tmpfs_only(test_tmpfs_region_file)
add_pegasus_test(test_topology)
