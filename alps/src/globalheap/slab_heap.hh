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

#ifndef _ALPS_GLOBALHEAP_SLABHEAP_HH_
#define _ALPS_GLOBALHEAP_SLABHEAP_HH_

#include "alps/common/assert_nd.hh"

#include "globalheap/extentheap.hh"
#include "globalheap/slab.hh"
#include "globalheap/nvslab.hh"

namespace alps {

// forward declaration
class Extent;
class ExtentHeap;
class Zone;


/**
 * @brief Slab heap organizes slabs in per-sizeclass free lists 
 *
 * @details 
 * This class methods are not-thread safe. User is responsible for proper
 * serialization via lock/unlock.
 * 
 */
class SlabHeap
{
public:
    SlabHeap(ExtentHeap* extentheap)
        : extentheap_(extentheap)
    { 
        ASSERT_ND(pthread_mutex_init(&mutex_, NULL) == 0);
    }

    RRegion::TPtr<void> alloc_block(Slab* slab);
    void free_block(Slab* slab, RRegion::TPtr<void> ptr);

    Slab* find_slab(const int szclass);
    Slab* insert_slab(RRegion::TPtr<nvSlab> nvslab);
    void insert_slab(Slab* slab, int szclass);
    void remove_slab(Slab* slab);
    void move_slab(Slab* slab, int szclass, int to);

    void stream_to(std::ostream& os) const;

    void lock();
    void unlock();

private:
    void insert_slab_to_empty(Slab* slab);
    Slab* reuse_empty_slab(int szclass);

protected:
    pthread_mutex_t mutex_;
    ExtentHeap*     extentheap_;
    SlabList        full_slabs_[kSizeClasses][kSlabFullnessBins]; // completely or partially full slabs
    SlabList        empty_slabs_; // completely empty slabs (that can be reused as a different size class)
};

inline void SlabHeap::lock()
{
    pthread_mutex_lock(&mutex_);
}

inline void SlabHeap::unlock()
{
    pthread_mutex_unlock(&mutex_);
}

inline std::ostream& operator<<(std::ostream& os, const SlabHeap& slabheap)
{
    slabheap.stream_to(os);
    return os;
}


class InsertSlabFunctor {
public:
    InsertSlabFunctor(SlabHeap* slabheap)
        : slabheap_(slabheap)
    { }

    void operator()(Zone* zone, Extent* extent);

private:
    SlabHeap* slabheap_;
};

} // namespace alps

#endif // _ALPS_GLOBALHEAP_SLABHEAP_HH_
