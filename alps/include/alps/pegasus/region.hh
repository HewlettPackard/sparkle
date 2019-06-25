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

#ifndef _ALPS_PEGASUS_REGION_BASE_HH_
#define _ALPS_PEGASUS_REGION_BASE_HH_

#include <vector>

#include "alps/pegasus/interleave_group.hh"

namespace alps {

//forward declarations
class AddressSpace;
class RegionFile;

typedef uint64_t RegionId;

/**
 * \brief Persistent memory region.
 *
 * \details
 * A base class that represents a persistent memory region in the 
 * logical address space (alps::AddressSpace).
 *
 * A region is instantiated by mapping (and optionally binding) a region file 
 * (or a set of region files) to an AddressSpace object.
 * 
 */
class Region {
public:
    Region(AddressSpace* address_space, RegionFile* region_file)
      : region_id_(0xBEEF),
        address_space_(address_space),
        file_(region_file)
    { }

    /**
     * \brief Returns the region file backing this region.
     */
    RegionFile* file()
    {
        return file_;
    }

    loff_t length()
    {
        return length_;
    }
 
    /**
     * \brief Returns a global identifier identifying the region.
     */
    RegionId id()
    {
        return region_id_;
    }
 
    /**
     * \brief Returns the Address Space object the region is mapped to.
     */
    AddressSpace* address_space()
    {
        return address_space_;
    }
 
    /**
     * \brief Sets interleave group hint for pages mapped in the range 
     * [\a offset, \a offset + \a length).
     */
    ErrorCode set_interleave_group(loff_t offset, loff_t length, const std::vector<InterleaveGroup>& vig);

    /**
     * \brief Returns assigned interleave group for pages mapped in the 
     * range [\a offset, \a offset + \a length).
     */
    ErrorCode interleave_group(loff_t offset, loff_t length, std::vector<InterleaveGroup>* vig);
 
protected:
    RegionId      region_id_;
    AddressSpace* address_space_;
    loff_t        length_;        ///< cached value of the backing file's length
    RegionFile*   file_;          ///< the file backing the persistent memory region
};

} // namespace alps

#endif // _ALPS_PEGASUS_REGION_BASE_HH_
