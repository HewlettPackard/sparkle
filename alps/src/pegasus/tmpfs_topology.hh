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

#ifndef _ALPS_PEGASUS_TMPFS_TOPOLOGY_HH_
#define _ALPS_PEGASUS_TMPFS_TOPOLOGY_HH_

#include "alps/pegasus/pegasus_options.hh"

#include "pegasus/topology.hh"

namespace alps {

class TmpfsTopology: public Topology
{
public:
    static Topology* construct(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options);

public:
    TmpfsTopology(const boost::filesystem::path& path, const PegasusOptions& pegasus_options);
    InterleaveGroup max_interleave_group();
    InterleaveGroup nearest_ig();
    ~TmpfsTopology();

public:
    // API methods only meant for use by tests

    int run_on_node(int n);

private:
    void update();
    unsigned int node_running_on();

private:
    bool is_stale_; // whether topology has changed, and therefore cached values are stale
    unsigned int max_node_;
    unsigned int last_node_;
};


} // namespace alps


#endif // _ALPS_PEGASUS_TMPFS_TOPOLOGY_HH_
