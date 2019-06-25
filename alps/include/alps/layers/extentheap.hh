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

#ifndef _ALPS_LAYERS_EXTENTHEAP_HH_
#define _ALPS_LAYERS_EXTENTHEAP_HH_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <map>

#include "alps/common/assorted_func.hh"
#include "alps/common/error_code.hh"
#include "alps/common/error_stack.hh"
#include "alps/layers/bits/extentinterval.hh"
#include "alps/layers/bits/freespacemap.hh"
#include "alps/layers/bits/nvextentheap.hh"


namespace alps {

template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
class ExtentHeap;


template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
class Extent {
    friend ExtentHeap<Context, TPtr, PPtr>;
public:
    Extent()
        : exheap_(NULL)
    { }

    Extent(ExtentHeap<Context, TPtr, PPtr>* exheap, size_t start, size_t len)
        : exheap_(exheap),
          interval_(start, len)
    { 
        init();
    }

    size_t start() const { return interval_.start(); }
    size_t len() const { return interval_.len(); }
    size_t end() const { return interval_.end(); }

    ExtentInterval interval()
    {
        return interval_;
    }

    TPtr<nvExtentHeader<Context, TPtr>> nvheader()
    {
        return nvextentheader_;
    }

    TPtr<void> nvextent()
    {
        return nvextent_;
    }

    bool operator==(const Extent<Context, TPtr, PPtr> &other) const 
    {  
        return interval_ == other.interval_;  
    }

    void stream_to(std::ostream& os) const 
    {
        os << interval_;
    }

private:
    void init()
    {
        TPtr<nvExtentHeap<Context, TPtr, PPtr>> nvexheap = exheap_->nvexheap_;
        nvextentheader_ = static_cast<TPtr<nvExtentHeader<Context, TPtr>>>(nvexheap->extent_header(interval_.start()));
        nvextent_ = nvexheap->block(interval_.start());
    }

    void mark_alloc()
    {
        nvextentheader_->mark_alloc(interval_.len());
    }

    void mark_free()
    {
        nvextentheader_->mark_free();
    }
    
    ExtentHeap<Context, TPtr, PPtr>* exheap_;
    ExtentInterval interval_;
    TPtr<nvExtentHeader<Context,TPtr>> nvextentheader_;
    TPtr<void>                 nvextent_;
};

template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
inline std::ostream& operator<<(std::ostream& os, const Extent<Context, TPtr, PPtr>& ex)
{
    ex.stream_to(os);
    return os;
}


/**
 * @brief Manages a heap of extents
 */
template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
class ExtentHeap {
    friend Extent<Context, TPtr,PPtr>;

public:
    static ExtentHeap* make(TPtr<void> region, size_t region_size, size_t block_log2size)
    {
        ExtentHeap* exheap = new ExtentHeap;

        exheap->nvexheap_ = nvExtentHeap<Context, TPtr, PPtr>::make(region, region_size, block_log2size);
        exheap->init();

        return exheap;
    }

    static ExtentHeap* load(TPtr<void> region)
    {
        ExtentHeap* exheap = new ExtentHeap;

        exheap->nvexheap_ = nvExtentHeap<Context, TPtr, PPtr>::load(region);
        exheap->init();

        return exheap;
    }

    uint64_t blocksize()
    {
        return 1 << nvexheap_->header_.block_log2size_;
    }

    ErrorCode extent(ExtentInterval interval, Extent<Context, TPtr,PPtr>* ex)
    {
        *ex = Extent<Context, TPtr, PPtr>(this, interval.start(), interval.len());
        return kErrorCodeOk;
    }

    ErrorCode extent(TPtr<void> ptr, Extent<Context, TPtr, PPtr>* ex)
    {
        TPtr<nvBlock> nvblock = ptr;

        if (nvblock < nvexheap_->block(0) || 
            nvblock >= nvexheap_->block(0) + nvexheap_->header_.region_size_)
        {
            return kErrorCodeMemoryInvalidAddress;
        }
        uintptr_t diff = nvblock - nvexheap_->block(0);
        size_t idx = diff >> nvexheap_->header_.block_log2size_;
        TPtr<nvExtentHeader<Context, TPtr>> exhdr = nvexheap_->extent_header(idx);
        *ex = Extent<Context, TPtr, PPtr>(this, idx, exhdr->size());

        return kErrorCodeOk;
    }

    ErrorCode alloc_extent(Context& ctx, size_t size_nblocks, Extent<Context, TPtr, PPtr>* ex)
    {
        ExtentInterval exintv;
        if (fsmap_.alloc_extent(size_nblocks, &exintv) == 0) {
            *ex = Extent<Context, TPtr, PPtr>(this, exintv.start(), exintv.len());
            ex->mark_alloc();
            LOG(info) << "Allocated extent: " << ex;
            return kErrorCodeOk;
        }
        return kErrorCodeOutofmemory;
    }

    ErrorCode free_extent(Context& ctx, Extent<Context, TPtr, PPtr>& ex)
    {
        fsmap_.free_extent(ex.interval());
        ex.mark_free();
        return kErrorCodeOk;
    }

    ErrorCode free_extent(Context& ctx, TPtr<void> ptr)
    {
        Extent<Context,TPtr,PPtr> ex;
        CHECK_ERROR_CODE(extent(ptr, &ex));
        return free_extent(ctx, ex);
    }

    ErrorCode malloc(Context& ctx, size_t size_bytes, TPtr<void>* ptr)
    {
        ErrorCode rc;
        Extent<Context,TPtr,PPtr> ex;

        pthread_mutex_lock(&mutex_);

        // round up to next multiple of block_size
        size_t size_nblocks = size_bytes / blocksize() + (size_bytes % blocksize() ? 1: 0);

        rc = alloc_extent(ctx, size_nblocks, &ex);
        if (rc == kErrorCodeOk) {
            *ptr = ex.nvextent();
        }

        pthread_mutex_unlock(&mutex_);
        return rc;
    }

    void free(Context& ctx, TPtr<void> ptr)
    {
        pthread_mutex_lock(&mutex_);
        ErrorCode rc = free_extent(ctx, ptr);
        ASSERT_ND(rc == kErrorCodeOk);
        pthread_mutex_unlock(&mutex_);
    }

    size_t getsize(TPtr<void> ptr)
    {
        Extent<Context,TPtr,PPtr> ex;
        if (kErrorCodeOk != extent(ptr, &ex)) {
            return 0;
        }
        return blocksize() * ex.len();
    }

public:
    class Iterator {
    public:
        Iterator()
        { }

        Iterator(ExtentHeap<Context, TPtr, PPtr>* exheap, typename nvExtentHeap<Context, TPtr, PPtr>::Iterator nvit)
            : exheap_(exheap),
                //nvexheap_(exheap->nvexheap_),
              nvit_(nvit)
        { }

        Iterator& operator++()
        {
            ++nvit_;
            return *this;
        }  

        Extent<Context, TPtr, PPtr> operator*()
        {
            ExtentInterval interval = *nvit_;
            return Extent<Context, TPtr, PPtr>(exheap_, interval.start(), interval.len());
        }

        bool operator==(const Iterator& other) 
        {
            return nvit_ == other.nvit_;
        }

        bool operator!=(const Iterator& other) 
        {
            return nvit_ != other.nvit_;
        }

        ExtentHeap<Context, TPtr, PPtr>* exheap_;
        typename nvExtentHeap<Context, TPtr, PPtr>::Iterator nvit_;
    };

    Iterator begin()
    {
        return Iterator(this, nvexheap_->begin());
    }

    Iterator end()
    {
        return Iterator(this, nvexheap_->end());
    }

private:

    ErrorStack init()
    {
        pthread_mutex_init(&mutex_, NULL);
        typename nvExtentHeap<Context, TPtr, PPtr>::Iterator it;
        for (it = nvexheap_->begin(); it != nvexheap_->end(); ++it) {
            if (nvexheap_->is_free(*it)) {
                LOG(info) << "Load free extent: " << *it;
                fsmap_.insert(*it);
            }
        }
        return kRetOk;
    }

private:
    pthread_mutex_t mutex_;
    TPtr<nvExtentHeap<Context, TPtr, PPtr>> nvexheap_;
    FreeSpaceMap<TPtr> fsmap_;        
};


} // namespace alps

#endif // _ALPS_LAYERS_EXTENTHEAP_HH_
