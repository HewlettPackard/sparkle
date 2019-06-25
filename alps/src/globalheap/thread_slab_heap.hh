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

#ifndef _ALPS_GLOBALHEAP_THREAD_SLAB_HEAP_HH_
#define _ALPS_GLOBALHEAP_THREAD_SLAB_HEAP_HH_

#include <pthread.h>

#include "alps/pegasus/relocatable_region.hh"
#include "globalheap/slab_heap.hh"

namespace alps {

//forward declarations
class ProcessSlabHeap;

/*
 * @brief Per-thread slab heap
 */
class ThreadSlabHeap: public SlabHeap 
{
public:
    ThreadSlabHeap(ProcessSlabHeap* processheap);

    RRegion::TPtr<void> malloc(size_t size);

//private:
    ProcessSlabHeap*  process_slab_heap_;
};

} // namespace alps

#endif // _ALPS_GLOBALHEAP_THREAD_SLAB_HEAP_HH_
