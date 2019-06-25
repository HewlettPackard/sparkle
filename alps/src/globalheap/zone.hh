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

#ifndef _ALPS_GLOBALHEAP_ZONE_HH_
#define _ALPS_GLOBALHEAP_ZONE_HH_

#include "alps/common/error_code.hh"
#include "alps/pegasus/relocatable_region.hh"

#include "common/debug.hh"
#include "globalheap/layout.hh"
#include "globalheap/freespacemap.hh"
#include "globalheap/helper.hh"

namespace alps {

typedef size_t ZoneId;

class Zone {
public:
    Zone(RRegion::TPtr<nvHeap> nvheap, RRegion::TPtr<nvZone> nvzone)
        : nvheap_(nvheap),
          nvzone_(nvzone),
          fsmap_(nvzone)
    { 
        nvzone_->header.zone = this;
        LOG(info) << "NVZONE: " << nvzone_.get();
        LOG(info) << "NVZONE: ID: " << zone_id();
    }

    ErrorStack init() {
        return fsmap_.init();
    }

    bool has_free_space(size_t size_nblocks);
    ErrorCode alloc_extent(size_t size_nblocks, RRegion::TPtr<nvExtentHeader>* nvexheader, RRegion::TPtr<void>* nvex);
    void free_extent(RRegion::TPtr<void> ptr);

    ZoneId zone_id();
    RRegion::TPtr<nvZone> nvzone() { return nvzone_; }
    FreeSpaceMap* fsmap() { return &fsmap_; }
    template<typename T> void enumerate(T callback);

private:
    RRegion::TPtr<nvHeap> nvheap_;
    RRegion::TPtr<nvZone> nvzone_;
    FreeSpaceMap          fsmap_;        
};

inline ZoneId Zone::zone_id()
{
    return nvZone::zone_id(nvheap_, nvzone_);
}


template<typename T>
void Zone::enumerate(T callback_functor)
{
    LOG(info) << "Enumerate zone: " << zone_id();

    size_t nblocks = nvzone_->nblocks();
    Extent extent;
    bool extent_is_free;
    for (size_t next_start = 0;
         find_extent(nvzone_, next_start, nblocks, &extent, &extent_is_free) < nblocks; ) 
    {
        next_start = extent.start() + extent.len();
        if (!extent_is_free) {
            callback_functor(this, &extent);
        }
    }
}


class NullEnumerateFunctor {
public:
    void operator()(Zone* zone, Extent* extent)
    {
        // do nothing
    }
};



} // namespace alps


#endif // _ALPS_GLOBALHEAP_ZONE_HH_
