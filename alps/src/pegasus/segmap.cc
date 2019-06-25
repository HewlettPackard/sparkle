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

#include "alps/pegasus/bits/segmap.hh"
#include "alps/common/error_code.hh"
#include "alps/pegasus/address_space.hh"
#include "alps/pegasus/region.hh"

#include "common/debug.hh"
#include "pegasus/mm.hh"
#include "pegasus/region_file.hh"
#include "pegasus/vmarea.hh"

namespace alps {

ErrorCode SegmentMap::map()
{
    void*    addr_hint = NULL;
    int      prot = PROT_READ | PROT_WRITE;
    int      mflags = 0;
    VmArea*  vmarea;

    CHECK_ERROR_CODE(region_->address_space()->mm()->map(region_, 0, region_->length(), addr_hint, prot, mflags|MAP_SHARED, &vmarea));
    tbl_[0] = vmarea->vm_start();

    LOG(info) << "map region " << std::hex << " base_addr: 0x" << tbl_[0] << std::dec << " size: " << region_->length()
              << " addr_range: 0x" << std::hex << tbl_[0] << "--" << tbl_[0] + region_->length() << std::dec;
 
    return kErrorCodeOk;
}

ErrorCode SegmentMap::unmap()
{
    void* addr = reinterpret_cast<void*>(tbl_[0]);
    CHECK_ERROR_CODE(region_->address_space()->mm()->unmap(region_, addr, region_->length()));
    return kErrorCodeOk;
}

} // namespace alps
