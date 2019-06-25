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

#ifndef _ALPS_PEGASUS_SEGMENT_MAP_HH_
#define _ALPS_PEGASUS_SEGMENT_MAP_HH_

#include <stdint.h>
#include "../../common/error_code.hh"
#include "../addr.hh"

// TODO: the last segment might no be mapped completely to a region 
// (if not available books) so we should mprotect that part to 
// effectively generate a seg fault.

// TODO: support multiple segments. mprotect the segment table for 
// segments not mapped to generate a segfault upon address translation
// or include a base address that generates an invalid (unmapped) virtual 
// address for non-mapped segments

namespace alps {

// forward declarations
class MemoryManager;
class Region;

typedef void* Segment;


/**
 * @brief A mapper that directly memory maps a region into a single 
 * contiguous segment of virtual memory.
 */
class SegmentMap {
public:
    SegmentMap(Region* region)
        : region_(region)
    { }

    // translate linear address to virtual address
    uintptr_t trans(LinearAddr offset)
    {
        return tbl_[0] + offset;
    }

    ErrorCode map();
    ErrorCode unmap();
private:
  
    Region*   region_; ///< Region mapped by this segment mapper 
    uintptr_t tbl_[1]; ///< Table of base addresses of segments mapping the region
};



} // namespace alps

#endif // _ALPS_PEGASUS_SEGMENT_MAP_HH_
