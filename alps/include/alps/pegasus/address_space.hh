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

#ifndef _ALPS_PEGASUS_ADDRESS_SPACE_HH_
#define _ALPS_PEGASUS_ADDRESS_SPACE_HH_

#include <sys/types.h>
#include <map>
#include <vector>

#include "../common/error_code.hh"

#include "addr.hh"
#include "address_space_options.hh"


namespace alps {

// forward declarations
class MemoryManager;
class Region;
class RegionFile;



/**
 * \brief Logical address space. 
 *
 * \details
 * Represents a logical address space where region files can be bound and 
 * mapped to. There should be a single AddressSpace object instance per 
 * process even though nothing prevents a user from having multiple instances.
 *
 * Mapping and binding a region file to a global address space results in a 
 * named region. The global address space supports different modes of region 
 * mappings to allow users select different performance and flexibility tradeoffs. 
 * These modes are supported through template polymorphism rather than virtual 
 * polymorphish to reduce the runtime overhead for accessing regions.
 * 
 * Mapping modes include:
 * - single segment direct relocatable mapping, where the whole region is 
 *   directly mapped to an arbitraty address. This mode enables using 
 *   relative offset pointers (similar to offset_ptr) for flexible mapping 
 *   and addressing at the expense of performance (i.e., some overhead to perform 
 *   simple pointer arithmetic relative to the base of region). 
 *
 */
class AddressSpace {
public:
    AddressSpace(const AddressSpaceOptions& address_space_options);

    ErrorCode init();

     /**
     * \brief Maps a region file into the global address space.
     *
     * \param[in] region_file The region file to map into the global address space.
     * \param[out] pregion An object representing the region of the global address 
     * space where the file is mapped to. 
     */
    template<class RegionT>
    ErrorCode map(RegionFile* region_file, RegionT** pregion);

    /**
     * \brief Binds a region to a shorthand name identifier.
     *
     * \param[in] region The region object to bind.
     * \param[in] region_id The shorthand name identifier to bind the region to.
     *
     * \details
     * After binding, pointers can identify the region using the shorthand 
     * name identifier.
     */
    template<class RegionT>
    ErrorCode bind(RegionT* region, RegionId region_id);

    /**
     * \brief Unmaps a previously mapped region.
     *
     * \param[in] region The region object to unmap.
     */
    template<class RegionT>
    ErrorCode unmap(RegionT* region);

    /**
     * \brief Returns region identified by region identifier \a region_id.
     *
     * \param[in] region_id Region identifier 
     */
    Region* region(RegionId region_id);

    /**
     * \brief Performs reverse translation of a virtual address to 
     * a <\a pregion, \a offset> pair.
     *
     * \param[in] vaddr The virtual address to reverse translate.
     * \param[out] pregion The region the virtual address is mapped to.
     * \param[out] offset The offset relative to the region base.
     */
    ErrorCode rtrans(void* vaddr, Region** pregion, LinearAddr* offset);

    MemoryManager* mm();

//private:
    AddressSpaceOptions              address_space_options_; ///< Address space runtime options
    MemoryManager*                   mm_; ///< Memory manager mapping regions into virtual memory
    std::multimap<RegionId, Region*> regions_; ///< A table mapping region_id to region
};


inline MemoryManager* AddressSpace::mm()
{
    return mm_;
}


} // namespace alps

#endif // _ALPS_PEGASUS_ADDRESS_SPACE_HH_
