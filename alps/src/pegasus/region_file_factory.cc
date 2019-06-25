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

#include "pegasus/region_file_factory.hh"

#include <stdio.h>
#include <libgen.h>
#include <sys/mman.h>
#include <string.h>
#include <iostream>
#include <boost/filesystem.hpp>

#include "alps/common/assert_nd.hh"
#include "alps/common/error_code.hh"

#include "common/os.hh"
#include "common/debug.hh"
#include "pegasus/region_file.hh"
#include "pegasus/multi_region_file.hh"

namespace alps {


RegionFileFactory::RegionFileFactory(const PegasusOptions& pegasus_options)
    : FileSystemTypeFactory<RegionFile>(pegasus_options)
{ }

ErrorCode RegionFileFactory::create(const boost::filesystem::path& pathname, mode_t mode, RegionFile** region_file)
{
    RegionFile* rfile;

    CHECK_ERROR_CODE(construct(pathname, &rfile));
    CHECK_ERROR_CODE(rfile->create(mode));
    *region_file = rfile;

    return kErrorCodeOk;
}

ErrorCode RegionFileFactory::open(const boost::filesystem::path& pathname, int flags, mode_t mode, RegionFile** region_file)
{
    RegionFile* rfile;

    CHECK_ERROR_CODE(construct(pathname, &rfile));
    CHECK_ERROR_CODE(rfile->open(flags, mode));
    *region_file = rfile;

    return kErrorCodeOk;
}

ErrorCode RegionFileFactory::open(const boost::filesystem::path& pathname, int flags, RegionFile** region_file)
{
    RegionFile* rfile;

    CHECK_ERROR_CODE(construct(pathname, &rfile));
    CHECK_ERROR_CODE(rfile->open(flags));
    *region_file = rfile;

    return kErrorCodeOk;
}

ErrorCode RegionFileFactory::region_files(const std::vector<boost::filesystem::path>& pathnames, std::vector<RegionFile*>* rfiles)
{
    RegionFile* rfile;
    std::vector<boost::filesystem::path>::const_iterator it;
    
    for (it = pathnames.begin(); it != pathnames.end(); it++) {
        boost::filesystem::path pathname = *it;
        CHECK_ERROR_CODE(construct(pathname, &rfile));
        (*rfiles).push_back(rfile);
    }

    return kErrorCodeOk;
}

ErrorCode RegionFileFactory::multi_region_file(const std::vector<boost::filesystem::path>& pathnames, MultiRegionFile** mrfile)
{
    std::vector<RegionFile*> rfiles;
    CHECK_ERROR_CODE(region_files(pathnames, &rfiles));
    *mrfile = new MultiRegionFile(rfiles);
    return kErrorCodeOk;
}

ErrorCode RegionFileFactory::open(const std::vector<boost::filesystem::path>& pathnames, int flags, mode_t mode, RegionFile** region_file)
{
    MultiRegionFile* mrfile;
    CHECK_ERROR_CODE(multi_region_file(pathnames, &mrfile));
    CHECK_ERROR_CODE(mrfile->open(flags, mode));
    *region_file = mrfile;

    return kErrorCodeOk;
}

ErrorCode RegionFileFactory::open(const std::vector<boost::filesystem::path>& pathnames, int flags, RegionFile** region_file)
{
    MultiRegionFile* mrfile;
    CHECK_ERROR_CODE(multi_region_file(pathnames, &mrfile));
    CHECK_ERROR_CODE(mrfile->open(flags));
    *region_file = mrfile;

    return kErrorCodeOk;
}

} // namespace alps
