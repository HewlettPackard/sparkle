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

#include "globalheap/slab_heap.hh"

#include "alps/pegasus/relocatable_region.hh"

#include "globalheap/extent.hh"
#include "globalheap/extentheap.hh"
#include "globalheap/layout.hh"
#include "globalheap/nvslab.hh"
#include "globalheap/slab.hh"
#include "globalheap/zone.hh"

namespace alps {

RRegion::TPtr<void> SlabHeap::alloc_block(Slab* slab)
{
    RRegion::TPtr<void> ptr;

    bool empty = slab->empty();
    int old_fullness = slab->fullness();
    ptr = slab->alloc_block();
    if (ptr != null_ptr) {
        int new_fullness = slab->fullness();
        int szclass = slab->sizeclass();
        LOG(info) << "Allocate block: " << "new_fullness: " << new_fullness << " old_fullness: " << old_fullness;
        if (empty || (new_fullness != old_fullness)) {
            move_slab(slab, szclass, new_fullness);
        }
    }
    return ptr;
}

void SlabHeap::free_block(Slab* slab, RRegion::TPtr<void> ptr)
{
    int old_fullness = slab->fullness();
    slab->free_block(ptr);
    if (slab->empty()) {
        LOG(info) << "Free block: " << "recycle now empty slab";
        slab->remove();
        insert_slab_to_empty(slab);
    } else {
        int new_fullness = slab->fullness();
        int szclass = slab->sizeclass();
        LOG(info) << "Free block: " << "new_fullness: " << new_fullness << " old_fullness: " << old_fullness;
        move_slab(slab, szclass, new_fullness);
    }

}

Slab* SlabHeap::find_slab(const int szclass)
{
    Slab* slab = NULL;

    // Skip kSlabFullnessBins-1 slab list as it contains completely full slabs
    int fullness_hint = kSlabFullnessBins-2;

    // Find the most-full slab 
    for (int i=fullness_hint; i>=0; i--) {
        SlabList& sl = full_slabs_[szclass][i];
        if (sl.size()) {
            slab = sl.front();
            break;
        }
    }

    if (!slab) {
        slab = reuse_empty_slab(szclass);
    }
    return slab;
}

Slab* SlabHeap::insert_slab(RRegion::TPtr<nvSlab> nvslab)
{
    LOG(info) << "Insert slab: " << nvslab;

    Slab* slab = new Slab(nvslab);
    insert_slab(slab, nvslab->sizeclass());
    return slab;
}

void SlabHeap::insert_slab(Slab* slab, int szclass)
{
    slab->set_owner(this);

    int fullness = slab->fullness();

    if (slab->empty()) {
        insert_slab_to_empty(slab);
    } else {
        slab->insert(&full_slabs_[szclass][fullness]);
    }
}

void SlabHeap::remove_slab(Slab* slab)
{
    slab->set_owner(NULL);
    slab->remove();
}

void SlabHeap::move_slab(Slab* slab, int szclass, int to)
{
    slab->remove();
    slab->insert(&full_slabs_[szclass][to]);
}

void SlabHeap::insert_slab_to_empty(Slab* slab)
{
    slab->insert(&empty_slabs_);
}

Slab* SlabHeap::reuse_empty_slab(int szclass)
{
    Slab* slab = NULL;
    if (empty_slabs_.size()) {
        slab = empty_slabs_.front();
        int fullness = slab->fullness();
        move_slab(slab, szclass, fullness);
        if (slab->sizeclass() != szclass) {
            slab->init(szclass);
        }
    }
    return slab;
}

void SlabHeap::stream_to(std::ostream& os) const {
    for (int i=0; i<kSlabFullnessBins; i++) {
        std::cout << "[" << i << "] ";
        bool comma = false;
        for (int c=0; c<kSizeClasses; c++) {
            for (SlabList::const_iterator it = full_slabs_[c][i].begin();
                 it != full_slabs_[c][i].end();
                 it++) 
            {
                if (comma) { std::cout << ", "; }
                std::cout << **it;
                comma = true;
            }
        }
        std::cout << std::endl;
    }
    std::cout << "[R] ";
    bool comma = false;
    for (SlabList::const_iterator it = empty_slabs_.begin();
         it != empty_slabs_.end();
         it++) 
    {
        if (comma) { std::cout << ", "; }
        std::cout << **it;
        comma = true;
    }
    std::cout << std::endl;
}


void InsertSlabFunctor::operator()(Zone* zone, Extent* extent)
{
    RRegion::TPtr<nvExtentHeader> nvexheader = static_cast<RRegion::TPtr<nvExtentHeader>>(zone->nvzone()->block_header(extent->start()));
    RRegion::TPtr<nvSlab> nvslab = static_cast<RRegion::TPtr<nvSlab>>(zone->nvzone()->block(extent->start()));

    if (nvexheader->secondary_type == nvExtentHeader::kExtentTypeSlab) {
        slabheap_->insert_slab(nvslab);
    }
}



} // namespace alps
