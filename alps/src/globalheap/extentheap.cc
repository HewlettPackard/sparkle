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

#include "globalheap/extentheap.hh"

#include "alps/common/error_code.hh"

#include "common/debug.hh"
#include "globalheap/freespacemap.hh"
#include "globalheap/zone.hh"
#include "globalheap/zone_heap.hh"

namespace alps {

ExtentHeap::ExtentHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation)
    : ZoneHeap(nvheap, generation),
      last_alloc_zone_(NULL) 
{ 
    assert(pthread_mutex_init(&mutex_, 0) == 0);
}

ExtentHeap::ExtentHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation, const MemAttrib& memattrib)
    : ZoneHeap(nvheap, generation, memattrib),
      last_alloc_zone_(NULL) 
{ 
    assert(pthread_mutex_init(&mutex_, 0) == 0);
}


ErrorStack ExtentHeap::init()
{
    return ZoneHeap::init();
}


ErrorCode ExtentHeap::alloc_extent(size_t size_nblocks, Zone* zone_hint, RRegion::TPtr<nvExtentHeader>* nvexheader, RRegion::TPtr<void>* nvex, Zone** pzone)
{
    RRegion::TPtr<void> ptr;

    // follow hint first
    if (zone_hint) {
        if (zone_hint->alloc_extent(size_nblocks, nvexheader, nvex) == kErrorCodeOk) {
            *pzone = zone_hint;
            return kErrorCodeOk;
        }
    }
    // hint failed; iterate through all available zones till we find a fit
    for (ZoneSet::iterator it = zones_.begin();
         it != zones_.end();
         it++) 
    {
        Zone* zone = *it;
        if (zone->alloc_extent(size_nblocks, nvexheader, nvex) == kErrorCodeOk) {
            *pzone = zone;
            return kErrorCodeOk;
        }
    }
    return kErrorCodeOutofmemory;
}


void ExtentHeap::free(Zone* zone, RRegion::TPtr<void> nvex)
{
    pthread_mutex_lock(&mutex_);
    zone->free_extent(nvex);
    pthread_mutex_unlock(&mutex_);
}

} // namespace alps
