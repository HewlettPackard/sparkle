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

#ifndef _ALPS_PEGASUS_ADDR_HH_
#define _ALPS_PEGASUS_ADDR_HH_

#include <iostream>

#include "alps/pegasus/region.hh"

namespace alps {

// linear address
typedef uint64_t LinearAddr;

// represents a persistent linear address 
struct PAddr {

    LinearAddr linear_addr_; // linear address relative to the region

    // null_ptr
    PAddr()
        : linear_addr_(1)
    { }

    PAddr(LinearAddr linear_addr)
        : linear_addr_(linear_addr)
    { }

    bool operator==(const PAddr& other) const {
        return other.linear_addr_ == linear_addr_;
    }

    bool operator!=(const PAddr& other) const {
        return !(*this == other);
    }

    void stream_to(std::ostream& os) const
    {
        std::ios::fmtflags f(os.flags());
        os << std::hex << "(" << "0x" << linear_addr_ << ")";
        os.flags(f);
    }
};


inline std::ostream& operator<<(std::ostream& os, const PAddr& paddr)
{
    paddr.stream_to(os);
    return os;
}




// represents a Zeta (128-bit) far/fat persistent linear address
struct ZAddr {
    ZAddr()
        : region_id_(-1),
          linear_addr_(-1)
    { } 

    ZAddr(RegionId region_id, LinearAddr linear_addr)
        : region_id_(region_id),
          linear_addr_(linear_addr)
    { }

    bool operator==(const ZAddr& other) const {
        return other.linear_addr_ == linear_addr_ && other.region_id_ == region_id_;
    }

    bool operator!=(const ZAddr& other) const {
        return !(*this == other);
    }

    void stream_to(std::ostream& os) const
    {
        std::ios::fmtflags f(os.flags());
        os << std::hex << "(" << "0x" << region_id_ << ", " << "0x" << linear_addr_ << ")";
        os.flags(f);
    }

    RegionId   region_id_;  // persistent memory region identifier
    LinearAddr linear_addr_; // linear address relative to the region
};

inline std::ostream& operator<<(std::ostream& os, const ZAddr& zaddr)
{
    zaddr.stream_to(os);
    return os;
}



} // namespace alps

#endif // _ALPS_PEGASUS_ADDR_HH_
