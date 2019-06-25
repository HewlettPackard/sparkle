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

#include "common/os.hh"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <numa.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/magic.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>

#include <chrono>
#include <fstream>
#include <regex>
#include <sstream>

#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include "alps/common/debug.hh"

namespace alps {

std::string os_flags_to_str(int flags, const std::string delimiter)
{
    bool insert_delimiter = false;
    std::stringstream ss;
    
    if (flags & O_CREAT) {
        ss << "O_CREAT";
        insert_delimiter = true;
    }
    if (flags & O_EXCL) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_EXCL";
        insert_delimiter = true;
    }
    if (flags & O_TRUNC) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_TRUNC";
        insert_delimiter = true;
    }
    if (flags & O_DIRECT) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_DIRECT";
        insert_delimiter = true;
    }
    if (flags & O_APPEND) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_APPEND";
        insert_delimiter = true;
    }
    if (flags & O_RDWR) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_RDWR";
        insert_delimiter = true;
    }
    if (flags & O_RDONLY) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_RDONLY";
        insert_delimiter = true;
    }
    if (flags & O_WRONLY) {
        if (insert_delimiter) ss << delimiter;
        ss << "O_WRONLY";
        insert_delimiter = true;
    }


    return ss.str();
}

std::string os_fstype(const char* pathname)
{
    struct mntent  mntbuf;
    struct mntent* mnt;
    char   buf[4096];

    mnt = os_mountpoint(pathname, &mntbuf, buf, 4096);

    if (mnt) {    
        return std::string(mnt->mnt_type);
    }
    return std::string("unknown");
}

struct mntent *os_mountpoint(const char *filename, struct mntent *mnt, char *buf, size_t buflen)
{
    struct stat s;
    FILE *      fp;
    dev_t       dev;


    if (stat(filename, &s) != 0) {
        if (errno == ENOENT) {
            // file does not exist so try parent as the mountpoint should be
            // the same (assuming filename is an absolute path...)
            boost::filesystem::path pathname(filename);
            std::string dirname = pathname.parent_path().string();
            if (stat(dirname.c_str(), &s) != 0) {
                return NULL;
            }
        } else {
            return NULL;
        }
    }

    dev = s.st_dev;

    if ((fp = setmntent("/proc/mounts", "r")) == NULL) {
        return NULL;
    }

    while (getmntent_r(fp, mnt, buf, buflen)) {
        if (stat(mnt->mnt_dir, &s) != 0) {
            continue;
        }
        if (s.st_dev == dev) {
            endmntent(fp);
            return mnt;
        }
    }

    endmntent(fp);

    // Should never reach here.
    errno = EINVAL;
    return NULL;
}

ErrorCode os_filesize(const char* pathname, loff_t* size)
{
    int         ret;
    struct stat buf;

    if ((ret = stat(pathname, &buf)) < 0) {
        return kErrorCodeFsStatFailed;
    }

    *size = buf.st_size;
    return kErrorCodeOk;
}

ErrorCode os_filesize(int fd, loff_t* size)
{
    int         ret;
    struct stat buf;

    if ((ret = fstat(fd, &buf)) < 0) {
        return kErrorCodeFsStatFailed;
    }

    *size = buf.st_size;
    return kErrorCodeOk;
}

int os_write_zeros(int descriptor, off_t offset, size_t amount) 
{
    int ret;
    char buffer[1024*1024];
    memset(&buffer, 0, sizeof(buffer));

    if (offset > 0) {
        if ((ret = lseek(descriptor, offset, SEEK_SET) < 0)) {
            return ret;
        }
    }
    while (amount > 0) {
        ssize_t written = write(descriptor, buffer, 
                                amount>sizeof(buffer) ? sizeof(buffer) : amount);
        if (written < 0) {
            return -1;
        }
        amount -= written;
    }

    return 0;
}

int os_set_membind(int node)
{
    struct bitmask* nodemask = numa_allocate_nodemask();
    numa_bitmask_clearall(nodemask);
    numa_bitmask_setbit(nodemask, node);
    numa_set_membind(nodemask);
    numa_free_nodemask(nodemask);
    return 0;
}

std::string os_exec(const char* cmd) 
{
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return "ERROR";
    }
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

std::string fs_unique_name(uint64_t differentiator) {
  return fs_unique_name("%%%%-%%%%-%%%%-%%%%", differentiator);
}

std::string fs_unique_name(const std::string& model, uint64_t differentiator) {
  const char* kHexChars = "0123456789abcdef";
  uint64_t seed64 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  seed64 += ::getpid();  // further use process ID to randomize. may help.
  seed64 ^= differentiator;
  uint32_t seed32 = (seed64 >> 32) ^ seed64;
  std::string s(model);
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '%') {                 // digit request
      seed32 = ::rand_r(&seed32);
      s[i] = kHexChars[seed32 & 0xf];  // convert to hex digit and replace
    }
  }
  return s;
}

bool fs_fsync(const boost::filesystem::path& path, bool sync_parent_directory) {
  int sync_ret;
  if (!is_directory(path)) {
    int descriptor = ::open(path.c_str(), O_RDONLY);
    if (descriptor < 0) {
      return false;
    }
    sync_ret = ::fsync(descriptor);
    ::close(descriptor);
  } else {
    DIR *dir = ::opendir(path.c_str());
    if (!dir) {
      return false;
    }
    int descriptor = ::dirfd(dir);
    if (descriptor < 0) {
      return false;
    }

    sync_ret = ::fsync(descriptor);
    ::closedir(dir);
  }

  if (sync_ret != 0) {
    return false;
  }

  if (sync_parent_directory) {
    return fs_fsync(path.parent_path(), false);
  } else {
    return true;
  }
}

bool fs_durable_atomic_rename(const boost::filesystem::path& old_path, const boost::filesystem::path& new_path) {
  if (!fs_fsync(old_path, false)) {
    return false;
  }
  if (!fs_atomic_rename(old_path, new_path)) {
    return false;
  }
  return fs_fsync(new_path.parent_path(), false);
}

bool fs_atomic_rename(const boost::filesystem::path& old_path, const boost::filesystem::path& new_path) {
  return ::rename(old_path.c_str(), new_path.c_str()) == 0;
}

bool fs_create_directories(const boost::filesystem::path& p, bool sync) {
  if (boost::filesystem::exists(p)) {
    return true;
  }
  if (fs_create_directory(p, sync)) {
    return true;
  }
  // if failed, create parent then try again
  boost::filesystem::path parent = p.parent_path();
  if (parent.empty()) {
    return false;
  }
  if (!fs_create_directories(parent, sync) && !boost::filesystem::exists(parent)) {
    return false;
  }
  // now ancestors exist.
  return fs_create_directory(p, sync);
}

bool fs_create_directory(const boost::filesystem::path& p, bool sync) {
  int ret = ::mkdir(p.c_str(), S_IRWXU);
  if (ret != 0) {
    return false;
  }
  if (sync) {
    return fs_fsync(p, true);
  } else {
    return true;
  }
}


void fs_list_entries(const boost::filesystem::path& path, std::list<boost::filesystem::path>* file_list) 
{
    if (boost::filesystem::is_directory(path)) {
        for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path), {})) {
            file_list->push_back(entry);
        }
    }
}


void fs_remove_matched(const boost::filesystem::path& dir, const std::regex regex, bool wait_for_reclamation, int timeout_sec)
{
    struct statfs statfs_buf;
    assert(statfs(dir.string().c_str(), &statfs_buf) == 0);

    size_t block_size = statfs_buf.f_frsize;
    size_t old_free_blocks = statfs_buf.f_bfree;
    size_t deleted_blocks = 0;  // this is a low bound

    std::list<boost::filesystem::path> lst;
    fs_list_entries(dir, &lst);
    for (auto& entry: lst) {
        if (std::regex_match(boost::filesystem::basename(entry), regex)) {
            struct stat stat_buf;
            //The correct way is to wait for allocated blocks to be reclaimed, but 
            //LFS does not correctly report the number of allocated blocks.
            //assert(stat(entry.string().c_str(), &stat_buf) == 0);
            //deleted_blocks += (stat_buf.st_blocks * 512) / block_size;
            deleted_blocks += boost::filesystem::file_size(entry) / block_size;
            boost::filesystem::remove(entry);
        }
    }
    
    if (wait_for_reclamation) {
        // now wait for free space to be reclaimed
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        for (;;) {
            assert(statfs(dir.string().c_str(), &statfs_buf) == 0);
            size_t free_blocks = statfs_buf.f_bfree;
            if (free_blocks >= old_free_blocks + deleted_blocks) {
                break;
            }
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = end-start;
            if (elapsed.count() > timeout_sec) {
                break;
            }   
            sleep(1);
        }
    }
}


ProcessMap::ProcessMap()
    : ProcessMap(getpid())
{ 
}

ProcessMap::ProcessMap(pid_t pid)
    : pid_(pid)
{
}

std::pair<size_t, size_t> ProcessMap::range(const std::string name)
{
    std::stringstream ss;
    ss << "/proc/" << pid_ << "/maps";

    std::ifstream infile(ss.str());
    std::string line;
    std::string tmp;
    std::string col_range;
    std::string col_name;

    while (getline (infile, line)) {
        std::istringstream iss(line);
        iss >> col_range;
        iss >> tmp >> tmp >> tmp >> tmp;
        iss >> col_name;
        if (col_name == name) {
            infile.close();
            size_t sep = col_range.find('-');
            return std::pair<size_t, size_t>(std::stoll(col_range.substr(0, sep), 0, 16),
                                             std::stoll(col_range.substr(sep+1, col_range.length() - sep), 0, 16));
        }
    }
    infile.close();

    return std::pair<size_t, size_t>(0, 0);
}

} // namespace alps
