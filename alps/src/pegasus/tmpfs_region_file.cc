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

/*
 * Implementation of a pseudo librarian file system (LFS) over /dev/shm 
 * for a generic Linux PC
 * 
 * This implementation is intended for testing code on a generic
 * Linux PC without any special hardware, such as a researcher's
 * laptop or PC. It supports multiple nodes without real persistence. 
 * It works by creating persistent regions in /dev/shm/regions 
 * (/dev/shm is a all-in-RAM filesystem). These files go away when the 
 * PC is rebooted. Allocation granularity is configurable (with default 
 * value of 8 GB) and pathname can be pretty much anything that can 
 * follow /dev/shm/lfs/.
 *
 * If you attempt to create a file whose name contains a slash,
 * an attempt will be made to automatically create the relevant
 * directories first via mkdir -p.  For example, a "mkdir -p
 * /dev/shm/lfs/foo/bar" will be preformed before attempting to
 * create "foo/bar/baz".
 * 
 * Directories can be created and files renamed out of band
 * using the usual Linux command line tools (e.g., mv
 * /dev/shm/lfs-mock/foo /dev/shm/lfs-mock/bar or mkdir /dev/shm/lfs-mock/my_folder).
 * The file space must be initialized before use via the associated 
 * shell script reset_persistent_regions. All persistent regions can 
 * be erased by rerunning this shell script out of band.
 * 
 * Concurrency is handled via a single global lock, implemented
 * via process-local pthread mutexes (for controlling access among
 * local threads) and a lock file locked using flock (for controlling
 * access among processes).
 */

#include "pegasus/tmpfs_region_file.hh"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <numa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp> 
#include <boost/archive/binary_iarchive.hpp> 


#include "alps/common/assert_nd.hh"

#include "common/debug.hh"
#include "common/os.hh"


namespace alps {

const char*                       TmpfsRegionFile::kLockFileName = "/dev/shm/@@lockfile@@";
const char*                       TmpfsRegionFile::kXattrExtension = ".xattr";
TmpfsRegionFile::InterleavePolicy TmpfsRegionFile::interleave_policy_ = TmpfsRegionFile::kPreciseAllocate;

static pthread_mutex_t         nvram__mutex = PTHREAD_MUTEX_INITIALIZER;
static int                     tmpfs_lock_descriptor;

static void tmpfs_acquire_global() 
{
    pthread_mutex_lock(&nvram__mutex);

    tmpfs_lock_descriptor = ::open(TmpfsRegionFile::kLockFileName, O_RDWR);
    if (tmpfs_lock_descriptor < 0) {
        fprintf(stderr, "unable to open tmpfs lock file (did you forget to run reset_persistent_regions?): %s\n", 
                strerror(errno));
        pthread_mutex_unlock(&nvram__mutex);
        exit(99);
    }

    int result;
    do {
        result = ::flock(tmpfs_lock_descriptor, LOCK_EX);
    } while (result == EINTR);
    if (result != 0) {
        fprintf(stderr, "unable to flock lock file (shouldn't happen): %s\n",
                strerror(errno));
        ::close(tmpfs_lock_descriptor);
        pthread_mutex_unlock(&nvram__mutex);
        exit(99);
    }
}

static void tmpfs_release_global() 
{
  flock(tmpfs_lock_descriptor, LOCK_UN);
  close(tmpfs_lock_descriptor);
  pthread_mutex_unlock(&nvram__mutex);
}

RegionFile* TmpfsRegionFile::construct(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options) 
{
    boost::filesystem::path xpathname = boost::filesystem::path(pathname.string() + std::string(kXattrExtension));
    return new TmpfsRegionFile(pathname, xpathname, pegasus_options);
}

ErrorCode TmpfsRegionFile::open(int flags, mode_t mode)
{
    LOG(info) << "Open TMPFS region file path: " << pathname_ << " flags: " << os_flags_to_str(flags);

    tmpfs_acquire_global();

    fd_ = ::open(pathname_.c_str(), flags, mode);
    if (fd_ < 0) {
        tmpfs_release_global();
        return kErrorCodeFsFailedToOpen;
    }

    tmpfs_release_global();

    return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::open(int flags)
{
    LOG(info) << "Open TMPFS region file path: " << pathname_ << " flags: " << os_flags_to_str(flags);

    tmpfs_acquire_global();

    fd_ = ::open(pathname_.c_str(), flags);
    if (fd_ < 0) {
        tmpfs_release_global();
        return kErrorCodeFsFailedToOpen;
    }

    tmpfs_release_global();

    return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::create(mode_t mode)
{
    LOG(info) << "Create TMPFS region file path: " << pathname_;

    return open(O_CREAT|O_WRONLY|O_TRUNC, mode);
}

ErrorCode TmpfsRegionFile::unlink()
{
  struct stat buf;

  tmpfs_acquire_global();
  if (::stat(xpathname_.c_str(), &buf) == 0) {
    if (::unlink(xpathname_.c_str()) < 0) {
        tmpfs_release_global();
        return kErrorCodeFsUnlinkFailed;
    }
  }
  if (::unlink(pathname_.c_str()) < 0) {
    tmpfs_release_global();
    return kErrorCodeFsUnlinkFailed;
  }
  tmpfs_release_global();
  return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::close()
{
    tmpfs_acquire_global();

    LOG(info) << "Close TMPFS region file path: " << pathname_;

    if (fd_ > -1) {
        if (::close(fd_) < 0) {
            tmpfs_release_global();
            return kErrorCodeFsFailedToClose;
        }
    }

    tmpfs_release_global();

    return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::__alloc_physical_frames(int fd, loff_t offset, loff_t length, bool zero_books) 
{
    if (length < 0) {
        return kErrorCodeInvalidParameter;
    }
    /* round offset down to booksize multiple: */
    loff_t begin = (offset / booksize_) * booksize_;

    /* round offset+length up to next booksize multiple: */
    loff_t end = (offset+length+booksize_-1)/booksize_*booksize_;

    std::map<std::string, std::string> map;
    load_xattr_map(xpathname_.c_str(), &map);

    std::string interleave_request;
    switch (interleave_policy_) {
        // allocate based on the user's interleave request
        case kPreciseAllocate:
            if (map.find("user.LFS.InterleaveRequest") == map.end()) {
                interleave_request = "";
            } else {
                interleave_request = map["user.LFS.InterleaveRequest"];
            }
            break;

        // allocate based on the round-robin interleave policy
        // to this by constructing an interleave request that emulates a round-robin 
        // interleave policy across the sockets/nodes of the system
        case kRoundRobin:
            int max_n = numa_max_node();
            for (size_t i=0; i<end/booksize_; i++) {
                interleave_request.push_back(i % max_n);
            } 
            break;
    }
     
    // pad the interleave_request with zeros if not enough bytes for the 
    // new total number of books comprising the file
    loff_t new_num_books = end / booksize_;

    for (int i=interleave_request.size(); i<new_num_books; i++) {
        interleave_request.push_back(0x0);
    }

    int i;
    loff_t off;
    for (off = begin, i = begin/booksize_; 
         off < end; 
         off += booksize_, i++) 
    {
        int interleave_group = static_cast<int>(interleave_request[i]);
        os_set_membind(interleave_group); // the interleave group is the socket node
        if (zero_books) {
            if (os_write_zeros(fd, off, booksize_)) {
                return kErrorCodeGeneric;
            }
        } else {
#if _XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L
            if (posix_fallocate(fd, off, booksize_) < 0) {
                return kErrorCodeGeneric;
            }
#else
            if (os_write_zeros(fd, off, booksize_)) {
                return kErrorCodeGeneric;
            }
#endif
        }
    }

    // describe where each book in the file is allocated 
    map["user.LFS.Interleave"] = interleave_request;
    save_xattr_map(xpathname_.c_str(), map);

    return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::alloc_physical_frames(int fd, loff_t offset, loff_t length, bool zero_books)
{
#ifdef PARALLEL_ALLOC
    // we did not find this effective as tmpfs holds a file mutex when 
    // allocating space to a file and thus serializes allocations
    int N = 4;
    pid_t pid[32];
    for (int i=0; i<N; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            ErrorCode rc = __alloc_physical_frames(fd, offset+i*length/N, length/N, zero_books);
            int ret = static_cast<int>(rc);
            exit(ret);
        }
    }

    for (int i=0; i<N; i++) {
        if (pid[i] != 0) {
            int status;
            wait(&status);
            if (status != 0) {
                return static_cast<ErrorCode>(status);
            }
        }
    }
    return kErrorCodeOk;
#else
    pid_t pid = fork();
    if (pid == 0) {
        // child
        ErrorCode rc = __alloc_physical_frames(fd, offset, length, zero_books);
        int ret = static_cast<int>(rc);
        exit(ret);
    } else {
        // parent
        int status;
        wait(&status);
        if (status != 0) {
            return static_cast<ErrorCode>(status);
        }
        return kErrorCodeOk;
    }
#endif
}

ErrorCode TmpfsRegionFile::ftruncate(int fd, off_t length, bool lock)
{
    ErrorCode rc;
    loff_t    existing_length;

    if (length < 0) {
        return kErrorCodeFsNameTooLong;
    }
    /* round length up to next booksize multiple: */
    length = (length+booksize_-1)/booksize_*booksize_;

    if (lock) tmpfs_acquire_global();    

    if ((rc = os_filesize(fd, &existing_length)) != kErrorCodeOk) {
        if (lock) tmpfs_release_global();
        return rc;
    }

    loff_t delta = length - existing_length;
    if (delta > 0) {
        if ((rc = alloc_physical_frames(fd, existing_length, delta, false)) != kErrorCodeOk) {
            if (lock) tmpfs_release_global();    
            return rc;
        }
    } else if (delta < 0) {
        if (::ftruncate(fd, length) != 0) {
            if (lock) tmpfs_release_global();
            return kErrorCodeFsTruncateFailed;
        }
        // FIXME: do we need to truncate the extended attributes too?
    }

    if (lock) tmpfs_release_global();
    return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::truncate(loff_t length)
{
    int       fd;
    ErrorCode rc;

    LOG(info) << "Truncate TMPFS region file path: " << pathname_ << " size: " << length;

    if ((fd = ::open(pathname_.c_str(), O_RDWR)) < 0) {
        return kErrorCodeFsFailedToOpen;
    }
    rc = ftruncate(fd, length, false);
    ::close(fd);

    return rc;
}

int TmpfsRegionFile::save_xattr_map(const char* path, const std::map<std::string, std::string>& map)
{
    std::ofstream fo(path, std::ios::binary);
    if (fo.fail()) return -1;
    boost::archive::binary_oarchive oa(fo); 
    oa << map; 
    fo.close();
    return 0;
}

int TmpfsRegionFile::load_xattr_map(const char* path, std::map<std::string, std::string>* map)
{
    std::ifstream fi(path, std::ifstream::in);
    if (fi.fail()) return -1;
    boost::archive::binary_iarchive ia(fi); 
    ia >> *map; 
    fi.close();
    return 0;
}

ErrorCode TmpfsRegionFile::getxattr(const char *name, void *value, size_t size)
{
    // serialize/deserialize attribute map to .xattr file
    // a rather inefficient way to do this but works

    int ret;

    tmpfs_acquire_global();

    std::map<std::string, std::string> map;
    if ((ret = load_xattr_map(xpathname_.c_str(), &map)) < 0) {
        tmpfs_release_global();
        return kErrorCodeGeneric;
    }
    if (map.find(std::string(name)) == map.end()) {
        tmpfs_release_global();
        return kErrorCodeGeneric;
    }
    std::string vval = map[std::string(name)];
    size_t cp_size = size < vval.size() ? size : vval.size();
    memcpy(value, vval.c_str(), cp_size);

    tmpfs_release_global();

    return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::setxattr(const char *name, const void *value, size_t size, int flags)
{
  // serialize/deserialize attribute map to .xattr file
  // a rather inefficient way to do this but works
  
  tmpfs_acquire_global();

  std::map<std::string, std::string> map;
  load_xattr_map(xpathname_.c_str(), &map);

  std::string vval;
  vval.resize(size);
  std::copy(reinterpret_cast<const char*>(value), reinterpret_cast<const char*>(value)+size, vval.begin());
  map[std::string(name)] = vval;

  save_xattr_map(xpathname_.c_str(), map);

  tmpfs_release_global();

  return kErrorCodeOk;
}

ErrorCode TmpfsRegionFile::size(loff_t* length)
{
    return os_filesize(fd_, length);
}

ErrorCode TmpfsRegionFile::map(void* addr_hint, size_t length, int prot, int flags, loff_t offset, void** mapped_addr) 
{
    if (flags & ~(MAP_FIXED|MAP_SHARED|MAP_HUGETLB|MAP_SYNCONEXIT)) {
        return kErrorCodeMemoryMapFailed;
    }

    void* p = ::mmap(addr_hint, length, prot, flags|MAP_SHARED, fd_, offset);
    
    if (p == MAP_FAILED) {
        return kErrorCodeMemoryMapFailed;
    }
    LOG(trace) << "mmap addr_hint: " << addr_hint << ", length: " << length << ", fd: " << fd_ << ", offset: " << offset << ", ret: " << p;


    *mapped_addr = p;
    return kErrorCodeOk;

}

ErrorCode TmpfsRegionFile::unmap(void* addr, size_t length)
{
    if (::munmap(addr, length) < 0) {
        return kErrorCodeMemoryUnmapFailed;
    }


    return kErrorCodeOk;
}


size_t TmpfsRegionFile::booksize()
{
    return booksize_;
}

} // namespace alps
