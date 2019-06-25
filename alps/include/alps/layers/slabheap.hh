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

#ifndef _ALPS_LAYER_SLABHEAP_HH_
#define _ALPS_LAYER_SLABHEAP_HH_

#include "alps/common/assert_nd.hh"

#include "alps/layers/extentheap.hh"

#include "alps/layers/bits/slab.hh"

namespace alps {

/**
 * @brief Slab heap organizes slabs in per-sizeclass free lists 
 *
 * @details 
 * This class methods are not-thread safe. User is responsible for proper
 * serialization via lock/unlock.
 * 
 */
template<typename Context, template<typename> class TPtr, template<typename> class PPtr>
class SlabHeap
{
public:
    typedef Slab<Context, TPtr, PPtr> SlabT;
    typedef ExtentHeap<Context, TPtr, PPtr> ExtentHeapT;

public:
    SlabHeap(size_t slabsize)
        : slabsize_(slabsize)
    { 
        int err = pthread_mutex_init(&mutex_, NULL);
        ASSERT_ND(err == 0);
    }

    SlabHeap(size_t slabsize, SlabHeap* parentslabheap, ExtentHeapT* extentheap)
        : slabsize_(slabsize),
          parentslabheap_(parentslabheap),
          extentheap_(extentheap)
    {
        int err = pthread_mutex_init(&mutex_, NULL);
        ASSERT_ND(err == 0);
    }

    ErrorCode init(Context& ctx)
    {
        if (extentheap_) {
            for (typename ExtentHeapT::Iterator it = extentheap_->begin(); 
                 it != extentheap_->end(); ++it) 
            {
                if ((*it).nvheader()->is_free() && (*it).nvheader()->size() == slabsize_) 
                {
                    SlabT* slab = insert_slab(ctx, (*it).nvextent());
                    if (!slab) {
                        return kErrorCodeOutofmemory;    
                    }
                }
            }
        }
        return kErrorCodeOk;
    }

    ErrorCode malloc(Context& ctx, size_t size_bytes, TPtr<void>* ptr)
    {
        const int szclass = sizeclass(size_bytes);

        lock(); 
        SlabT* slab = find_slab(szclass);

        // No slab of requested sizeclass, so try to reuse an empty one.
        if (!slab) {
            slab = reuse_empty_slab(ctx, szclass);
        }

        // No slab in this heap so try to get a slab from the parent slab 
        // heap if we have one
        if (!slab && parentslabheap_) {
            slab = parentslabheap_->acquire_slab(ctx, szclass);
            if (slab) {
                insert_slab(slab, szclass);
            }
        }
 
        // No slab in parent heap so try to get a new chunk from the extent 
        // heap and format it as a slab
        if (!slab && extentheap_) {
            TPtr<void> region;
            if (extentheap_->malloc(ctx, slabsize_, &region) == kErrorCodeOk) {
                slab = SlabT::make(ctx, region, slabsize_, szclass);
                insert_slab(slab, szclass);
            }
        }
            
        if (slab) {
            *ptr = alloc_block(ctx, slab);
        }
        unlock(); 

        return kErrorCodeOk;
    }

    void free(Context& ctx, TPtr<void> ptr) 
    {
        Extent<Context,TPtr,PPtr> ex;
        ErrorCode rc = extentheap_->extent(ptr, &ex);
        ASSERT_ND(rc == kErrorCodeOk);

        SlabT* slab = SlabT::slab(ex.nvextent());

        // Expect this to finish after a few iterations as a slab that is 
        // moved between two slab heaps eventually ends up in a slabheap
        for (;;) {
            SlabHeap* owner = reinterpret_cast<SlabHeap*>(slab->owner());
            if (owner) {
                owner->lock();
                if (owner == slab->owner()) {
                    owner->free_block(ctx, slab, ptr);
                    owner->unlock();
                    break;
                }
                owner->unlock();
            }
        }
    }

    size_t getsize(TPtr<void> ptr) 
    {
        Extent<Context, TPtr, PPtr> ex;
        ErrorCode rc = extentheap_->extent(ptr, &ex);
        if (rc != kErrorCodeOk) {
            return 0;
        }
        size_t exsz = extentheap_->blocksize() * ex.len(); 
        if (exsz == slabsize_) {
            SlabT* slab = SlabT::slab(ex.nvextent());
            return slab->block_size();
        }
        return exsz;
    }

    TPtr<void> alloc_block(Context& ctx, SlabT* slab)
    {
        TPtr<void> ptr;

        bool empty = slab->empty();
        int old_fullness = slab->fullness();
        ptr = slab->alloc_block(ctx);
        if (!ptr) {
            int new_fullness = slab->fullness();
            int szclass = slab->sizeclass();
            LOG(info) << "Allocate block: " << "new_fullness: " << new_fullness << " old_fullness: " << old_fullness;
            if (empty || (new_fullness != old_fullness)) {
                move_slab(slab, szclass, new_fullness);
            }
        }
        return ptr;
    }

    void free_block(Context& ctx, SlabT* slab, TPtr<void> ptr)
    {
        int old_fullness = slab->fullness();
        slab->free_block(ctx, ptr);
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

    SlabT* acquire_slab(Context& ctx, int szclass)
    {
        SlabT* slab;

        lock();
        slab = find_slab(szclass);
        if (!slab) {
            slab = reuse_empty_slab(ctx, szclass);
        }
        if (slab) {
            remove_slab(slab);
            unlock();
            return slab;
        }

        if (extentheap_) {
            TPtr<void> region;
            if (extentheap_->malloc(ctx, slabsize_, &region) == kErrorCodeOk) {
                SlabT* slab = SlabT::make(ctx, region, slabsize_, szclass);
                unlock();
                return slab;
            }
        }
        unlock();
        return NULL;
    }

    SlabT* find_slab(const int szclass)
    {
        SlabT* slab = NULL;

        // Skip kSlabFullnessBins-1 slab list as it contains completely full slabs
        int fullness_hint = kSlabFullnessBins-2;

        // Find the most full slab 
        for (int i=fullness_hint; i>=0; i--) {
            typename SlabT::SlabList& sl = full_slabs_[szclass][i];
            if (sl.size()) {
                slab = sl.front();
                break;
            }
        }

        return slab;
    }

    SlabT* insert_slab(Context& ctx, TPtr<nvSlab<Context,TPtr>> nvslab)
    {
        LOG(info) << "Insert slab: " << nvslab;

        SlabT* slab = SlabT::load(ctx, nvslab);
        insert_slab(slab, nvslab->sizeclass());
        return slab;
    }


    void insert_slab(SlabT* slab, int szclass)
    {
        slab->set_owner(this);
        
        int fullness = slab->fullness();

        if (slab->empty()) {
            insert_slab_to_empty(slab);
        } else {
            slab->insert(&full_slabs_[szclass][fullness]);
        }
    }


    void remove_slab(SlabT* slab)
    {
        slab->set_owner(NULL);
        slab->remove();
    }

    void move_slab(SlabT* slab, int szclass, int to)
    {
        slab->remove();
        slab->insert(&full_slabs_[szclass][to]);
    }

    void stream_to(std::ostream& os) const
    {
        for (int i=0; i<kSlabFullnessBins; i++) {
            std::cout << "[" << i << "] ";
            bool comma = false;
            for (int c=0; c<kSizeClasses; c++) {
                for (typename SlabT::SlabList::const_iterator it = full_slabs_[c][i].begin();
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
        for (typename SlabT::SlabList::const_iterator it = empty_slabs_.begin();
             it != empty_slabs_.end();
             it++) 
        {
            if (comma) { std::cout << ", "; }
            std::cout << **it;
            comma = true;
        }
        std::cout << std::endl;
    }


    friend std::ostream& operator<<(std::ostream& os, const SlabHeap& slabheap)
    {
        slabheap.stream_to(os);
        return os;
    }

    void lock()
    {
        pthread_mutex_lock(&mutex_);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex_);
    }

private:
    void insert_slab_to_empty(SlabT* slab)
    {
        slab->insert(&empty_slabs_);
    }

    SlabT* reuse_empty_slab(Context& ctx, int szclass)
    {
        SlabT* slab = NULL;
        if (empty_slabs_.size()) {
            slab = empty_slabs_.front();
            int fullness = slab->fullness();
            move_slab(slab, szclass, fullness);
            if (slab->sizeclass() != szclass) {
                slab->reset(ctx, slabsize_, szclass);
            }
        }
        return slab;
    }

protected:
    size_t            slabsize_;
    SlabHeap*         parentslabheap_;
    ExtentHeapT*      extentheap_;
    pthread_mutex_t   mutex_;
    
    //! completely or partially full slabs
    typename SlabT::SlabList full_slabs_[kSizeClasses][kSlabFullnessBins]; 

    //! completely empty slabs (that can be reused as a different size class)
    typename SlabT::SlabList empty_slabs_; 
};

} // namespace alps

#endif // _ALPS_LAYER_SLABHEAP_HH_
