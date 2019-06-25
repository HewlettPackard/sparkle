/* 
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ALPS_PEGASUS_VMAREA_HH_
#define _ALPS_PEGASUS_VMAREA_HH_

#include <iostream>
#include "alps/pegasus/addr.hh"

namespace alps {

class VmArea {
public:
    VmArea(Region* region, LinearAddr offset, uintptr_t vm_start, uintptr_t vm_end)
        : region_(region),
          offset_(offset),
          vm_start_(vm_start),
          vm_end_(vm_end)
    { }

    Region* region() 
    { return region_; }

    LinearAddr offset()
    { return offset_; }

    uintptr_t vm_start() const 
    { return vm_start_; }

    uintptr_t vm_end() const
    { return vm_end_; }

private:
    Region*    region_;     // pointer to the persistent region owning the memory region
    LinearAddr offset_;      // offset in persistent region
    uintptr_t  vm_start_;    // first virtual address inside the region
    uintptr_t  vm_end_;      // first virtual address after the region
};

} // namespace alps

#endif // _ALPS_PEGASUS_VMAREA_HH_
