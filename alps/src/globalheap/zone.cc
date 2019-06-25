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

#include "globalheap/zone.hh"

#include <stddef.h>
#include <stdint.h>

#include "alps/common/error_code.hh"

#include "common/debug.hh"
#include "globalheap/helper.hh"

namespace alps {

// TODO: allocating extent should not mark every block comprising the extent but instead the 
//       extent header should write down the extentsize. The find_extent function should be
//       modified accordingly to skip blocks that are part of an allocated extent 


bool Zone::has_free_space(size_t size_nblocks)
{
    return fsmap_.exists_extent(size_nblocks);
}

ErrorCode Zone::alloc_extent(size_t size_nblocks, RRegion::TPtr<nvExtentHeader>* nvexheader, RRegion::TPtr<void>* nvex)
{
    Extent ex;
    if (fsmap_.alloc_extent(size_nblocks, &ex) == 0) {
        *nvexheader = static_cast<RRegion::TPtr<nvExtentHeader>>(nvzone_->block_header(ex.start()));
        (*nvexheader)->mark_alloc(ex.len());
        RRegion::TPtr<nvBlock> nvblock = nvzone_->block(ex.start());
        //*nvex = RRegion::TPtr<void>(nvblock.region(), nvblock.offset());
        *nvex = RRegion::TPtr<void>(nvblock);
        LOG(info) << "Allocated zone_id: " << zone_id() << " extent: " << ex << " " << (*nvex).get();
        return kErrorCodeOk;
    }
    return kErrorCodeOutofmemory;
}

void Zone::free_extent(RRegion::TPtr<void> ptr)
{
    RRegion::TPtr<nvBlock> nvblock_base = nvzone_->block(0);
    uintptr_t offset = ptr.offset() - nvblock_base.offset();
    size_t idx = offset >> BLOCK_LOG2SIZE;
    RRegion::TPtr<nvExtentHeader> nvexheader = static_cast<RRegion::TPtr<nvExtentHeader>>(nvzone_->block_header(idx));
    fsmap_.free_extent(Extent(idx, nvexheader->size));
    nvexheader->mark_free();
}


} // namespace alps
