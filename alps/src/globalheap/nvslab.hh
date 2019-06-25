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

#ifndef _ALPS_GLOBALHEAP_NVSLAB_HH_
#define _ALPS_GLOBALHEAP_NVSLAB_HH_

#include "alps/common/assorted_func.hh"
#include "alps/pegasus/relocatable_region.hh"

#include "globalheap/bitmap.hh"
#include "globalheap/layout.hh"
#include "globalheap/size_class.hh"

namespace alps {

// forward declarations
class Slab;

static size_t slab_size = 256*1024LLU;

/**
 * @brief Variable-size slab header
 */
struct nvSlabHeader {
    // When adding a member field, ensure method size_of() includes that field too
    uint32_t header_size;
    uint16_t sizeclass;
    uint32_t nblocks;
    Slab*    slab; // pointer to the slab's volatile descriptor for quick lookup
    BitMap   block_map; // variable size structure

    // total size of the fixed part of the header
    static size_t size_of() {
        return sizeof(header_size) + sizeof(sizeclass) + sizeof(nblocks) + sizeof(Slab*);
    }

    static RRegion::TPtr<nvSlabHeader> make(RRegion::TPtr<nvSlabHeader> header, size_t slab_size, int sizeclass)
    {
        size_t block_size = size_from_class(sizeclass);
        size_t nblocks = max_nblocks(slab_size, block_size);
        size_t header_sz = size_of() + BitMap::size_of(nblocks);
        header->sizeclass = sizeclass;
        header->header_size = align<size_t, kCacheLineSize>(header_sz);
        // Adjust (reduce) number of blocks to accomodate extra space needed 
        // for roundup
        header->nblocks = (slab_size - header->header_size) / block_size;
        BitMap::make(nblocks, &header->block_map);
        persist((void*)&header, sizeof(nvSlabHeader));
        persist((void*)&header->block_map, BitMap::size_of(nblocks));
        return header;
    }

    size_t size() {
        return header_size;
    }

    /**
     * @brief Return maximum number of blocks for a given slab size and block size
     *
     * @details
     * Return max N so that:
     *   sizeof(header_size) + sizeof(sizeclass) + sizeof(nblocks) + ceil(N/nblocks_per_bitmap_byte) + N*block_size < slab_size; 
     *   sizeof(header_size) + sizeof(sizeclass) + sizeof(nblocks) + (N/nblocks_per_bitmap_byte + 1) + N*block_size < slab_size; 
     */
    static size_t max_nblocks(size_t slab_size, size_t block_size) 
    {
        size_t nblocks_per_bitmap_byte = BitMap::kEntrySize;
        return (slab_size - size_of() - 1) * nblocks_per_bitmap_byte / (1 + nblocks_per_bitmap_byte * block_size); 
    }
};

// Slab
// A slab comprises a header followed by a number of blocks. 
struct nvSlab
{
    nvSlabHeader header;    // Variable-size header

    static RRegion::TPtr<nvSlab> make(RRegion::TPtr<nvSlab> nvslab, int sizeclass)
    {
        nvSlabHeader::make(&nvslab->header, slab_size, sizeclass);
        assert(nvslab->block_offset(nvslab->nblocks()) <= slab_size);
        return nvslab;
    }

    static RRegion::TPtr<nvSlab> make(RRegion::TPtr<nvExtentHeader> nvexheader, RRegion::TPtr<nvSlab> nvslab, int sizeclass)
    {
        RRegion::TPtr<nvSlab> _nvslab = make(nvslab, sizeclass);

        // Linearization point (with respect to failures)
        // 
        // Persisting the type of the extent as a slab after we create the extent
        // is a single atomic step. If we fail turning the extent into a slab, the
        // heap is still consistent
        nvexheader->secondary_type = nvExtentHeader::kExtentTypeSlab;
        persist((void*)&nvexheader->secondary_type, sizeof(nvexheader->secondary_type));

        return _nvslab;
    }

    size_t nblocks() { return header.nblocks; }
    size_t sizeclass() { return header.sizeclass; }
    size_t block_size() { return size_from_class(header.sizeclass); }
    
    size_t block_offset(int id) 
    {
        size_t offset_0 = header.size();
        return offset_0 + id * block_size();
    }

    RRegion::TPtr<void> block(size_t block_id)
    {
        RRegion::TPtr<char> base = static_cast<RRegion::TPtr<char>>((char*)&header);   
        RRegion::TPtr<char> block_ptr = base + block_offset(block_id);
        return RRegion::TPtr<void>(block_ptr);
    }

    size_t block_id(RRegion::TPtr<void> ptr)
    {
        RRegion::TPtr<char> block0 = block(0);
        return (RRegion::TPtr<char>(ptr) - block0) / block_size();
    }

    bool is_free(size_t block_idx) 
    {
        assert(block_idx < nblocks());
        return !header.block_map.is_set(block_idx);
    }

    void set_alloc(size_t block_idx)
    {
        header.block_map.set(block_idx);
    }

    void set_free(size_t block_idx)
    {
        header.block_map.clear(block_idx);
    }

    void set_slab(Slab* slab)
    {
        header.slab = slab;
    }

    size_t nblocks_free() 
    {
        size_t cnt=0; 
        for (size_t i=0; i<nblocks(); i++) {
            cnt = is_free(i) ? cnt + 1: cnt;
        }
        return cnt;
    }
};


} // namespace alps 

#endif // _ALPS_GLOBALHEAP_NVSLAB_HH_
