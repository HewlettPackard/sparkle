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

#include "globalheap/memattrib_heap.hh"

#include "alps/common/error_code.hh"
#include "alps/globalheap/memattrib.hh"

#include "globalheap/extentheap.hh"
#include "globalheap/process_slab_heap.hh"
#include "globalheap/thread_slab_heap.hh"

namespace alps {

ErrorCode MemAttribHeap::init()
{
    extentheap_ = new ExtentHeap(nvheap_, generation_, memattrib_);
    COERCE_ERROR(extentheap_->init());
    process_slab_heap_ = new ProcessSlabHeap(extentheap_);
    ASSERT_ND(extentheap_ != NULL);
    ASSERT_ND(process_slab_heap_ != NULL);

    for (int i=0; i<kMaxThreadSlabHeaps; i++)
    {
        ThreadSlabHeap* th = new ThreadSlabHeap(process_slab_heap_);
        thread_slab_heaps_.push_back(th);
    }
    return kErrorCodeOk;
}

ErrorCode MemAttribHeap::teardown()
{
    COERCE_ERROR(extentheap_->teardown());
    delete extentheap_;
    return kErrorCodeOk;
}

ThreadSlabHeap* MemAttribHeap::thread_slab_heap()
{
    int idx = pthread_self() % kMaxThreadSlabHeaps;
    return thread_slab_heaps_[idx];
}

RRegion::TPtr<void> MemAttribHeap::malloc(size_t size)
{
    LOG(info) << "Allocate block size: " << size;

    // large block
    if (size > slab_size/2) {
        return process_slab_heap_->malloc(size);
    }

    // small block
    ThreadSlabHeap* theap = thread_slab_heap();
    return theap->malloc(size);
}

void MemAttribHeap::free(RRegion::TPtr<void> ptr)
{
    LOG(info) << "Free block ptr: " << ptr;

    return process_slab_heap_->free(ptr);
}


} // namespace alps
