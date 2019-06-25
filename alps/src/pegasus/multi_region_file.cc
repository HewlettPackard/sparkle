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

#include "pegasus/multi_region_file.hh"

#include <algorithm>

#include "alps/common/error_code.hh"
#include "alps/common/error_stack.hh"

#include "common/debug.hh"
#include "pegasus/region_file.hh"


namespace alps {

ErrorCode MultiRegionFile::create(mode_t mode)
{
    return kErrorCodeNotSupported;
}

ErrorCode MultiRegionFile::open(int flags, mode_t mode)
{
    std::vector<RegionFile*>::iterator it;

    for (it = region_files_.begin(); it != region_files_.end(); it++) {
        RegionFile* rfile = *it;
        CHECK_ERROR_CODE(rfile->open(flags, mode));
    }
    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::open(int flags)
{
    std::vector<RegionFile*>::iterator it;

    for (it = region_files_.begin(); it != region_files_.end(); it++) {
        RegionFile* rfile = *it;
        CHECK_ERROR_CODE(rfile->open(flags));
    }
    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::unlink()
{
    std::vector<RegionFile*>::iterator it;

    for (it = region_files_.begin(); it != region_files_.end(); it++) {
        RegionFile* rfile = *it;
        CHECK_ERROR_CODE(rfile->unlink());
    }
    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::close()
{
    std::vector<RegionFile*>::iterator it;

    for (it = region_files_.begin(); it != region_files_.end(); it++) {
        RegionFile* rfile = *it;
        CHECK_ERROR_CODE(rfile->close());
    }
    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::truncate(loff_t length)
{
    return kErrorCodeNotSupported;
}

ErrorCode MultiRegionFile::size(loff_t* length)
{
    std::vector<RegionFile*>::iterator it;

    *length = 0;
    for (it = region_files_.begin(); it != region_files_.end(); it++) {
        RegionFile* rfile = *it;
        loff_t      rfile_len;
        CHECK_ERROR_CODE(rfile->size(&rfile_len));
        *length += rfile_len;
    }
    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::map(void* addr_hint, size_t length, int prot, int flags, loff_t offset, void** mapped_addr)
{
    // Allocate a hole large enough to fit all the required region files contiguously 
    // and then map each individual file into the hole
    ErrorCode rc;
    uintptr_t uaddr_hint;
    void*     hole_base;
    std::vector<std::tuple<void*, loff_t> > mapped_addr_vec;

    hole_base = ::mmap(addr_hint, length, prot, flags|MAP_ANONYMOUS, 0, 0);

    uaddr_hint = reinterpret_cast<uintptr_t>(hole_base);
    loff_t pos = 0;
    for (std::vector<RegionFile*>::iterator it = region_files_.begin(); 
         it != region_files_.end(); 
         it++) 
    {
        size_t map_length;
        loff_t map_offset;
        RegionFile* rfile = *it;
        loff_t      rfile_len;
        CHECK_ERROR_CODE(rfile->size(&rfile_len));
        if (offset >= pos && offset < pos + rfile_len) {
            // left most file mapping
            map_length = std::min((loff_t) length, pos + rfile_len - offset);
            map_offset = offset - pos;
        } else if (pos > offset && pos < offset + (loff_t) length) {
            // rest of file mappings
            map_length = std::min((loff_t) rfile_len, offset + (loff_t) length - pos);
            map_offset = 0;
        } else {
            // no mapping
            map_length = 0;
            map_offset = 0;
        }
        if (map_length > 0) {
            void* mpaddr;
            addr_hint = reinterpret_cast<void*>(uaddr_hint);
            if (kErrorCodeOk != (rc = rfile->map(addr_hint, map_length, prot, flags|MAP_FIXED, map_offset, &mpaddr))) {
                // FIXME: undo map
                return rc;
            }
            uaddr_hint = uaddr_hint + map_length;
            mapped_addr_vec.push_back(std::make_tuple(mpaddr, map_length));
        }
        pos += rfile_len;
    }

    // verify that files have been mapped contiguously
    for (unsigned int i=1; i < mapped_addr_vec.size(); i++) {
        uintptr_t prev_mapaddr = reinterpret_cast<uintptr_t>(std::get<0>(mapped_addr_vec[i-1]));
        loff_t    prev_length = std::get<1>(mapped_addr_vec[i-1]);
        uintptr_t cur_mapaddr = reinterpret_cast<uintptr_t>(std::get<0>(mapped_addr_vec[i]));
        if (prev_mapaddr + prev_length != cur_mapaddr) {
            LOG(warning) << "Cannot map region files contiguously";
            // FIXME: undo map
            return kErrorCodeMemoryMapFailed;
        }
    }

    *mapped_addr = std::get<0>(mapped_addr_vec[0]);

    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::unmap(void* addr, size_t length)
{
    if (munmap(addr, length)) {
        return kErrorCodeMemoryUnmapFailed;
    }
    return kErrorCodeOk;
}

ErrorCode MultiRegionFile::getxattr(const char *name, void *value, size_t size)
{
    return kErrorCodeNotSupported;
}

ErrorCode MultiRegionFile::setxattr(const char *name, const void *value, size_t size, int flags)
{
    return kErrorCodeNotSupported;
}

size_t MultiRegionFile::booksize()
{
    return region_files_[0]->booksize();
}

ErrorCode MultiRegionFile::set_interleave_group(loff_t offset, loff_t length, const std::vector<InterleaveGroup>& vig)
{
    return kErrorCodeNotSupported;
}

ErrorCode MultiRegionFile::interleave_group(loff_t offset, loff_t length, std::vector<InterleaveGroup>* vig)
{
    std::vector<RegionFile*>::iterator it;
    loff_t cur_pos; // current position in the logical range comprised of multi region files

    for (it = region_files_.begin(), cur_pos = 0;
         it != region_files_.end();
         it++)
    {   
        RegionFile* file = *it;
        loff_t file_size;
        CHECK_ERROR_CODE(file->size(&file_size));

        // ----+-------------+-------------+-------------+----
        //     |             |             |             |
        // ----+-------------+-------------+-------------+----
        //           ^_____________________________^
        //           offset                        offset + length
        //           | CASE 1| CASE 2      | CASE 3|

        loff_t local_offset;
        loff_t local_length;
        if (offset >= cur_pos && ((offset + length) > (cur_pos + file_size))) {
            // CASE 1
            local_offset = offset - cur_pos;
            local_length = cur_pos + file_size - offset;
        } 
        else if (cur_pos > offset && (cur_pos + file_size < offset + length)) {
            // CASE 2
            local_offset = 0;
            local_length = file_size;
        } else if (cur_pos > offset && (cur_pos < offset + length)) {
            // CASE 3
            local_offset = 0;
            local_length = offset + length - cur_pos;
        } else {
            assert(0);
        }
        std::vector<InterleaveGroup> local_vig;
        CHECK_ERROR_CODE(file->interleave_group(local_offset, local_length, &local_vig));
        vig->insert(vig->end(), local_vig.begin(), local_vig.end());
        cur_pos += file_size;
    } 
    return kErrorCodeOk;
}

} // namespace alps
