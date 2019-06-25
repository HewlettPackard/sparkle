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

#ifndef _ALPS_PEGASUS_PEGASUS_HH_
#define _ALPS_PEGASUS_PEGASUS_HH_

#ifdef HAVE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#endif

#include "alps/common/error_code.hh"
#include "alps/common/error_stack.hh"

namespace alps {

class AddressSpace;
class PegasusOptions;
class RegionFileFactory;
class RegionFile;
class TopologyFactory;
class Topology;

/**
 * \brief Pegasus environment
 *
 * \details A static singleton class that encapsulates a Pegasus environment
 */
class Pegasus {
public:
    /**
     * \brief Loads configuration options from configuration file into 
     * object pointed by \a pegasus_options.
     *
     * \details 
     * Caller is responsible to pass a valid PegasusOptions object
     *
     * Load configuration options in the following order:
     * - If \a use_system_wide_conf is then load configuration options from
     *   the system wide configuration file (/etc/default/alps.yml), and then
     * - If path \a use_environ is set then load configuration options from 
     *   the file declared by environment variable ALPS_CONF, and then
     * - Load configuration options from \a config_file.
     */
    static ErrorStack load_options(const char* config_file, bool use_system_wide_conf, bool use_environ, PegasusOptions* pegasus_options);

    /**
     * \brief Initialize Pegasus singleton class by loading configuration 
     * options from a file.
     *
     * \details 
     * A shorthand for load_options() followed by init(PegasusOptions*).
     *
     * Load configurations options in the order described under load_options.
     */
    static ErrorStack init(const char* config_file, bool use_system_wide_conf, bool use_environ);

    /**
     * \brief Initialize Pegasus singleton class by loading configuration 
     * options from options object \a pegasus_options.
     */
    static ErrorStack init(const PegasusOptions& pegasus_options);

    static ErrorCode create_region_file(const char* pathname, mode_t mode, RegionFile** region_file);
    static ErrorCode open_region_file(const char* pathname, int flags, mode_t mode, RegionFile** region_file);
    static ErrorCode open_region_file(const char** pathnames, int npathnames, int flags, mode_t mode, RegionFile** region_file);
    static ErrorCode open_region_file(const char* pathname, int flags, RegionFile** region_file);
    static ErrorCode open_region_file(const char** pathnames, int npathnames, int flags, RegionFile** region_file);

#ifdef HAVE_BOOST_FILESYSTEM
    static ErrorCode create_region_file(const boost::filesystem::path& pathname, mode_t mode, RegionFile** region_file);
    static ErrorCode open_region_file(const boost::filesystem::path& pathname, int flags, mode_t mode, RegionFile** region_file);
    static ErrorCode open_region_file(const std::vector<boost::filesystem::path>& pathnames, int flags, mode_t mode, RegionFile** region_file);
    static ErrorCode open_region_file(const boost::filesystem::path& pathname, int flags, RegionFile** region_file);
    static ErrorCode open_region_file(const std::vector<boost::filesystem::path>& pathnames, int flags, RegionFile** region_file);
#endif

    static AddressSpace* address_space() 
    {
        return address_space_;
    }

    static RegionFileFactory* region_file_factory()
    {
        return region_file_factory_;
    }

    static TopologyFactory* topology_factory()
    {
        return topology_factory_;
    }
private:
    static bool               initialized_;
protected:
    static AddressSpace*      address_space_;
    static RegionFileFactory* region_file_factory_;
    static TopologyFactory*   topology_factory_;
};


} // namespace alps

#endif // _ALPS_PEGASUS_PEGASUS_HH_
