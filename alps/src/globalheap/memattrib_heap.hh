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

#ifndef _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HEAP_HH_
#define _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HEAP_HH_

#include <vector>

#include "alps/common/error_code.hh"
#include "alps/pegasus/relocatable_region.hh"
#include "alps/globalheap/memattrib.hh"

#include "globalheap/layout.hh"
#include "globalheap/lease.hh"

namespace alps {

// forward declarations
class ExtentHeap;
class ProcessSlabHeap;
class ThreadSlabHeap;

class MemAttribHeap {
public:
    const int kMaxThreadSlabHeaps = 32;

public:
    MemAttribHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation, const MemAttrib& memattrib)
        : nvheap_(nvheap),
          generation_(generation),
          memattrib_(memattrib)
    { }

    RRegion::TPtr<void> malloc(size_t size);
    void free(RRegion::TPtr<void> ptr);

    ErrorCode init();
    ErrorCode teardown();

    ThreadSlabHeap* thread_slab_heap();

private:
    RRegion::TPtr<nvHeap>        nvheap_;
    Generation                   generation_;
    MemAttrib                    memattrib_;
    ExtentHeap*                  extentheap_;
    ProcessSlabHeap*             process_slab_heap_;
    std::vector<ThreadSlabHeap*> thread_slab_heaps_;
};

} // namespace alps

#endif // _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HEAP_HH_
