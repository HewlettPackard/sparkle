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

#ifndef _ALPS_LAYERS_NVEXTENTHEAP_HH_
#define _ALPS_LAYERS_NVEXTENTHEAP_HH_

namespace alps {

#define kCacheLineSize 64

struct nvBlock {
    uint8_t data[1]; // block has at least one byte space
};


template<typename Context, template<typename> class TPtr>
struct nvExtentHeader {
public:
    enum {
        kBlockTypeFree = 0,
        kBlockTypeExtentFirst = 1, // first block of an extent
        kBlockTypeExtentRun = 2,   // run block of an extent 
    };

    static TPtr<nvExtentHeader> make(TPtr<nvExtentHeader> header, uint8_t type)
    {
        header->type_ = type;
        //persist((void*)&header, sizeof(nvExtentHeader));
        return header;
    }

    uint32_t size()
    {
        return size_;
    }

    bool is_free() {
        return (type_ == kBlockTypeFree);
    }

    void mark_alloc(uint32_t nblocks)
    {
        size_ = nblocks;
        nvExtentHeader* this_bh = reinterpret_cast<nvExtentHeader*>(this);
        for (uint32_t i=1; i<nblocks; i++) {
            nvExtentHeader* bh = this_bh + i;
            bh->type_ = nvExtentHeader::kBlockTypeExtentRun;
            //persist((void*) &bh->type_, sizeof(bh->type_));
        }

        // Linearization point (with respect to failures)
        // 
        // Persisting the primary_type of the first block is a single atomic 
        // step that identifies the block group as an extent run.
        this_bh->type_ = nvExtentHeader::kBlockTypeExtentFirst;
        //persist((void*) &this_bh->primary_type, sizeof(this_bh->primary_type));
    }

    void mark_free()
    {
        uint32_t nblocks = size_;
        nvExtentHeader* this_bh = reinterpret_cast<nvExtentHeader*>(this);
        for (uint32_t i=0; i<nblocks; i++) {
            nvExtentHeader* bh = this_bh + i;
            bh->type_ = nvExtentHeader::kBlockTypeFree;
        }
    }

//protected:
    union {
        struct {
            /** Block type */
            uint8_t  type_;

            /** Extent size in number of blocks */
            uint32_t size_;
        };
        uint8_t u8_[64];
    };
};

template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
class ExtentHeap;

struct nvExtentHeapHeader {
    union {
        struct {
            // first cacheline
            uint32_t  magic;
            uint64_t  region_size_;
            uint64_t  block_log2size_;
            uint64_t  nblocks;
            uint64_t  extent_headers_offset_; // extent headers offset relative to payload
            uint64_t  blocks_offset_; // blocks offset relative to payload
            void*     extentheap_; // pointer to the heap's volatile descriptor for quick lookup
        };
        uint8_t u8_[64];
    };
};

static_assert(sizeof(nvExtentHeapHeader) == 64, "nvExtentHeapHeader must be multiple of cache-line size");

template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
struct nvExtentHeap {
public:
    static TPtr<nvExtentHeap> make(TPtr<void> region, size_t region_size, size_t block_log2size)
    {
        // Calculate number of blocks that can fit in the zone and set 
        // the block_header and block pointers accordingly.
        // The first block must be aligned at cache-line multiple so that it
        // doesn't share a cacheline with the last block-header. 
        size_t block_size = 1LLU << block_log2size;
        size_t effective_region_size = region_size - sizeof(nvExtentHeap);
        size_t max_nblocks = effective_region_size / (sizeof(nvExtentHeader<Context, TPtr>) + block_size); 
        // Adjust (reduce) number of blocks to accomodate extra space needed
        // for alignment.
        size_t extent_headers_aligned_total_size = round_up(max_nblocks * sizeof(nvExtentHeader<Context, TPtr>), kCacheLineSize);
        size_t blocks_total_size = effective_region_size - extent_headers_aligned_total_size;
        size_t nblocks = blocks_total_size / block_size;

        assert((sizeof(nvExtentHeap<Context, TPtr,PPtr>) + (sizeof(nvExtentHeader<Context, TPtr>) + block_size) * nblocks <= region_size)); 

        // Set and persist header fields
        TPtr<nvExtentHeap<Context, TPtr, PPtr> > exheap = region;
        exheap->header_.region_size_ = region_size;
        exheap->header_.block_log2size_ = block_log2size;
        exheap->header_.nblocks = nblocks;
        //exheap->extent_headers = &exheap->payload[0];
        //exheap->blocks = static_cast<TPtr<nvBlock>>((nvBlock*)&exheap->payload[extent_headers_aligned_total_size]);
        exheap->header_.extent_headers_offset_ = 0;
        exheap->header_.blocks_offset_ = extent_headers_aligned_total_size;
        //persist((void*)&exheap->header, sizeof(exheap->header));

        // Format block headers
        for (size_t i=0; i<exheap->header_.nblocks; i++) {
            nvExtentHeader<Context, TPtr>::make(exheap->extent_header(i), nvExtentHeader<Context, TPtr>::kBlockTypeFree);
        } 
        return exheap;
    }

    static TPtr<nvExtentHeap> load(TPtr<void> region)
    {
        return reinterpret_cast<nvExtentHeap<Context, TPtr, PPtr>*>(region.get());
    }

    TPtr<nvExtentHeader<Context, TPtr> > extent_header(int idx)
    {
        return &payload_[header_.extent_headers_offset_ + idx*sizeof(nvExtentHeader<Context, TPtr>)];
    }
 
    TPtr<nvBlock> block(int idx) 
    {
        size_t block_size = 1LLU << header_.block_log2size_;
        return &payload_[header_.blocks_offset_ + idx * block_size];
    }

    bool is_free(const ExtentInterval& interval)
    {
        TPtr<nvExtentHeader<Context, TPtr>> exhdr  = extent_header(interval.start());
        return exhdr->is_free();
    }

    ExtentHeap<Context, TPtr, PPtr>* extentheap() 
    {
        return reinterpret_cast<ExtentHeap<Context, TPtr, PPtr>*>(header_.extentheap_); // pointer to the heap's volatile descriptor for quick lookup
    }

    void set_extentheap(ExtentHeap<Context, TPtr, PPtr>* extentheap) {
        header_.extentheap_ = reinterpret_cast<void*>(header_.extentheap_);
    }

private:
    size_t find_extent(size_t begin, size_t end, ExtentInterval* extent_interval, bool* extent_is_free) 
    {
        size_t i;
        size_t ext_begin = begin;
        size_t ext_end = begin;

        *extent_is_free = false;
        for (i=begin; i<end; i++) {
            TPtr<nvExtentHeader<Context, TPtr> > exh = extent_header(i);
            if (exh->type_ == nvExtentHeader<Context, TPtr>::kBlockTypeFree) {
                *extent_is_free = true;
                ext_end = i + 1;
            } else if (exh->type_ == nvExtentHeader<Context, TPtr>::kBlockTypeExtentFirst) {
                if (ext_end > ext_begin) {
                    // report the free extent we found before this one
                    break;
                } else {
                    ext_begin = i;
                    ext_end = ext_begin + exh->size_;
                    break;
                }
            }
        }

        size_t ext_len = ext_end - ext_begin;
        *extent_interval = ExtentInterval(ext_begin, ext_len);

        return ext_begin;
    }

public:
    class Iterator {
    public:
        Iterator()
        { }

        Iterator(TPtr<nvExtentHeap<Context, TPtr, PPtr>> nvexheap, size_t begin, size_t end, size_t cur)
            : nvexheap_(nvexheap),
              begin_(begin),
              end_(end),
              cur_(cur),
              next_cur_(cur)
        {
            next();
        }

        Iterator& operator++()
        {
            next();
            return *this;
        }  

        bool operator==(const Iterator& other) 
        {
            return begin_ == other.begin_ && end_ == other.end_ && cur_ == other.cur_;
        }

        bool operator!=(const Iterator& other) 
        {
            return !(*this == other);
        }

        ExtentInterval operator*()
        {
            return cur_extent_interval_;
        }

        void next()
        {
            bool extent_is_free;
            cur_ = nvexheap_->find_extent(next_cur_, end_, &cur_extent_interval_, &extent_is_free); 
            if (cur_ < end_) {
                next_cur_ = cur_extent_interval_.start() + cur_extent_interval_.len();
            }
        }

        void stream_to(std::ostream& os) const {
            os << "(" << begin_ << ", " << end_ << ", " << cur_ << ")";
        }

        std::string to_string() const {
            std::stringstream ss;
            stream_to(ss);
            return ss.str();
        }

    //private:
        TPtr<nvExtentHeap<Context, TPtr, PPtr>> nvexheap_;
        size_t                         begin_;
        size_t                         end_;
        size_t                         cur_;
        size_t                         next_cur_;
        ExtentInterval                 cur_extent_interval_;
    };

    Iterator begin() 
    {
        return Iterator(this, 0, header_.nblocks, 0);
    }

    Iterator end()
    {
        return Iterator(this, 0, header_.nblocks, header_.nblocks);
    }

//private:
    struct nvExtentHeapHeader header_; 
    uint8_t                   payload_[0];
};

} // namespace alps

#endif /* _ALPS_LAYERS_NVEXTENTHEAP_HH_ */
