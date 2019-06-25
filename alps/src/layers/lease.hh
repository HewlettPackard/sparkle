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

#ifndef _ALPS_LEASE_HH_
#define _ALPS_LEASE_HH_

#include <stdint.h>

//#include "alps/pegasus/relocatable_region.hh"


namespace alps {

typedef int64_t Generation;

struct LeaseSuperblock {
    //static RRegion::TPtr<LeaseSuperblock> make(RRegion::TPtr<LeaseSuperblock> superblock);
    //Generation incr_generation();

    union {
        Generation generation;    
        uint8_t    raw[64];
    };
};

#if 0
struct Lease {
    enum {
        kUnlocked = 0,
        kLocked = 1,
    };

    static RRegion::TPtr<Lease> make(RRegion::TPtr<Lease> lease);
    uint32_t lock_status();
    int try_lock(Generation owner);
    void unlock();

    union {
        int64_t lock;
        uint8_t raw[64];
    };
};

#endif

} // namespace alps

#endif // _ALPS_LEASE_HH_
