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

#include "globalheap/zone_heap.hh"

#include "alps/common/error_code.hh"
#include "alps/common/error_stack.hh"

#include "common/debug.hh"
#include "globalheap/zone.hh"

namespace alps {

ErrorStack ZoneHeap::init()
{
    return kRetOk;
}


ErrorStack ZoneHeap::teardown()
{
    for (ZoneSet::iterator it = zones_.begin();
         it != zones_.end();
         it++) 
    {
        Zone* zone = *it;
        release_zone(zone, false);
    }
    return kRetOk;
}

Zone* ZoneHeap::acquire_new_zone()
{
    return acquire_new_zone(NullEnumerateFunctor());
}


void ZoneHeap::release_zone(Zone* zone, bool remove)
{
    LOG(info) << "Release zone: " << zone->zone_id(); 

    zone->nvzone()->header.lease.unlock();
    if (remove) {
        zones_.erase(zone);
    }
}

} // namespace alps
