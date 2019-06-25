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

#ifndef _ALPS_PEGASUS_TMPFS_REGIONFILE_HH_
#define _ALPS_PEGASUS_TMPFS_REGIONFILE_HH_

#include <string>
#include <boost/filesystem.hpp>

#include "alps/pegasus/pegasus_options.hh"

#include "pegasus/region_file.hh"

/*
 * This is defined via sys/mman.h in L4TM; it is defined here for use
 * by generic_pregion clients
 */
#ifndef MAP_SYNCONEXIT
# define MAP_SYNCONEXIT	0x80000		/* Synchronize on process exit if still mapped.  */
#endif

namespace alps {

/**
 * @brief Represents a region file backed by TMPFS.
 */
class TmpfsRegionFile: public RegionFile {
public:
    static const char* kLockFileName;
    static const char* kXattrExtension;

    enum InterleavePolicy {
        kPreciseAllocate = 0,
        kRoundRobin  
    };

    static InterleavePolicy              interleave_policy_;
public:
    TmpfsRegionFile(boost::filesystem::path pathname, 
                    boost::filesystem::path xpathname,
                    const PegasusOptions& pegasus_options)
        : pathname_(pathname),
          xpathname_(xpathname),
          fd_(-1),
          booksize_(pegasus_options.tmpfs_options.book_size_bytes)
    { }

    ~TmpfsRegionFile() {};  

    static RegionFile* construct(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options);
    ErrorCode create(mode_t mode);
    ErrorCode open(int flags, mode_t mode);
    ErrorCode open(int flags);
    ErrorCode unlink();
    ErrorCode close();
    ErrorCode truncate(loff_t length);
    ErrorCode size(loff_t* length);
    ErrorCode map(void* addr_hint, size_t length, int prot, int flags, loff_t offset, void** mapped_addr);
    ErrorCode unmap(void* addr, size_t length);
    ErrorCode getxattr(const char *name, void *value, size_t size);
    ErrorCode setxattr(const char *name, const void *value, size_t size, int flags);
    size_t booksize();
 
private:
    int write_zeros(int descriptor, off_t offset, size_t amount);
    int save_xattr_map(const char* path, const std::map<std::string, std::string>& map);
    int load_xattr_map(const char* path, std::map<std::string, std::string>* map);
    void set_interleave_policy(InterleavePolicy policy);
    ErrorCode __alloc_physical_frames(int fd, loff_t offset, loff_t length, bool zero_books);
    ErrorCode alloc_physical_frames(int fd, loff_t offset, loff_t length, bool zero_books);
    ErrorCode ftruncate(int fd, off_t length, bool lock);
 
private:
    boost::filesystem::path pathname_;  // absolute pathname to the file
    boost::filesystem::path xpathname_; // file path to shadow metadata file
                                        // storing extended attributes (xattr)
    int         fd_;
    size_t      booksize_;
};

} // namespace alps

#endif // _ALPS_PEGASUS_TMPFS_REGIONFILE_HH_
