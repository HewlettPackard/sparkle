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

#include <fcntl.h>
#include <sys/file.h>

#ifdef __ARCH_FAM__
#include <fam_atomic.h>
#endif // __ARCH_FAM__

#include "alps/common/assorted_func.hh"

#include "globalheap/layout.hh"

namespace alps {



#ifdef __ARCH_FAM__
RRegion::TPtr<LeaseSuperblock> LeaseSuperblock::make(RRegion::TPtr<LeaseSuperblock> superblock)
{
    fam_atomic_64_write(&superblock->generation, 1);
    return superblock;
}


Generation LeaseSuperblock::incr_generation()
{
    Generation gen = fam_atomic_64_fetch_add(&generation, 1);

    return gen;
}


RRegion::TPtr<Lease> Lease::make(RRegion::TPtr<Lease> lease) 
{
    fam_atomic_64_write(&lease->lock, kUnlocked);
    return lease;
} 


uint32_t Lease::lock_status()
{
    return fam_atomic_64_read(&lock);
}


int Lease::try_lock(Generation owner)
{
    int ret;
    
    assert(is_aligned(&lock, kCacheLineSize) == true);

    if (fam_atomic_64_compare_store(&lock, kUnlocked, owner) == kUnlocked) {
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}


void Lease::unlock()
{
    fam_atomic_64_write(&lock, kUnlocked);
}

#endif // __ARCH_FAM__


#ifndef __ARCH_FAM__
RRegion::TPtr<LeaseSuperblock> LeaseSuperblock::make(RRegion::TPtr<LeaseSuperblock> superblock)
{
    superblock->generation = 1;
    return superblock;
}


Generation LeaseSuperblock::incr_generation()
{
    Generation gen = __atomic_add_fetch(&generation, 1, __ATOMIC_SEQ_CST);

    return gen;
}


RRegion::TPtr<Lease> Lease::make(RRegion::TPtr<Lease> lease) 
{
    lease->lock = 0;
    return lease;
} 


uint32_t Lease::lock_status()
{
    return lock;
}


int Lease::try_lock(Generation owner)
{
    int ret;
    uint64_t expected = 0;
    
    assert(is_aligned(&lock, kCacheLineSize) == true);

    if (__atomic_compare_exchange_n(&lock, &expected, owner, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}


void Lease::unlock()
{
    __atomic_store_n(&lock, kUnlocked, __ATOMIC_SEQ_CST);
}

#endif // !__ARCH_FAM__

} // namespace alps
