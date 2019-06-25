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

#include "alps/pegasus/pegasus.hh"

#include <vector>
#include <boost/filesystem.hpp>

#include "alps/common/error_code.hh"
#include "alps/common/error_stack.hh"
#include "alps/pegasus/address_space.hh"
#include "alps/pegasus/pegasus_options.hh"

#include "common/debug.hh"
#include "pegasus/lfs_region_file.hh"
#include "pegasus/lfs_topology.hh"
#include "pegasus/region_file_factory.hh"
#include "pegasus/region_file.hh"
#include "pegasus/tmpfs_region_file.hh"
#include "pegasus/tmpfs_topology.hh"
#include "pegasus/topology_factory.hh"

namespace alps {

AddressSpace* Pegasus::address_space_;
RegionFileFactory* Pegasus::region_file_factory_;
TopologyFactory* Pegasus::topology_factory_;

bool Pegasus::initialized_ = false;

ErrorStack Pegasus::load_options(const char* config_file, bool use_system_wide_conf, bool use_environ_conf, PegasusOptions* pegasus_options) 
{
    const char* system_wide_conf_yaml[] = { "/etc/default/alps.yaml",
                                            "/etc/default/alps.yml", 0 };
    const char* environ_conf_varname = "ALPS_CONF";

    // Allow missing parameters as this initialization guarantees default 
    // values for optional/missing YAML elements when loading from files.
    *pegasus_options = PegasusOptions();  

    if (!pegasus_options) {
        return ERROR_STACK_MSG(kErrorCodeInvalidParameter, "Passed PegasusOptions object is NULL");
    }

    if (use_system_wide_conf) {
        for (int i=0; system_wide_conf_yaml[i] != 0; i++) {
            if (boost::filesystem::exists(system_wide_conf_yaml[i])) {
                CHECK_ERROR(pegasus_options->load_from_file(system_wide_conf_yaml[i], true));
                break;
            } 
        }
    }

    if (use_environ_conf) {
        const char* env_config_file = getenv(environ_conf_varname);
        if (env_config_file) {
            CHECK_ERROR(pegasus_options->load_from_file(env_config_file, true));
        }
    }

    if (config_file) {
        CHECK_ERROR(pegasus_options->load_from_file(config_file, true));
    }

    return kRetOk;
}


ErrorStack Pegasus::init(const char* config_file, bool use_system_wide_conf, bool use_environ) 
{
    PegasusOptions pegasus_options;

    CHECK_ERROR(load_options(config_file, use_system_wide_conf, use_environ, &pegasus_options));
    return Pegasus::init(pegasus_options);
}


ErrorStack Pegasus::init(const PegasusOptions& pegasus_options) 
{
    if (initialized_) {
        return kRetOk;
    }
    init_log(pegasus_options.debug_options);

    address_space_ = new AddressSpace(pegasus_options.address_space_options);
    region_file_factory_ = new RegionFileFactory(pegasus_options);
    topology_factory_ = new TopologyFactory(pegasus_options);
    CHECK_OUTOFMEMORY(address_space_); 
    CHECK_OUTOFMEMORY(region_file_factory_);
    CHECK_OUTOFMEMORY(topology_factory_);
    WRAP_ERROR_CODE(address_space_->init());

    region_file_factory_->register_fstype("tmpfs", TmpfsRegionFile::construct);
    region_file_factory_->register_fstype("tmfs", LfsRegionFile::construct);
    region_file_factory_->register_fstype("any", LfsRegionFile::construct);

    topology_factory_->register_fstype("tmpfs", TmpfsTopology::construct);
    topology_factory_->register_fstype("tmfs", LfsTopology::construct);
    topology_factory_->register_fstype("any", LfsTopology::construct);

    initialized_ = true;  

    return kRetOk;
}


std::vector<boost::filesystem::path> path_vector(const char** pathnames, int npathnames)
{
    std::vector<boost::filesystem::path> pl;
    for (int i=0; i<npathnames; i++) {
        boost::filesystem::path p{pathnames[i]};
        pl.push_back(p);
    }
    return pl;
}

ErrorCode Pegasus::create_region_file(const char* pathname, mode_t mode, RegionFile** region_file)
{
    boost::filesystem::path p{pathname};
    return region_file_factory_->create(p, mode, region_file);
}

ErrorCode Pegasus::open_region_file(const char* pathname, int flags, mode_t mode, RegionFile** region_file)
{
    return region_file_factory_->open(pathname, flags, mode, region_file);
}

ErrorCode Pegasus::open_region_file(const char** pathnames, int npathnames, int flags, mode_t mode, RegionFile** region_file)
{
    return region_file_factory_->open(path_vector(pathnames, npathnames), flags, mode, region_file);
}

ErrorCode Pegasus::open_region_file(const char* pathname, int flags, RegionFile** region_file)
{
    return region_file_factory_->open(pathname, flags, region_file);
}

ErrorCode Pegasus::open_region_file(const char** pathnames, int npathnames, int flags, RegionFile** region_file)
{
    return region_file_factory_->open(path_vector(pathnames, npathnames), flags, region_file);
}


#ifdef HAVE_BOOST_FILESYSTEM

ErrorCode Pegasus::create_region_file(const boost::filesystem::path& pathname, mode_t mode, RegionFile** region_file)
{
    return region_file_factory_->create(pathname, mode, region_file);
}

ErrorCode Pegasus::open_region_file(const boost::filesystem::path& pathname, int flags, mode_t mode, RegionFile** region_file)
{
    return region_file_factory_->open(pathname, flags, mode, region_file);
}

ErrorCode Pegasus::open_region_file(const std::vector<boost::filesystem::path>& pathnames, int flags, mode_t mode, RegionFile** region_file)
{
    return region_file_factory_->open(pathnames, flags, mode, region_file);
}

ErrorCode Pegasus::open_region_file(const boost::filesystem::path& pathname, int flags, RegionFile** region_file)
{
    return region_file_factory_->open(pathname, flags, region_file);
}

ErrorCode Pegasus::open_region_file(const std::vector<boost::filesystem::path>& pathnames, int flags, RegionFile** region_file)
{
    return region_file_factory_->open(pathnames, flags, region_file);
}

#endif

} // namespace alps
