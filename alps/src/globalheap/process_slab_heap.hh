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

#ifndef _ALPS_GLOBALHEAP_PROCESS_SLAB_HEAP_HH_
#define _ALPS_GLOBALHEAP_PROCESS_SLAB_HEAP_HH_

#include <pthread.h>

#include "alps/pegasus/relocatable_region.hh"
#include "globalheap/slab_heap.hh"
#include "globalheap/extentheap.hh"

namespace alps {


/**
 * @brief Central per-process slab heap 
 */
class ProcessSlabHeap: public SlabHeap 
{
public:
    ProcessSlabHeap(ExtentHeap* extentheap)
        : SlabHeap(extentheap)
    { }

    /**
     * @brief Returns a slab that supports sizeclass \a szclass.
     */
    Slab* acquire_slab(int szclass);
    RRegion::TPtr<void> malloc(size_t size);
    void free(RRegion::TPtr<void> ptr);
};

} // namespace alps

#endif // _ALPS_GLOBALHEAP_PROCESS_SLAB_HEAP_HH_
