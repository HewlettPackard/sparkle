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

#include <numa.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <iostream>

#include "alps/common/assorted_func.hh"

#include "common/os.hh"
#include "pegasus/lfs_topology.hh"

namespace alps {

Topology* LfsTopology::construct(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options) 
{
    return new LfsTopology(pathname, pegasus_options);
}


LfsTopology::LfsTopology(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options)
    : lfs_options_(pegasus_options.lfs_options),
      is_stale_(true),
      node_count_(pegasus_options.lfs_options.node_count)
{
    // initialize topology information
    update();
}


void LfsTopology::update()
{
    char* envvar;

    // Offset nodes by 1 as LFS numbers nodes by starting at 1 but 
    // RMB numbers them by starting at 0
    last_node_ = lfs_options_.node - 1;
    max_node_ = node_count_ - 1;
    is_stale_ = false;
}


InterleaveGroup LfsTopology::max_interleave_group()
{
    if (is_stale_) {
        update();
    }
    return max_node_;
}


unsigned int LfsTopology::node_running_on()
{
    if (is_stale_) {
        update();
    }
    return last_node_;
}


InterleaveGroup LfsTopology::nearest_ig()
{
    return node_running_on();
}


LfsTopology::~LfsTopology()
{
    // nothing to do
}

} // namespace alps
