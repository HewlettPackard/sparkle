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

#include "globalheap/freespacemap.hh"

#include <stddef.h>
#include <stdint.h>

#include "alps/common/error_code.hh"

#include "common/debug.hh"
#include "globalheap/helper.hh"
#include "globalheap/zone_heap.hh"

namespace alps {

ErrorStack FreeSpaceMap::init()
{
    Extent extent;
    bool   extent_is_free;
    size_t nblocks = nvzone_->nblocks();

    for (size_t next_start = 0;
         find_extent(nvzone_, next_start, nblocks, &extent, &extent_is_free) < nblocks; ) 
    {
        next_start = extent.start() + extent.len();
        if (extent_is_free) {
            insert(extent);
        }
    }
    return kRetOk;
}

bool FreeSpaceMap::exists_extent(size_t size_nblocks)
{
    Extent ex;
    return find_ge(size_nblocks, &ex) == 0 ? true : false;
}

int FreeSpaceMap::alloc_extent(size_t size_nblocks, Extent* ex)
{
    if (remove_ge(size_nblocks, ex) == 0) {
        // if returned size is larger than requested size then return the
        // rest of the space back to the extendmap
        if (ex->len() > size_nblocks) {
            insert(Extent(ex->start() + size_nblocks, ex->len() - size_nblocks));
        }
        *ex = Extent(ex->start(), size_nblocks);
        return 0;   
    }
    return -1;
}


void FreeSpaceMap::free_extent(const Extent& ex)
{
    insert(ex);
}

} // namespace alps
