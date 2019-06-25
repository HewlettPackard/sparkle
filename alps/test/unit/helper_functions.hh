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

#ifndef _ALPS_TEST_HELPER_FUNCTIONS_HH_
#define _ALPS_TEST_HELPER_FUNCTIONS_HH_

#include <vector>
#include <boost/filesystem.hpp>
#include "alps/common/error_code.hh"
#include "alps/pegasus/interleave_group.hh"
#include "pegasus/helper_functions.hh"

namespace alps {

void create_region_file_paths(std::string path_prefix, 
                              int num_region_files, 
                              std::vector<boost::filesystem::path>* region_file_paths,
                              const char*** region_file_paths_c)
{
    for (int i=0; i < num_region_files; i++) {
        std::stringstream ss;
        ss << path_prefix << "_" << i;
        boost::filesystem::path region_file_name = boost::filesystem::path(ss.str());
        (*region_file_paths).push_back(region_file_name);
    }
    *region_file_paths_c = new const char*[num_region_files];
    for (unsigned int i=0; i<(*region_file_paths).size(); i++) {
        (*region_file_paths_c)[i] = (*region_file_paths)[i].c_str();
    }
}

} // namespace alps

#endif // _ALPS_TEST_HELPER_FUNCTIONS_HH_
