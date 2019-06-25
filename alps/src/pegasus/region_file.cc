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

#include "pegasus/region_file.hh"

#include <vector>

#include "alps/common/assorted_func.hh"
#include "alps/pegasus/interleave_group.hh"

#include "common/debug.hh"

namespace alps {

ErrorCode RegionFile::set_interleave_group(loff_t offset, loff_t length, const std::vector<InterleaveGroup>& vig)
{
    loff_t file_size;

    loff_t begin = round_down(offset, booksize());
    loff_t end = round_up(offset+length, booksize());

    CHECK_ERROR_CODE(size(&file_size));
    loff_t old_num_books = file_size / booksize();
    loff_t new_num_books = (end > file_size) ? end / booksize() : file_size / booksize();
    uint8_t* igbuf = new uint8_t[new_num_books];

    CHECK_ERROR_CODE(setxattr("user.LFS.AllocationPolicy", "RequestIG", 9, 0));

    if (getxattr("user.LFS.Interleave", igbuf, old_num_books) != kErrorCodeOk) {
        // attribute not set -- consider it zero
        for (int i=0; i<new_num_books; i++) {
            igbuf[i] = 0;
        }
    }

    // set the interleave groups for books in the range [offset, offset+length)
    int idx;
    loff_t off;
    for (off = begin, idx = begin/booksize(); 
         off < end; off += booksize(), 
         idx++) 
    {
        igbuf[idx] = vig[idx % vig.size()];
    }
    CHECK_ERROR_CODE2(setxattr("user.LFS.InterleaveRequest", igbuf, new_num_books, 0), delete[] igbuf);

    delete[] igbuf;

    return kErrorCodeOk;
}


ErrorCode RegionFile::interleave_group(loff_t offset, loff_t length, std::vector<InterleaveGroup>* vig)
{
    loff_t file_size;

    CHECK_ERROR_CODE(size(&file_size));

    loff_t begin = round_down(offset, booksize());
    loff_t end = round_up(std::min(file_size, offset+length), booksize());
    loff_t roundup_file_size = round_up(file_size, booksize());
    loff_t total_num_books = (roundup_file_size) / booksize();
    uint8_t* igbuf = new uint8_t[total_num_books];

    CHECK_ERROR_CODE2(getxattr("user.LFS.Interleave", igbuf, total_num_books), delete[] igbuf);

    int idx;
    loff_t off;
    for (off = begin, idx = begin/booksize(); 
         off < end; off += booksize(), 
         idx++) 
    {
        vig->push_back(igbuf[idx]);
    }
    
    delete [] igbuf;
    
    return kErrorCodeOk;
}

} // namespace alps
