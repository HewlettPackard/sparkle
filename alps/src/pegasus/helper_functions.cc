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

#include "pegasus/helper_functions.hh"

#include <thread>

#include "alps/pegasus/pegasus.hh"

#include "common/debug.hh"
#include "pegasus/region_file.hh"
#include "pegasus/topology.hh"
#include "pegasus/topology_factory.hh"

#include <sstream>

namespace alps {

static ErrorCode __truncate_region_file(boost::filesystem::path path, size_t size)
{
    RegionFile* rfile;

    CHECK_ERROR_CODE(Pegasus::open_region_file(path.c_str(), O_RDWR, &rfile));
    CHECK_ERROR_CODE(rfile->truncate(size));
    CHECK_ERROR_CODE(rfile->close());

    return kErrorCodeOk;
}

static void __truncate_region_file_thread(boost::filesystem::path path, size_t size)
{
    COERCE_ERROR_CODE(__truncate_region_file(path, size));
}

ErrorCode truncate_region_files(const std::vector< std::tuple<boost::filesystem::path, size_t> >& files, bool parallel)
{
    std::vector< std::tuple<boost::filesystem::path, size_t> >::const_iterator it;
    std::vector<std::thread> threads;
 
    for (it = files.begin(); it != files.end(); it++) {
        boost::filesystem::path rfile_path = std::get<0>(*it);
        size_t rfile_size = std::get<1>(*it);
        if (parallel) {
            std::thread t(__truncate_region_file_thread, rfile_path, rfile_size);
            threads.push_back(std::move(t));
        } else {
            __truncate_region_file(rfile_path, rfile_size);
        }
    }
    if (parallel) {
        for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); it++) {
            it->join();
        }
    }
    return kErrorCodeOk;
}

ErrorCode create_region_files(const std::vector<boost::filesystem::path> paths, size_t size, mode_t mode, bool parallel, bool interleave, size_t interleave_size)
{
    Topology* topology;
    InterleaveGroup ig;

    // Get topology information about the memory backing files paths[].
    // Use the first file (assuming all the files are backed by the same 
    // memory file system)
    CHECK_ERROR_CODE(Pegasus::topology_factory()->construct(paths[0], &topology));

    size_t num_avail_igs = topology->max_interleave_group() + 1;
    std::vector< std::tuple<boost::filesystem::path, size_t> > files;
    std::vector<boost::filesystem::path>::const_iterator it;
    for (ig = 0, it = paths.begin(); it != paths.end(); it++, ig = (ig+1) % num_avail_igs) {
        RegionFile* rfile;
        boost::filesystem::path path = *it;
        files.push_back(std::tuple<boost::filesystem::path, size_t>(path, size));
        CHECK_ERROR_CODE(Pegasus::create_region_file(path.c_str(), mode, &rfile));
        std::vector<InterleaveGroup> vig{ig};
        CHECK_ERROR_CODE(rfile->set_interleave_group(0, size, vig));
        CHECK_ERROR_CODE(rfile->close());
    }
    delete topology;
    return truncate_region_files(files, parallel);
}

ErrorCode create_region_file(const boost::filesystem::path path, size_t size, mode_t mode, bool interleave, size_t interleave_size)
{
    RegionFile* region_file;
    Topology* topology;

    CHECK_ERROR_CODE(Pegasus::topology_factory()->construct(path, &topology));
    CHECK_ERROR_CODE(Pegasus::create_region_file(path, S_IRUSR | S_IWUSR, &region_file));

    if (size % interleave_size != 0) {
        LOG(error) << "Parameter size must be multiple of interleave size.";
        return kErrorCodeInvalidParameter;
    }

    if (interleave_size % region_file->booksize() != 0) {
        LOG(error) << "Parameter interleave size must be multiple of region-file booksize.";
        return kErrorCodeInvalidParameter;
    }

    std::vector<InterleaveGroup> vig;
    for (size_t sz = 0; sz < size; ) {
        for (InterleaveGroup ig = 0; ig <= topology->max_interleave_group() && sz < size; ig++, sz+=interleave_size) {
            for (size_t i = 0; i < interleave_size; i+=region_file->booksize()) {
                vig.push_back(ig);
            }
        }
    }

    CHECK_ERROR_CODE(region_file->set_interleave_group(0, size, vig));
    CHECK_ERROR_CODE(region_file->truncate(size));
    CHECK_ERROR_CODE(region_file->close());
    delete topology;
    return kErrorCodeOk;
}

} // namespace alps
