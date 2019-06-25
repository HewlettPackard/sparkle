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

#ifndef _ALPS_PEGASUS_LFS_TOPOLOGY_HH_
#define _ALPS_PEGASUS_LFS_TOPOLOGY_HH_

#include "alps/pegasus/pegasus_options.hh"
#include "alps/pegasus/lfs_options.hh"

#include "pegasus/topology.hh"

namespace alps {

class LfsTopology: public Topology {
public:
    static Topology* construct(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options);

public:
    LfsTopology(const boost::filesystem::path& path, const PegasusOptions& pegasus_options);
    InterleaveGroup max_interleave_group();
    InterleaveGroup nearest_ig();
    ~LfsTopology();

private:
    void update();
    unsigned int node_running_on();

private:
    LfsOptions   lfs_options_;
    bool         is_stale_;
    unsigned int node_count_;
    unsigned int max_node_;
    unsigned int last_node_;
};


class FRDNode 
{
public:
    FRDNode(unsigned int node_id)
    {
        unsigned int n = node_id - 1; // modulus math
        rack_ = (n / 80) + 1;
        encl_ = ((n % 80) / 10) + 1;
        node_ = (n % 10) + 1;
    }

    FRDNode(unsigned int rack, unsigned int encl, unsigned int node)
        : rack_(rack),
          encl_(encl),
          node_(node)
    {

    }

    unsigned int id() const
    {
        return (rack_ - 1)*80 + (encl_ - 1)*10 + node_;
    }

    bool operator==(const FRDNode& other)
    {
        return id() == other.id();
    }

    bool operator !=(const FRDNode& other)
    {
        return !(*this == other);
    }

    unsigned int operator-(const FRDNode& other)
    {
        if (encl_ != other.encl_) {
            return 5;
        }
        if (node_ != other.node_) {
            return 3;
        }
        return 1;
    }

private:
    unsigned int rack_;
    unsigned int encl_;
    unsigned int node_;
};


} // namespace alps


#endif // _ALPS_PEGASUS_LFS_TOPOLOGY_HH_
