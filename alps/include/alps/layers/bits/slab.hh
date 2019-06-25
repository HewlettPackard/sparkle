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

#ifndef _ALPS_LAYERS_BITS_SLAB_H_
#define _ALPS_LAYERS_BITS_SLAB_H_

#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <list>
#include <iostream>

#include "alps/common/debug.hh"
#include "alps/layers/bits/bitmap.hh"

#include "size_class.hh"


namespace alps {

/**
 * @brief Variable-size slab header
 */
template<typename Context, template<typename> class TPtr>
struct nvSlabHeader {
    // When adding a member field, ensure method size_of() includes that field too
    uint32_t header_size;
    uint16_t sizeclass;
    uint32_t nblocks;
    void* slab; // pointer to the slab's volatile descriptor for quick lookup
    nvBitMap<Context> block_map; // variable size structure

    // total size of the fixed part of the header
    static size_t size_of() {
        return sizeof(header_size) + sizeof(sizeclass) + sizeof(nblocks) + sizeof(void*);
    }

    static TPtr<nvSlabHeader> make(Context& ctx, TPtr<nvSlabHeader> header, size_t slab_size, int size_class)
    {
        size_t block_size = size_from_class(size_class);
        size_t nblocks = max_nblocks(slab_size, block_size);
        size_t header_sz = size_of() + nvBitMap<Context>::size_of(nblocks);
        header->sizeclass = size_class;
        header->header_size = align<size_t, kCacheLineSize>(header_sz);
        // Adjust (reduce) number of blocks to accomodate extra space needed 
        // for roundup
        header->nblocks = (slab_size - header->header_size) / block_size;
        nvBitMap<Context>::make(ctx, nblocks, &header->block_map);
        //persist((void*)&header, sizeof(nvSlabHeader));
        //persist((void*)&header->block_map, nvBitMap::size_of(nblocks));
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
        size_t nblocks_per_bitmap_byte = nvBitMap<Context>::kEntrySize;
        return (slab_size - size_of() - 1) * nblocks_per_bitmap_byte / (1 + nblocks_per_bitmap_byte * block_size); 
    }
};

// Slab
// A slab comprises a header followed by a number of blocks. 
template<typename Context, template<typename> class TPtr>
struct nvSlab
{
    nvSlabHeader<Context, TPtr> header;    // Variable-size header

    static TPtr<nvSlab> make(Context& ctx, TPtr<void> region, size_t slab_size, int size_class)
    {
        TPtr<nvSlab> nvslab = region;
        nvSlabHeader<Context, TPtr>::make(ctx, &nvslab->header, slab_size, size_class);
        assert(nvslab->block_offset(nvslab->nblocks()) <= slab_size);
        return nvslab;
    }

    size_t nblocks() { return header.nblocks; }
    size_t sizeclass() { return header.sizeclass; }
    size_t block_size() { return size_from_class(header.sizeclass); }
    
    size_t block_offset(int id) 
    {
        size_t offset_0 = header.size();
        return offset_0 + id * block_size();
    }

    TPtr<void> block(size_t block_id)
    {
        TPtr<char> base = static_cast<TPtr<char>>((char*)&header);   
        TPtr<char> block_ptr = base + block_offset(block_id);
        return TPtr<void>(block_ptr);
    }

    size_t block_id(TPtr<void> ptr)
    {
        TPtr<void> block0 = block(0);
        return (TPtr<char>((char*)ptr.get()) - TPtr<char>((char*)block0.get())) / block_size();
    }

    bool is_free(Context& ctx, size_t block_idx) 
    {
        assert(block_idx < nblocks());
        return !header.block_map.is_set(ctx, block_idx);
    }

    void set_alloc(Context& ctx, size_t block_idx)
    {
        header.block_map.set(ctx, block_idx);
    }

    void set_free(Context& ctx, size_t block_idx)
    {
        header.block_map.clear(ctx, block_idx);
    }

    void set_slab(void* slab)
    {
        header.slab = slab;
    }

    void* slab()
    {
        return header.slab;
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



const int kSlabFullnessBins = 3;

/**
 * @brief Per-process private volatile Slab descriptor wrapping underlying 
 * shared non-volatile Slab
 *
 * @details
 * Slabs are owned and managed by a SlabHeap and any call for allocating/freeing 
 * blocks in a slab must be done through the SlabHeap that owns the slab.
 *
 */
template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
class Slab
{
public:
    typedef std::list<Slab*> SlabList;

public:
    static Slab* make(Context& ctx, TPtr<void> region, size_t slab_size, size_t size_class)
    {
        TPtr<nvSlab<Context, TPtr>> nvslab = nvSlab<Context, TPtr>::make(ctx, region, slab_size, size_class);
        Slab* slab = new Slab(nvslab);
        slab->init(ctx);
        nvslab->set_slab(slab);
        return slab;
    }

    static Slab* load(Context& ctx, TPtr<void> region)
    {
        TPtr<nvSlab<Context,TPtr>> nvslab = region;
        Slab* slab = new Slab(nvslab);
        slab->init(ctx);
        nvslab->set_slab(slab);
        return slab;
    }

    static Slab* slab(TPtr<void> region)
    {
        TPtr<nvSlab<Context,TPtr>> nvslab = region;
        return reinterpret_cast<Slab*>(nvslab->slab());
    }

    Slab(TPtr<nvSlab<Context,TPtr>> nvslab)
        : nvslab_(nvslab),
          slab_list_(NULL)
    { }

    void init(Context& ctx)
    {   
        if (block_size()) {
            free_list_.clear();
            for (size_t i=0; i<nblocks(); i++) {
                if (nvslab_->is_free(ctx, i)) {
                    free_list_.push_back(i);
                }
            }
        }
    }

    void reset(Context& ctx, size_t slab_size, int szclass)
    {   
        nvSlab<Context,TPtr>::make(ctx, nvslab_, slab_size, szclass);
        init(ctx);
    }

    int sizeclass() const 
    {
        return nvslab_->sizeclass();
    }

    size_t block_size() const
    {
        return nvslab_->block_size();
    }

    size_t block_offset(int index)
    {
        return nvslab_->block_offset(index);
    }

    bool full() 
    {
        return (nblocks_free() == 0);
    }

    bool empty()
    {
        return nblocks_free() == nblocks();
    }

    size_t nblocks() const
    {
        return nvslab_->nblocks();
    }

    size_t nblocks_free() const
    {
        return free_list_.size();
    }

    void set_owner(void* owner)
    {
        owner_.store(owner, std::memory_order_seq_cst);
    }

    void* owner() 
    {
        return owner_.load(std::memory_order_seq_cst);
    }
 
    void insert(SlabList* slab_list)
    {
        assert(slab_list_ == NULL);
        slab_list->push_front(this);
        slab_list_it_ = slab_list->begin();
        slab_list_ = slab_list;
    }

    void remove()
    {
        assert(slab_list_ != NULL);
        slab_list_->erase(slab_list_it_);
        slab_list_ = NULL;
    }

    int fullness()
    {
        return ((kSlabFullnessBins - 1) * (nblocks() - nblocks_free())) / nblocks();
    }

    TPtr<void> alloc_block(Context& ctx)
    {
        TPtr<void> ptr;

        if (!free_list_.empty()) {
            int bid = free_list_.front();
            free_list_.pop_front();
            nvslab_->set_alloc(ctx, bid);
            ptr = nvslab_->block(bid);
            LOG(info) << "Allocate block: " << "nvslab: " << nvslab_ << " block: " << bid;
        } else {
            LOG(info) << "Allocate block: FAILED: no free space";
            ptr = 0;
        }
        return ptr;
    }

    void free_block(Context& ctx, TPtr<void> ptr)
    {
        size_t bid = nvslab_->block_id(ptr);

        LOG(info) << "Free block: " << "nvslab: " << nvslab_ << " block: " << bid;
        if (ctx.do_v) {
            free_list_.push_front(bid);
        }
        if (ctx.do_nv) {
            assert(nvslab_->is_free(ctx, bid) == false);
            nvslab_->set_free(ctx, bid);
        }
    }

    void stream_to(std::ostream& os) const 
    {
        os << "(" << block_size() << ", " << nblocks() << ", " << nblocks_free() << ")";
    }

    friend std::ostream& operator<<(std::ostream& os, const Slab& slab)
    {
        slab.stream_to(os);
        return os;
    }


    std::atomic<void*>           owner_;
    std::list<size_t>            free_list_;
    TPtr<nvSlab<Context, TPtr>>  nvslab_;
    SlabList*                    slab_list_; // list this slab belongs to
    typename SlabList::iterator  slab_list_it_; // position in the slab list
};

} // namespace alps

#endif // _ALPS_LAYERS_BITS_SLAB_H_
