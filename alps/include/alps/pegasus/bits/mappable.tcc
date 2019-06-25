#ifndef _ALPS_PEGASUS_MAPPABLE_TCC_
#define _ALPS_PEGASUS_MAPPABLE_TCC_

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../common/error_code.hh"

namespace alps {


template<class RegionType, class MemoryMapImpl, class PointerImpl>
Mappable<RegionType, MemoryMapImpl, PointerImpl>::Mappable(AddressSpace* address_space, RegionFile* region_file)
    : RegionType(address_space, region_file)
{ }


template<class RegionType, class MemoryMapImpl, class PointerImpl>
ErrorCode Mappable<RegionType, MemoryMapImpl, PointerImpl>::map()
{
    CHECK_ERROR_CODE(this->file_->size(&(this->length_)));

    if (!(mem_map_ = new MemoryMapImpl(this))) {
        return kErrorCodeOutofmemory;
    }
 
    CHECK_ERROR_CODE(mem_map_->map());
    
    return kErrorCodeOk; 
}


template<class RegionType, class MemoryMapImpl, class PointerImpl>
ErrorCode Mappable<RegionType, MemoryMapImpl, PointerImpl>::unmap()
{
    CHECK_ERROR_CODE(mem_map_->unmap());

    return kErrorCodeOk; 
}


template<class RegionType, class MemoryMapImpl, class PointerImpl>
uintptr_t Mappable<RegionType, MemoryMapImpl, PointerImpl>::trans(LinearAddr offset)
{
    return mem_map_->trans(offset);
}


} // namespace alps


#endif // _ALPS_PEGASUS_MAPPABLE_TCC_
