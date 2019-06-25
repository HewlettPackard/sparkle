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

#ifndef _ALPS_GLOBALHEAP_LAYOUT_HH_
#define _ALPS_GLOBALHEAP_LAYOUT_HH_

#include <stdint.h>

#include "alps/common/assorted_func.hh"
#include "alps/pegasus/relocatable_region.hh"

#include "common/debug.hh"
#include "globalheap/lease.hh"
#include "globalheap/persist.hh"

namespace alps {

#define BLOCK_LOG2SIZE 18                         /* 256 kilobytes */
#define BLOCK_SIZE ((size_t) 1 << BLOCK_LOG2SIZE)  


struct nvBlock {
    uint8_t data[BLOCK_SIZE];
};

struct nvBlockHeader {
    enum {
        kBlockTypeFree = 0,
        kBlockTypeAlloc = 1,
        kBlockTypeExtentFirst = 2, // first block of an extent
        kBlockTypeExtentRun = 3,   // run block of an extent 
    };

    static RRegion::TPtr<nvBlockHeader> make(RRegion::TPtr<nvBlockHeader> header, uint16_t primary_type)
    {
        header->primary_type = kBlockTypeFree;
        header->secondary_type = 0;
        persist((void*)&header, sizeof(nvBlockHeader));
        return header;
    }

    bool is_free() {
        return (primary_type == kBlockTypeFree);
    }

    uint8_t  primary_type;
    uint8_t  secondary_type;
    uint16_t flags;
    uint32_t size;
};

struct nvExtentHeader: public nvBlockHeader {
    enum {
        kExtentTypeNone = 0,
        kExtentTypeSlab = 1
    };

    void mark_alloc(uint32_t nblocks)
    {
        size = nblocks;
        nvBlockHeader* this_bh = reinterpret_cast<nvBlockHeader*>(this);
        for (uint32_t i=1; i<nblocks; i++) {
            nvBlockHeader* bh = this_bh+i;
            bh->primary_type = nvBlockHeader::kBlockTypeExtentRun;
            persist((void*) &bh->primary_type, sizeof(bh->primary_type));
        }

        // Linearization point (with respect to failures)
        // 
        // Persisting the primary_type of the first block is a single atomic 
        // step that identifies the block group as an extent run.
        this_bh->primary_type = nvBlockHeader::kBlockTypeExtentFirst;
        persist((void*) &this_bh->primary_type, sizeof(this_bh->primary_type));
    }

    void mark_free()
    {
        uint32_t nblocks = size;
        nvBlockHeader* this_bh = reinterpret_cast<nvBlockHeader*>(this);
        for (uint32_t i=0; i<nblocks; i++) {
            nvBlockHeader* bh = this_bh+i;
            bh->primary_type = nvBlockHeader::kBlockTypeFree;
        }
    }
};

// forward declaration
class Zone;
struct nvHeap;

struct nvZoneHeader {
    // first cacheline
    uint32_t     magic;
    uint32_t     metazone_log2size;
    uint64_t     zone_size;
    uint64_t     blocksize;
    uint64_t     blocks_per_zone;
    Zone*        zone; // pointer to the zone's volatile descriptor for quick lookup
    uint8_t      ig; // interleave group
    uint8_t      reserved[23];
    // second cacheline
    struct Lease lease; // must be cache aligned
};

static_assert(sizeof(nvZoneHeader) == 128, "nvZoneHeader must be multiple of cache-line size");

struct nvZone {
    static RRegion::TPtr<nvZone> make(RRegion::TPtr<nvZone> nvzone, size_t zone_size, size_t metazone_log2size)
    {
        // Calculate number of blocks that can fit in the zone and set 
        // the block_header and block pointers accordingly.
        // The first block must be aligned at cache-line multiple so that it
        // doesn't share a cacheline with the last block-header. 
        size_t effective_zone_size = zone_size - sizeof(nvZone);
        size_t max_nblocks = effective_zone_size / (sizeof(nvBlockHeader) + BLOCK_SIZE); 
        // Adjust (reduce) number of blocks to accomodate extra space needed
        // for alignment.
        size_t block_headers_aligned_total_size = round_up(max_nblocks * sizeof(nvBlockHeader), kCacheLineSize);
        size_t blocks_total_size = effective_zone_size - block_headers_aligned_total_size;
        size_t _nblocks = blocks_total_size / BLOCK_SIZE;

        assert((sizeof(nvZone) + (sizeof(nvBlockHeader) + BLOCK_SIZE) * _nblocks <= zone_size)); 

        // Set and persist header fields
        nvzone->header.metazone_log2size = uint32_t(metazone_log2size);
        nvzone->header.zone_size = zone_size;
        nvzone->header.blocksize = BLOCK_SIZE;
        nvzone->header.blocks_per_zone = _nblocks;
        Lease::make(&nvzone->header.lease);
        persist((void*)&nvzone->header, sizeof(nvzone->header));

        nvzone->block_headers = static_cast<RRegion::TPtr<nvBlockHeader>>((nvBlockHeader*) &nvzone->payload[0]);
        nvzone->blocks = static_cast<RRegion::TPtr<nvBlock>>((nvBlock*)&nvzone->payload[block_headers_aligned_total_size]);
        persist((void*)&nvzone->block_headers, sizeof(nvzone->block_headers));
        persist((void*)&nvzone->blocks, sizeof(nvzone->blocks));

        // Format block headers
        RRegion::TPtr<nvBlockHeader> tblhdr = static_cast<RRegion::TPtr<nvBlockHeader>>(nvzone->block_headers);
        for (size_t i=0; i<nvzone->header.blocks_per_zone; i++) {
            nvBlockHeader::make(tblhdr+i, nvBlockHeader::kBlockTypeFree);
        } 
        return nvzone;
    }

    static size_t zone_id(RRegion::TPtr<nvHeap> nvheap, RRegion::TPtr<nvZone> nvzone)
    {
        // operands are of different types so we cast them to a common type 
        // and do the pointer arithmetic ourselves 
        return (RRegion::TPtr<char>(nvzone) - RRegion::TPtr<char>(nvheap)) >> nvzone->header.metazone_log2size;
    }

    RRegion::TPtr<nvBlock> block(size_t idx)
    {
        // some ugly casting to get the char pointer
        RRegion::TPtr<nvBlock> _tptr = (RRegion::TPtr<nvBlock>) blocks;
        RRegion::TPtr<char> tptr = (RRegion::TPtr<char>) _tptr;
        size_t offset = idx << BLOCK_LOG2SIZE;
        return static_cast<RRegion::TPtr<struct nvBlock>>(tptr + offset);
    }

    RRegion::TPtr<nvBlockHeader> block_header(int idx)
    {
        return block_headers+idx;
    }

    static size_t block_size() 
    {
        return BLOCK_SIZE;
    }

    size_t nblocks()
    {
        return header.blocks_per_zone;
    }

    // first cacheline (header takes two cachelines)
    struct nvZoneHeader          header; 
    // third cacheline
    RRegion::PPtr<nvBlockHeader> block_headers;
    RRegion::PPtr<nvBlock>       blocks;
    char                         reserved[56];
    // fourth cacheline
    char                         payload[0];
};


struct nvHeapHeader {
    static RRegion::TPtr<nvHeapHeader> make(RRegion::TPtr<nvHeapHeader> header, size_t heap_size, size_t metazone_log2size)
    {
        LeaseSuperblock::make(&header->lease_superblock);
        header->heap_size = heap_size;
        header->metazone_log2size = metazone_log2size;
        persist((void*) &header, sizeof(nvHeapHeader));
        return header;
    }

    // first cacheline
    uint64_t            heap_size;
    uint64_t            metazone_log2size;
    uint8_t             reserved0[48];
    // second cacheline
    LeaseSuperblock     lease_superblock; // must be cache aligned
    // third cacheline
    RRegion::PPtr<void> root;
    uint64_t            checksum;
    uint8_t             reserved1[880];
};

static_assert(sizeof(nvHeapHeader) == 1024, "nvHeapHeader does not have expected size");

struct nvMetaZone {
    static RRegion::TPtr<nvMetaZone> make(RRegion::TPtr<nvMetaZone> nvmetazone, size_t heap_size, size_t metazone_log2size)
    {
        size_t metazone_size = 1LLU << metazone_log2size;
        size_t zone_size = metazone_size - sizeof(nvMetaZone); 

        nvHeapHeader::make(&nvmetazone->heapheader, heap_size, metazone_log2size);
        nvZone::make(nvmetazone->zone(), zone_size, metazone_log2size); 

        return nvmetazone;
    }

    RRegion::TPtr<struct nvZone> zone()
    {
        return static_cast<RRegion::TPtr<struct nvZone>>((nvZone*)&payload[0]);
    }

    struct nvHeapHeader          heapheader;
    char                         payload[0];
};


/**
 * A heap comprises multiple metazones, and a metazone encapsulates a 
 * heap header and a zone. We choose this layout over the alternative 
 * layout of a single heap header followed by multiple zones because:
 *  1. This results in a symmetric layout which in turn results in 
 *     simpler code. Each zone has the same size and each zone is aligned 
 *     at book size boundaries. The alternative approach would require 
 *     the first zone following the heap header to be smaller than the 
 *     other zones to ensure it stays within the book alignment boundary.
 *  2. It replicates the heap header per each zone so it offers resilience
 *     to failures.
 */
struct nvHeap {
    static RRegion::TPtr<nvHeap> make(RRegion::TPtr<nvHeap> nvheap, size_t heap_size, size_t metazone_size)
    {
        LOG(info) << "Make heap layout: " << nvheap << " size: " << heap_size << " metazone_size: " << metazone_size;

        if (!is_power_of_two(metazone_size)) {
            LOG(error) << "Metazone size must be power of two.";
            return null_ptr;
        }
        if (heap_size % metazone_size) {
            LOG(error) << "Heap size must be multiple of metazone size.";
            return null_ptr;
        }

        size_t metazone_log2size = log2(metazone_size);
        size_t nmetazones = heap_size / metazone_size;
        for (size_t zid=0; zid<nmetazones; zid++) {
            RRegion::TPtr<struct nvMetaZone> mz = metazone(nvheap, zid, metazone_log2size);
            nvMetaZone::make(mz, heap_size, metazone_log2size);
        }
        return nvheap;    
    }

    static RRegion::TPtr<struct nvMetaZone> metazone0(RRegion::TPtr<nvHeap> nvheap)
    {
        return static_cast<RRegion::TPtr<nvMetaZone>>(nvheap);
    }

    static RRegion::TPtr<struct nvMetaZone> metazone(RRegion::TPtr<nvHeap> nvheap, size_t zone_id, size_t metazone_log2size)
    {
        size_t offset = zone_id << metazone_log2size;
        RRegion::TPtr<char> tptr = static_cast<RRegion::TPtr<char>>(nvheap);
        return static_cast<RRegion::TPtr<struct nvMetaZone>>(tptr + offset);
    }

    static RRegion::TPtr<struct nvMetaZone> metazone(RRegion::TPtr<nvHeap> nvheap, size_t zone_id)
    {
        RRegion::TPtr<struct nvMetaZone> mz0 = metazone0(nvheap);
        return metazone(nvheap, zone_id, mz0->heapheader.metazone_log2size); 
    }

    static RRegion::TPtr<nvZone> zone(RRegion::TPtr<nvHeap> nvheap, size_t zone_id)
    {
        return metazone(nvheap, zone_id)->zone();
    }

    RRegion::TPtr<struct nvMetaZone> metazone(size_t zone_id)
    {
        RRegion::TPtr<nvHeap> nvheap = this;
        RRegion::TPtr<struct nvMetaZone> mz0 = metazone0(nvheap);
        return metazone(nvheap, zone_id, mz0->heapheader.metazone_log2size); 
    }

    RRegion::TPtr<nvZone> zone(size_t zone_id)
    {
        return metazone(zone_id)->zone();
    }

    RRegion::TPtr<nvZone> zone(RRegion::TPtr<void> ptr)
    {
        size_t zone_id = ((char*) ptr.get() - (char*) this) / metazone_size();
        return zone(zone_id);
    }

    RRegion::TPtr<nvHeapHeader> header()
    {
        return &metazone(0)->heapheader;
    }

    RRegion::TPtr<void> root() 
    {
        return (RRegion::TPtr<void>) metazone(0)->heapheader.root;
    }

    void set_root(RRegion::TPtr<void> root) 
    {
        metazone(0)->heapheader.root = root;
    }

    size_t size() {
        return metazone(0)->heapheader.heap_size;
    }

    size_t metazone_size() {
        return 1LLU << metazone(0)->heapheader.metazone_log2size;
    }

    size_t metazone_log2size() {
        return metazone(0)->heapheader.metazone_log2size;
    }

    size_t nzones() {
        return size() / metazone_size();
    }
    char                payload[0];
};


} // namespace alps

#endif // _ALPS_GLOBALHEAP_LAYOUT_HH_
