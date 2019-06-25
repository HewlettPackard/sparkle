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

#ifndef _ALPS_PEGASUS_REGION_FILE_FACTORY_HH_
#define _ALPS_PEGASUS_REGION_FILE_FACTORY_HH_

#include <sys/types.h>
#include <sys/mman.h>
#include <vector>
#include <map>
#include <string>
#include <boost/filesystem.hpp>

#include "alps/common/error_code.hh"

#include "pegasus/fstype_factory.hh"

namespace alps {

//forward declaration
class RegionFile;
class MultiRegionFile;

/**
 * @brief Region file factory.
 */
class RegionFileFactory: public FileSystemTypeFactory<RegionFile> {
public:
    RegionFileFactory(const PegasusOptions& pegasus_options);

    ErrorCode create(const boost::filesystem::path& pathname, mode_t mode, RegionFile** region_file);
    ErrorCode open(const boost::filesystem::path& pathname, int flags, mode_t mode, RegionFile** region_file);
    ErrorCode open(const std::vector<boost::filesystem::path>& pathnames, int flags, mode_t mode, RegionFile** region_file);
    ErrorCode open(const boost::filesystem::path& pathname, int flags, RegionFile** region_file);
    ErrorCode open(const std::vector<boost::filesystem::path>& pathnames, int flags, RegionFile** region_file);

private:
    ErrorCode region_files(const std::vector<boost::filesystem::path>& pathnames, std::vector<RegionFile*>* rfiles);
    ErrorCode multi_region_file(const std::vector<boost::filesystem::path>& pathnames, MultiRegionFile** mrfile);
};

} // namespace alps

#endif // _ALPS_PEGASUS_REGION_FILE_FACTORY_HH_
