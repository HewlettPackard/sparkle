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

#ifndef _ALPS_GLOBALHEAP_EXTENTHEAP_HH_
#define _ALPS_GLOBALHEAP_EXTENTHEAP_HH_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <map>

#include "alps/common/error_code.hh"
#include "alps/globalheap/memattrib.hh"

#include "globalheap/layout.hh"
#include "globalheap/extent.hh"
#include "globalheap/zone.hh"
#include "globalheap/zone_heap.hh"

namespace alps {

//forward declarations
class FreeSpaceMap;

class NullInsertFunctor {
public:
    void operator()(Zone* zone, Extent* extent)
    {
        // do nothing
    }
};


/**
 * @brief Allocates extents from a pool of zones
 */
class ExtentHeap: public ZoneHeap {
public:
    ExtentHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation);
    ExtentHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation, const MemAttrib& memattrib);

    ErrorStack init();

    template<typename T> ErrorCode malloc(size_t size, bool can_extend, T enumerate_callback, RRegion::TPtr<nvExtentHeader>* nvexheader, RRegion::TPtr<void>* nvex);
    RRegion::TPtr<void> malloc(size_t size);
    void free(Zone* zone, RRegion::TPtr<void> nvex);
    void free(RRegion::TPtr<void> nvex);
    template<typename T> void free(RRegion::TPtr<void> nvex, T enumerate_callback);

    template<typename T> Zone* acquire_zone(size_t zone_id, T enumerate_callback_functor);

    /**
     * @brief Acquire nzones zones and invoke functor callback for each zone acquired.
     * 
     */ 
    template<typename T> int more_space(int nzones, T callback);
    
private:
    /**
     * @brief Allocate an extent from zones already owned by this extent heap.
     * 
     * @param[in] size_nblocks size of extent in number of blocks
     * @param[in] zone_hint hint to a zone that might have free space
     * @param[out] nvex a pointer to the extent 
     * @param[out] zone the zone we allocated extent from
     */
    ErrorCode alloc_extent(size_t size_nblocks, Zone* zone_hint, RRegion::TPtr<nvExtentHeader>* nvexheader, RRegion::TPtr<void>* nvex, Zone** pzone);

private:
    pthread_mutex_t  mutex_;
    Zone*            last_alloc_zone_; // zone that served the latest allocation request
};

template<typename T> 
Zone* ExtentHeap::acquire_zone(size_t zone_id, T enumerate_callback_functor)
{
    return ZoneHeap::acquire_zone(false, zone_id, 0, enumerate_callback_functor);
}


template<typename T>
int ExtentHeap::more_space(int nzones, T enumerate_callback_functor)
{
    Zone*               zone;
    RRegion::TPtr<void> nvex;

    for (int i=0; i<nzones; i++) {
        zone = ZoneHeap::acquire_new_zone(enumerate_callback_functor);
        if (!zone) {
            return i;
        }
    }
    return nzones;
}


template<typename T>
ErrorCode ExtentHeap::malloc(size_t size_bytes, bool can_extend, T callback, RRegion::TPtr<nvExtentHeader>* nvexheader, RRegion::TPtr<void>* nvex)
{
    ErrorCode rc;
    Zone*     zone;

    LOG(info) << "malloc";

    pthread_mutex_lock(&mutex_);

    // round up to next multiple of block_size
    size_t size_nblocks = size_bytes / nvZone::block_size() + (size_bytes % nvZone::block_size() ? 1: 0);

    // ALLOCATION POLICY: 
    // Try to allocate from zones that the extent-heap already owns. 
    // If that fails then find a zone from the backend zoneheap with sufficient 
    // space to satisfy the allocation request. Return back a zone that  didn't 
    // have enough space to satisfy the request. 
    if ((rc = alloc_extent(size_nblocks, last_alloc_zone_, nvexheader, nvex, &zone)) != kErrorCodeOk) {
        if (can_extend) {
            LOG(info) << "acquire_new_zone";
            zone = ZoneHeap::acquire_new_zone(size_nblocks, callback);
            if (zone) {
                LOG(info) << "acquire_new_zone: zone_id: " << zone->zone_id();
                rc = zone->alloc_extent(size_nblocks, nvexheader, nvex);
                ASSERT_ND(rc == kErrorCodeOk);
            } else {
                LOG(info) << "acquire_new_zone: zone_id: null";
                rc = kErrorCodeOutofmemory;
            }
        } else {
            rc = kErrorCodeOutofmemory;
        }
    }
    if (rc == kErrorCodeOk) {
        last_alloc_zone_ = zone;
    }
    pthread_mutex_unlock(&mutex_);
    return rc;
}


inline RRegion::TPtr<void> ExtentHeap::malloc(size_t size_bytes) 
{
    RRegion::TPtr<nvExtentHeader> nvexheader;
    RRegion::TPtr<void> nvex;

    if (malloc(size_bytes, true, NullEnumerateFunctor(), &nvexheader, &nvex) != kErrorCodeOk) {
        return null_ptr;
    }
    return nvex;
}


template<typename T>
void ExtentHeap::free(RRegion::TPtr<void> nvex, T callback)
{
    pthread_mutex_lock(&mutex_);

    // find the extent's zone
    size_t metazone_size = nvheap_->metazone_size();
    size_t zone_id = (nvex.offset() & ~(metazone_size - 1)) / metazone_size;
    Zone* zone = acquire_zone(zone_id, callback);
    if (zone) {
        zone->free_extent(nvex);
    } else {
        LOG(error) << "Attempt to free unknown address: " << nvex << std::endl;
    }
    pthread_mutex_unlock(&mutex_);
}

inline void ExtentHeap::free(RRegion::TPtr<void> nvex)
{
    return free(nvex, NullEnumerateFunctor());
}


} // namespace alps

#endif // _ALPS_GLOBALHEAP_EXTENTHEAP_HH_
