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

#include "globalheap/helper.hh"
#include "alps/pegasus/relocatable_region.hh"
#include "pegasus/topology.hh"
#include "pegasus/topology_factory.hh"
#include "globalheap/extent.hh"
#include "globalheap/layout.hh"

namespace alps {

size_t find_extent(RRegion::TPtr<nvZone> nvzone, size_t start, size_t end, Extent* extent, bool* extent_is_free) 
{
    size_t i;
    size_t ext_start = start;
    size_t ext_end = start;

    *extent_is_free = false;
    for (i=start; i<end; i++) {
        RRegion::TPtr<nvBlockHeader> bh = nvzone->block_header(i);
        if (bh->primary_type == nvBlockHeader::kBlockTypeFree) {
            *extent_is_free = true;
            ext_end = i + 1;
        } else if (bh->primary_type == nvBlockHeader::kBlockTypeExtentFirst) {
            if (ext_end > ext_start) {
                // report the free extent we found before this one
                break;
            } else {
                ext_start = i;
                ext_end = ext_start + bh->size;
                break;
            }
        }
    }

    size_t ext_len = ext_end - ext_start;
    *extent = Extent(ext_start, ext_len);

    return ext_start;
}

ErrorCode create_region(const std::vector<boost::filesystem::path>& pathnames, size_t region_size, size_t interleave_size)
{
    if (pathnames.size() > 1) {
        size_t region_file_size = region_size / pathnames.size();
        CHECK_ERROR_CODE(create_region_files(pathnames, region_file_size, S_IRUSR | S_IWUSR, true, true, interleave_size));
    } else {
        size_t region_file_size = region_size;
        CHECK_ERROR_CODE(create_region_file(pathnames[0], region_file_size, S_IRUSR | S_IWUSR, true, interleave_size));
    }
    return kErrorCodeOk;
}

ErrorCode map_region(const std::vector<boost::filesystem::path>& pathnames, RRegion** pregion)
{
    RegionFile* region_file;
    RRegion*    region;

    if (pathnames.size() > 1) {
        CHECK_ERROR_CODE(Pegasus::open_region_file(pathnames, O_RDWR, &region_file));
    } else {
        CHECK_ERROR_CODE(Pegasus::open_region_file(pathnames[0], O_RDWR, &region_file));
    }
    CHECK_ERROR_CODE(Pegasus::address_space()->map(region_file, &region));
    *pregion = region;
    return kErrorCodeOk;
}



} // namespace alps
