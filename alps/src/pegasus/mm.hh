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

#ifndef _ALPS_PEGASUS_MEMORY_MANAGEMENT_HH_
#define _ALPS_PEGASUS_MEMORY_MANAGEMENT_HH_

#include "alps/common/error_code.hh"
#include "pegasus/invtbl.hh"

namespace alps {

// forward declarations
class MemoryManager;
class VmArea;

/**
 * \brief The memory manager provides a mechanism for directly mapping persistent 
 * region files to virtual memory regions.
 * 
 * The manager is intended for use by a per-region memory mapper (MemoryMap) 
 * that implements mapping policy.
 *
 * \todo Provide an API for picking address hints:
 * - best effort mapping: find the largest hole in the address space where we can map regions 
 *   this can guide segment map to pick the right segment size
 * - guide underlying OS by picking an address_hint based on our past knowledge about 
 *   persistent regions to minimize address space fragmentation similar to the Mnemosyne runtime
 *   (this could be implemented in the map method)
 */
class MemoryManager {
public:

    ErrorCode map(Region* region, off_t offset, size_t length, void* addr_hint, int prot, int flags, VmArea** vmarea);
    ErrorCode unmap(Region* region, void* addr, size_t length);
    //trans()
    ErrorCode rtrans(void* addr, Region** pregion, LinearAddr* offset);
    ErrorCode rtrans(uintptr_t addr, Region** pregion, LinearAddr* offset);
 
private:
    InvertedTable invtbl_;
};


} // namespace alps

#endif // _ALPS_PEGASUS_MEMORY_MANAGEMENT_HH_
