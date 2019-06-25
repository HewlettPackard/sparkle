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

#ifndef _ALPS_PEGASUS_REGIONFILE_HH_
#define _ALPS_PEGASUS_REGIONFILE_HH_

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <map>
#include <vector>
#include <string>

#include "alps/common/error_code.hh"
#include "alps/pegasus/interleave_group.hh"

namespace alps {

class RegionFile;

/**
 * @brief Persistent region file.
 *
 * @details This base class implements a common interface to region files backed 
 * by different types of memory file systems (e.g., TMPFS, LFS, EXT4+DAX). 
 * The API method names are self-described; they do what you expect them to do.
 */
class RegionFile {
friend class RegionFileFactory;
public:
    /* methods to be implemented by sub-classes */
    virtual ~RegionFile() { };  
    virtual ErrorCode unlink() = 0;
    virtual ErrorCode close() = 0;
    virtual ErrorCode truncate(loff_t length) = 0;
    virtual ErrorCode size(loff_t* length) = 0;
    virtual ErrorCode map(void* addr_hint, size_t length, int prot, int flags, loff_t offset, void** mapped_addr) = 0;
    virtual ErrorCode unmap(void* addr, size_t length) = 0;
    virtual ErrorCode getxattr(const char *name, void *value, size_t size) = 0;
    virtual ErrorCode setxattr(const char *name, const void *value, size_t size, int flags) = 0;
  
    virtual size_t booksize() = 0;

public:
    /* these methods are intended for use as callbacks by the RegionFileFactory */
    virtual ErrorCode create(mode_t mode) = 0;
    virtual ErrorCode open(int flags, mode_t mode) = 0;
    virtual ErrorCode open(int flags) = 0;

public:
    /* methods implemented by this class or specialized by subclasses */
    virtual ErrorCode set_interleave_group(loff_t offset, loff_t length, const std::vector<InterleaveGroup>& vig);
    virtual ErrorCode interleave_group(loff_t offset, loff_t length, std::vector<InterleaveGroup>* vig);
};


} // namespace alps

#endif // _ALPS_PEGASUS_REGIONFILE_HH_
