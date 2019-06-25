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

#include "alps/common/error_code.hh"

#include "globalheap/thread_slab_heap.hh"
#include "globalheap/process_slab_heap.hh"
#include "globalheap/slab_heap.hh"
#include "globalheap/size_class.hh"

namespace alps {

ThreadSlabHeap::ThreadSlabHeap(ProcessSlabHeap* process_slab_heap)
    : SlabHeap(NULL),
      process_slab_heap_(process_slab_heap)
{
 
}

RRegion::TPtr<void> ThreadSlabHeap::malloc(size_t size)
{
    RRegion::TPtr<void> ptr;
    const int szclass = sizeclass(size);

    lock(); 

    Slab* slab = find_slab(szclass);

    if (!slab) {
        // No slab in my thread-local slab heap so try to get a slab 
        // from the central process-wide slab heap
        slab = process_slab_heap_->acquire_slab(szclass);
        if (!slab) {
            LOG(warning) << "Out of memory";
            ptr = null_ptr;
        } else {
            insert_slab(slab, szclass);
        }
    }
    if (slab) {
        ptr = alloc_block(slab);
    }
    unlock();

    return ptr;
}


} // namespace alps
