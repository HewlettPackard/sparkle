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

#ifndef _ALPS_PEGASUS_HELPER_FUNCTIONS_HH_
#define _ALPS_PEGASUS_HELPER_FUNCTIONS_HH_

#include <vector>

#include <boost/filesystem.hpp>

#include "alps/common/error_code.hh"

namespace alps {

/**
 * @brief A helper function for truncating multiple region files in parallel.
 */
ErrorCode truncate_region_files(const std::vector< std::tuple<boost::filesystem::path, size_t> >& files, bool parallel);

/**
 * @brief A helper function for creating multiple region files in parallel 
 * with each file truncated to length \a size.
 */
ErrorCode create_region_files(const std::vector<boost::filesystem::path> paths, size_t size, mode_t mode, bool parallel, bool interleave, size_t interleave_size);

/**
 * @brief A helper function for creating a region file truncated to length \a size.
 *
 * @detail Parameter \a size must be multiple of \a interleave_size, and 
 * \a interleave_size must be multiple of region-file booksize.
 */
ErrorCode create_region_file(const boost::filesystem::path path, size_t size, mode_t mode, bool interleave, size_t interleave_size);


} // namespace alps

#endif // _ALPS_PEGASUS_HELPER_FUNCTIONS_HH_
