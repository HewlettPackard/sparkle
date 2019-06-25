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

#ifndef _ALPS_GLOBALHEAP_FREESPACEMAP_HH_
#define _ALPS_GLOBALHEAP_FREESPACEMAP_HH_

#include "alps/common/error_stack.hh"

#include "globalheap/extent.hh"
#include "globalheap/extentmap.hh"

namespace alps {

class FreeSpaceMap: public ExtentMap {
public:
    FreeSpaceMap(RRegion::TPtr<nvZone> nvzone)
        : nvzone_(nvzone)
    { }
 
    ErrorStack init();

    bool exists_extent(size_t size_nblocks);
    int alloc_extent(size_t size_nblocks, Extent* ex);
    void free_extent(const Extent& ex);

private:
    RRegion::TPtr<nvZone> nvzone_;
};

} // namespace alps

#endif // _ALPS_GLOBALHEAP_FREESPACEMAP_HH_
