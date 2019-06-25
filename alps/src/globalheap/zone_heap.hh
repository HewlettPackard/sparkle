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

#ifndef _ALPS_GLOBALHEAP_ZONEHEAP_HH_
#define _ALPS_GLOBALHEAP_ZONEHEAP_HH_

#include <pthread.h>
#include <set>

#include "alps/globalheap/memattrib.hh"

#include "globalheap/layout.hh"
#include "globalheap/lease.hh"
#include "globalheap/rwlock.hh"
#include "globalheap/zone.hh"

namespace alps {

//TODO: ZoneHeap should keep track zone groups that have the same memory attributes
//TODO: acquire_zone API should accept a memory attribute hint that indicates 
//      from which interleave group to allocate memory from



/**
 * @brief A heap for allocating memory zones
 * 
 * @details
 * ZoneHeap is not thread safe. User has to serialize requests to a ZoneHeap
 * object. 
 * 
 * Zones acquired by a ZoneHeap are not released until process termination. 
 *
 */
class ZoneHeap {
protected:
    typedef std::set<Zone*> ZoneSet;

public:
    ZoneHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation)
        : nvheap_(nvheap),
          generation_(generation),
          memattrib_(0)
    { }

    ZoneHeap(RRegion::TPtr<nvHeap> nvheap, Generation generation, const MemAttrib& memattrib)
        : nvheap_(nvheap),
          generation_(generation),
          memattrib_(memattrib)
    { }

    ErrorStack init();
    ErrorStack teardown();

    template<typename T> Zone* acquire_zone(bool new_zone, size_t zone_id, size_t min_free_blocks, T enumerate_callback_functor);
    template<typename T> Zone* acquire_new_zone(size_t min_free_blocks, T enumerate_callback_functor);
    template<typename T> Zone* acquire_new_zone(T enumerate_callback_functor);
    Zone* acquire_new_zone();

    void release_zone(Zone* zone, bool remove = true);

    RRegion::TPtr<nvZone> nvzone(RRegion::TPtr<void> ptr);
    RRegion::TPtr<nvZone> nvzone(size_t zone_id);
    MemAttrib nvzone_memattrib(size_t zone_id);
    Zone* zone(size_t zone_id);
    RRegion::TPtr<nvHeap> nvheap()
    {
        return nvheap_;
    }

protected:
    ReaderWriterLock      rwlock_; // serializes zone acquisition
    RRegion::TPtr<nvHeap> nvheap_;
    Generation            generation_;
    MemAttrib             memattrib_;
    ZoneSet               zones_;
};

inline RRegion::TPtr<nvZone> ZoneHeap::nvzone(RRegion::TPtr<void> ptr)
{
    return nvheap_->zone(ptr);
}

inline RRegion::TPtr<nvZone> ZoneHeap::nvzone(size_t zone_id)
{
    return nvheap_->zone(zone_id);
}

inline MemAttrib ZoneHeap::nvzone_memattrib(size_t zone_id)
{
    InterleaveGroup ig = nvheap_->zone(zone_id)->header.ig;
    return MemAttrib(ig);
}

template<typename T> 
Zone* ZoneHeap::acquire_zone(bool alloc_new_zone, size_t zone_id, size_t min_free_blocks, T enumerate_callback_functor)
{
    RRegion::TPtr<nvZone> nvzone = nvheap_->zone(zone_id);

    if (memattrib_ != nvzone_memattrib(zone_id)) {
        return NULL;
    }

    if (!alloc_new_zone) {
        // We are not looking for a new zone and another thread from the 
        // same process has acquired the lease.
        // Read-serialize as the thread might still be in the process of 
        // initializing the volatile zone descriptor
        if (nvzone->header.lease.lock_status() == generation_) { 
            rwlock_.lock_read();
            Zone* zone = nvzone->header.zone;
            assert(zone);
            rwlock_.unlock_read();
            if (min_free_blocks > 0 && !zone->has_free_space(min_free_blocks)) {
                return NULL;
            }
            return zone;
        }
    }
        
    if (nvzone->header.lease.lock_status() == Lease::kUnlocked) { 
        rwlock_.lock_write();
        if (!alloc_new_zone) {
            // check whether some other thread has won the race and already initialized zone descriptor
            if (nvzone->header.lease.lock_status() == generation_) { 
                Zone* zone = nvzone->header.zone;
                rwlock_.unlock_write();
                if (min_free_blocks > 0 && !zone->has_free_space(min_free_blocks)) {
                    return NULL;
                }
                return zone;
            }
        }
        // if we won the race with other threads then initialize zone descriptor
        if (nvzone->header.lease.lock_status() == Lease::kUnlocked) { 
            if (nvzone->header.lease.try_lock(generation_) == 0) {
                Zone* zone = new Zone(nvheap_, nvzone);
                zone->init();
                if (min_free_blocks == 0 || zone->has_free_space(min_free_blocks)) {
                    LOG(info) << "Acquired zone: " << zone_id; 
                    zone->enumerate(enumerate_callback_functor);
                    zones_.insert(zone);
                    rwlock_.unlock_write();
                    return zone;
                } else {
                    release_zone(zone);
                    delete zone;
                } 
            }
        }
        rwlock_.unlock_write();
    }
    return NULL;
}


template<typename T> 
Zone* ZoneHeap::acquire_new_zone(size_t min_free_blocks, T enumerate_callback_functor)
{
    Zone* zone;
    size_t nzones = nvheap_->nzones();

    for (size_t i=0; i<nzones; i++) {
        if ((zone = acquire_zone(true, i, min_free_blocks, enumerate_callback_functor))) {
            return zone;
        }
    }
    return NULL;
}


template<typename T> 
Zone* ZoneHeap::acquire_new_zone(T enumerate_callback_functor)
{
    Zone* zone;
    size_t nzones = nvheap_->nzones();

    for (size_t i=0; i<nzones; i++) {
        if ((zone = acquire_zone(true, i, 1, enumerate_callback_functor))) {
            return zone;
        }
    }
    return NULL;
}


} // namespace alps

#endif // _ALPS_GLOBALHEAP_ZONEHEAP_HH_
