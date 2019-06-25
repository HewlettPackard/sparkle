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

#ifndef _ALPS_COMMON_OS_HH_
#define _ALPS_COMMON_OS_HH_

#include <stdio.h>
#include <mntent.h>
#include <sys/types.h>

#include <regex>
#include <string>
#include <utility>

#include "boost/filesystem/path.hpp"

#include "alps/common/error_code.hh"


// Useful OS utility functions

namespace alps {

std::string os_fstype(const char* pathname);
std::string os_flags_to_str(int flags, const std::string delimiter = "|");
struct mntent *os_mountpoint(const char *filename, struct mntent *mnt, char *buf, size_t buflen);
ErrorCode os_filesize(int fd, loff_t* size);
int os_write_zeros(int descriptor, off_t offset, size_t amount);
int os_set_membind(int node);
std::string os_exec(const char* cmd);


/**
 * Equivalent to unique_path("%%%%-%%%%-%%%%-%%%%").
 * @ingroup FILESYSTEM
 */
std::string fs_unique_name(uint64_t differentiator = 0);

/**
 * Returns a randomly generated file name with the given template.
 * @ingroup FILESYSTEM
 * @param[in] model file name template where % will be replaced with random hex numbers.
 * @param[in] differentiator optional parameter to further randomize this method.
 * @details
 * We use std::chrono::high_resolution_clock::now() to get a random seed.
 * \b However, even high_resolution_clock sometimes has low precision depending on environment.
 * When you are concerned with a conflict (eg running many concurrent testcases), also give
 * a differentiator.
 */
std::string fs_unique_name(const std::string& model, uint64_t differentiator = 0);

/**
 * @brief Makes the content and metadata of the file durable all the way up to devices.
 * @ingroup FILESYSTEM
 * @param[in] path path of the file to make durable
 * @param[in] sync_parent_directory (optional, default false) whether to also call fsync on
 * the parent directory to make sure the directory entry is written to device. This is required
 * when you create a new file, rename, etc.
 * @return whether the sync succeeded or not. If failed, check the errno global variable
 * (set by lower-level library).
 * @details
 * Surprisingly, there is no analogus method in boost::filesystem.
 * This method provides the fundamental building block of fault-tolerant systems; fsync.
 * We so far don't provide fdatasync (no metadata sync), but this should suffice.
 */
bool fs_fsync(const boost::filesystem::path& path, bool sync_parent_directory = false);

/**
 * @brief Renames the old file to the new file with the POSIX atomic-rename semantics.
 * @ingroup FILESYSTEM
 * @param[in] old_path path of the file to rename
 * @param[in] new_path path after rename
 * @return whether the rename succeeded
 * @pre exists(old_path)
 * @pre !exists(new_path) is \b NOT a pre-condition. See below.
 * @details
 * This is analogus to boost::filesystem::rename(), but we named this atomic_rename to clarify
 * that this implementation guarantees the POSIX atomic-rename semantics.
 * When new_path already exists, this method atomically swaps the file on filesystem with
 * the old_path file, appropriately deleting the old file.
 * This is an essential semantics to achieve safe and fault-tolerant file writes.
 * And, for that usecase, do NOT forget to also call fsync before/after rename, too.
 * Use durable_atomic_rename() to make sure.
 *
 * @see http://pubs.opengroup.org/onlinepubs/009695399/functions/rename.html
 * @see Eat My Data: How Everybody Gets File IO Wrong:
 * https://www.flamingspork.com/talks/2007/06/eat_my_data.odp
 */
bool fs_atomic_rename(const boost::filesystem::path& old_path, const boost::filesystem::path& new_path);

/**
 * @brief fsync() on source file before rename, then fsync() on the parent folder after rename.
 * @ingroup FILESYSTEM
 * @details
 * This method makes 2 fsync calls, one on old file \b before rename
 * and another on the parent directory \b after rename.
 *
 * Note that we don't need fsync on parent directory before rename assuming old_path and new_path
 * is in the same folder (if not, you have to call fsync yourself before calling this method).
 * Even if a crash happens right after rename, we still see the old content of new_path.
 *
 * Also, we don't need fsync on new_path after rename because POSIX rename doesn't change
 * the inode of renamed file. It's already there as soon as parent folder's fsync is done.
 *
 * Quite complex and expensive, but this is required to make it durable regardless of filesystems.
 * Fortunately, we have to call this method only once per epoch-advance.
 */
bool fs_durable_atomic_rename(const boost::filesystem::path& old_path, const boost::filesystem::path& new_path);



/**
 * Recursive mkdir (mkdirs).
 * @ingroup FILESYSTEM
 * @param[in] p path of the directory to create
 * @param[in] sync (optional, default false) wheter to call fsync() on the created directories
 * and their parents. This is required to make sure the new directory entries become durable.
 * @return whether the directory already exists or creation succeeded
 */
bool fs_create_directories(const boost::filesystem::path& p, bool sync = false);

/**
 * mkdir.
 * \ingroup FILESYSTEM
 * \param[in] p path of the directory to create
 * \param[in] sync (optional, default false) wheter to call fsync() on the created directory
 * and its parent. This is required to make sure the new directory entry becomes durable.
 * \return whether the directory already exists or creation whether succeeded
 */
bool fs_create_directory(const boost::filesystem::path& p, bool sync = false);


/**
 * \brief list files
 * \ingroup FILESYSTEM
 * \param[in] p path of the directory to list entries of
 * \param[out] el list of directory entries
 */
void fs_list_entries(const boost::filesystem::path& p, std::list<boost::filesystem::path>* el);


/**
 * \brief Remove files in directory \a path matching regular expression \a regex. 
 * \ingroup FILESYSTEM
 * \param[in] p path of the directory to remove entries of
 * \param[in] regex regular expression to match entries against 
 * \param[in] wait_for_reclamation wait for file system to reclaim space (only valid for LFS)
 */
void fs_remove_matched(const boost::filesystem::path& dir, const std::regex regex, bool wait_for_reclamation, int timeout_sec = 60);



class ProcessMap {
public:
    ProcessMap();
    ProcessMap(pid_t pid);

    std::pair<size_t, size_t> range(const std::string name);
private:

    pid_t pid_;

};


} // namespace alps

#endif // _ALPS_COMMON_OS_HH_
