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

#ifndef _ALPS_PEGASUS_MAPPABLE_HH_
#define _ALPS_PEGASUS_MAPPABLE_HH_

#include "../common/error_code.hh"
#include "pointer.hh"
#include "bits/segmap.hh"


namespace alps {

// forward declarations
class AddressSpace;
class RegionFile;

/**
 * \brief A template mixin class for defining mappable region types.
 * 
 * \details
 * The MemoryMapImpl class defines the mapping policy that implements
 * mapping of the underlying region file(s) onto the logical address space.
 *
 * The PointerImpl class defines smart pointer types for naming (addressing) 
 * and referencing locations within the region. 
 *
 * There is no notion of a root pointer baked into the API. However, users may 
 * agree on a convention where offset 0x0 represents a root, and instantiate a 
 * smart pointer that points to offset 0x0 and use that as a root pointer.
 *
 */
template<class RegionType, class MemoryMapImpl, class PointerImpl>
class Mappable: public RegionType {
friend class AddressSpace;
public:
    Mappable(AddressSpace* address_space, RegionFile* file);

public:
    // Pointer API     

    template<class T>
    using TPtr = typename PointerImpl::template TPtr<Mappable,T>;

    template<class T>
    using PPtr = typename PointerImpl::template PPtr<Mappable,T>;

    template<class T>
    using ZPtr = typename PointerImpl::template ZPtr<Mappable,T>;

public:
    // Mapping API

    ErrorCode map();
    ErrorCode unmap();

    template<class T>
    TPtr<T> base(LinearAddr offset)
    {
        //return TPtr<T>(this, offset);
        void* base_ptr = reinterpret_cast<void*>(mem_map_->trans(offset));
        return TPtr<T>(base_ptr);
    }

    uintptr_t trans(LinearAddr offset);
private:
    MemoryMapImpl*  mem_map_; // address space memory map 
};

} // namespace alps

#include "bits/mappable.tcc"

#endif // _ALPS_PEGASUS_MAPPABLE_HH_
