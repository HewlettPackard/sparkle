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

#include "pegasus/lfs_region_file.hh"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <attr/xattr.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#ifdef __ARCH_FAM__
#ifdef __cplusplus
extern "C" {
#endif
#include <fam_atomic.h>
#ifdef __cplusplus
}
#endif
#endif // __ARCH_FAM__

#include "alps/common/assert_nd.hh"

#include "common/debug.hh"
#include "common/os.hh"


namespace alps {

RegionFile* LfsRegionFile::construct(const boost::filesystem::path& path, const PegasusOptions& pegasus_options) 
{
    return new LfsRegionFile(path, pegasus_options); 
}

ErrorCode LfsRegionFile::open(int flags, mode_t mode)
{
    LOG(info) << "Open LFS region file path: " << pathname_ << " flags: " << os_flags_to_str(flags);

    fd_ = ::open(pathname_.c_str(), flags, mode);
    if (fd_ < 0) {
        return kErrorCodeFsFailedToOpen;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::open(int flags)
{
    LOG(info) << "Open LFS region file path: " << pathname_ << " flags: " << os_flags_to_str(flags);

    fd_ = ::open(pathname_.c_str(), flags);
    if (fd_ < 0) {
        return kErrorCodeFsFailedToOpen;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::create(mode_t mode)
{
    LOG(info) << "Create LFS region file path: " << pathname_;

    if ((fd_ = ::creat(pathname_.c_str(), mode)) < 0) {
        return kErrorCodeFsFailedToOpen;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::unlink()
{
    if (::unlink(pathname_.c_str()) < 0) {
        return kErrorCodeFsUnlinkFailed;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::close()
{
    if (fd_ > -1) {
        if (::close(fd_) < 0) {
            return kErrorCodeFsFailedToClose;
        }
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::truncate(loff_t length)
{
    if (::ftruncate(fd_, length) < 0) {
        return kErrorCodeFsTruncateFailed;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::getxattr(const char *name, void *value, size_t size)
{
    LOG(info) << "name == " << name << ", size == " << size;
    if (::fgetxattr(fd_, name, value, size) < 0) {
        LOG(error) << strerror(errno);
        return kErrorCodeFsGetxattrFailed;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::setxattr(const char *name, const void *value, size_t size, int flags)
{
    LOG(info) << "name == " << name << ", size == " << size;
    if (::fsetxattr(fd_, name, value, size, flags) < 0) {
        LOG(error) << strerror(errno);
        return kErrorCodeFsSetxattrFailed;
    }
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::size(loff_t* length)
{
    return os_filesize(fd_, length);
}

ErrorCode LfsRegionFile::map(void* addr_hint, size_t length, int prot, int flags, loff_t offset, void** mapped_addr) 
{
    void* p = ::mmap(addr_hint, length, prot, flags|MAP_SHARED, fd_, offset);
    
    if (p == MAP_FAILED) {
        return kErrorCodeMemoryMapFailed;
    }
    LOG(trace) << "mmap addr_hint: " << addr_hint << ", length: " << length << ", fd: " << fd_ << ", offset: " << offset << ", ret: " << p;

#ifdef __ARCH_FAM__
    LOG(trace) << "registering region with the fam atomic library: "
               << "range_addr: " << p << "--" << (void*) (uintptr_t(p) + length);
    ASSERT_ND(0 == fam_atomic_register_region(p, length, fd_, offset));
#endif // __ARCH_FAM__

    *mapped_addr = p;
    return kErrorCodeOk;
}

ErrorCode LfsRegionFile::unmap(void* addr, size_t length)
{
    if (::munmap(addr, length) < 0) {
        return kErrorCodeMemoryUnmapFailed;
    }

#ifdef __ARCH_FAM__
    LOG(trace) << "unregistering region with the fam atomic library" ;
    fam_atomic_unregister_region(addr, length);
#endif // __ARCH_FAM__

    return kErrorCodeOk;
}

size_t LfsRegionFile::booksize()
{
    return booksize_;
}



} // namespace alps
