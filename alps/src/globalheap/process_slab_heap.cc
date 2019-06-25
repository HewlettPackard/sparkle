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

#include "globalheap/process_slab_heap.hh"

namespace alps {

Slab* ProcessSlabHeap::acquire_slab(int szclass)
{
    RRegion::TPtr<nvExtentHeader> nvexheader;
    RRegion::TPtr<void>           nvex;
    Slab*                         slab;
    int                           retries = 1;
    
    LOG(info) << "acquire_slab";

    lock();
    LOG(info) << "acquire_slab: locked";

    // If there is no available slab and extentheap has no space left then retry 
    // a few times to extend the extentheap's size and reuse partially-full slabs 
    // before force allocating a new slab
    for (int r=0; r <= retries; r++) {
        slab = find_slab(szclass);
        if (slab) {
            remove_slab(slab);
            LOG(info) << "acquire_slab: unlock slab: " << slab;
            unlock();
            return slab;
        }
        assert(extentheap_);
        if (extentheap_->malloc(slab_size, false, NullEnumerateFunctor(), &nvexheader, &nvex) == kErrorCodeOk) {
            RRegion::TPtr<nvSlab> nvslab = nvSlab::make(nvexheader, nvex, szclass);
            slab = new Slab(nvslab);
            LOG(info) << "acquire_slab: unlock 2";
            unlock();
            return slab;
        }
        extentheap_->more_space(1, InsertSlabFunctor(this));
    }
    // No partially full slabs; try to allocate a new slab
    ErrorCode rc = extentheap_->malloc(slab_size, true, InsertSlabFunctor(this), &nvexheader, &nvex); 
    if (rc != kErrorCodeOk) {
        slab = NULL;
    } else {
        RRegion::TPtr<nvSlab> nvslab = nvSlab::make(nvexheader, nvex, szclass);
        slab = new Slab(nvslab);
    }
    LOG(info) << "acquire_slab: unlock 3: slab=" << slab;
    unlock();
    return slab;
}

RRegion::TPtr<void> ProcessSlabHeap::malloc(size_t size)
{
    RRegion::TPtr<nvExtentHeader> nvexheader;
    RRegion::TPtr<void>           nvex;

    lock();
    if (extentheap_->malloc(size, true, InsertSlabFunctor(this), &nvexheader, &nvex) != kErrorCodeOk) {
        nvex = null_ptr;
    } 
    unlock();
    return nvex;
}

void ProcessSlabHeap::free(RRegion::TPtr<void> ptr)
{
    RRegion::TPtr<nvZone> nvzone = extentheap_->nvzone(ptr);
    size_t zone_id = nvZone::zone_id(extentheap_->nvheap(), nvzone);
    Zone* zone = extentheap_->acquire_zone(zone_id, InsertSlabFunctor(this));

    if (!zone) {
        LOG(warning) << "Attempted to free a block owned by another process";
        return;
    }

    // Check if this is a pointer to a large block: a large block is 
    // allocated directly as an extent and therefore it is aligned at 
    // at a multiple of block size
    RRegion::TPtr<nvBlock> nvzone_blocks = nvzone->block(0);
    if ((RRegion::TPtr<char>(ptr) - (RRegion::TPtr<char>(nvzone_blocks))) % BLOCK_SIZE == 0) {
        return extentheap_->free(zone, ptr);
    }

    // This is a pointer to a small block; locate the nvslab/slab 
    size_t block_id = (RRegion::TPtr<char>(ptr) - RRegion::TPtr<char>(nvzone_blocks)) / BLOCK_SIZE;
    RRegion::TPtr<nvBlock> nvblock = nvzone->block(block_id);
    RRegion::TPtr<nvSlab> nvslab = static_cast<RRegion::TPtr<nvSlab> >(nvblock);
    Slab* slab = nvslab->header.slab;

    // Expect this to finish after a few iterations as a slab that is 
    // being moved between two slab-heaps eventually ends up in a single
    // slabheap
    for (;;) {
        SlabHeap* owner = slab->owner();
        if (owner) {
            owner->lock();
            if (owner == slab->owner()) {
                owner->free_block(slab, ptr);
                owner->unlock();
                break;
            }
            owner->unlock();
        }
    }
}

} // namespace alps
