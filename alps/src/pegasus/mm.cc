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

#include "pegasus/mm.hh"

#include "alps/common/error_code.hh"
#include "alps/pegasus/region.hh"

#include "pegasus/region_file.hh"
#include "pegasus/vmarea.hh"


namespace alps {


ErrorCode MemoryManager::map(Region* region, off_t offset, size_t length, void* addr_hint, int prot, int flags, VmArea** vmarea)
{
    void*    mapped_addr;

    CHECK_ERROR_CODE(region->file()->map(addr_hint, length, prot, flags, offset, &mapped_addr));

    uintptr_t vm_start = reinterpret_cast<uintptr_t>(mapped_addr);
    uintptr_t vm_end = reinterpret_cast<uintptr_t>(mapped_addr) + length;

    VmArea* vma = new VmArea(region, offset, vm_start, vm_end);
    if (!vma) {
        return kErrorCodeOutofmemory;
    }
    invtbl_.insert_vmarea(vma);   

    *vmarea = vma;
    return kErrorCodeOk;
}

ErrorCode MemoryManager::unmap(Region* region, void* addr, size_t length)
{
    CHECK_ERROR_CODE(region->file()->unmap(addr, length));

    uintptr_t vm_start = reinterpret_cast<uintptr_t>(addr);
    uintptr_t vm_end = reinterpret_cast<uintptr_t>(addr) + length;

    VmArea* vma = new VmArea(region, 0, vm_start, vm_end);
    if (!vma) {
        return kErrorCodeOutofmemory;
    }
    invtbl_.remove_vmarea(vma);   
    delete vma;

    return kErrorCodeOk;
}

ErrorCode MemoryManager::rtrans(void* addr, Region** pregion, LinearAddr* offset)
{
    uintptr_t uaddr = reinterpret_cast<uintptr_t>(addr);
    return rtrans(uaddr, pregion, offset);
}

ErrorCode MemoryManager::rtrans(uintptr_t addr, Region** pregion, LinearAddr* offset)
{
    VmArea* vma = invtbl_.find_vmarea(addr);
    if (vma == NULL) {
        return kErrorCodeMemoryInvalidAddress;
    }
    *offset = addr - vma->vm_start() + vma->offset();
    *pregion = vma->region();
    return kErrorCodeOk;
}

} // namespace alps
