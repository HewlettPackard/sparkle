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

#ifndef _ALPS_PEGASUS_TOPOLOGY_HH_
#define _ALPS_PEGASUS_TOPOLOGY_HH_

#include <boost/filesystem.hpp>

#include "alps/pegasus/interleave_group.hh"


namespace alps {

class Topology
{
public:
    /**
     * @brief Returns the highest node number available in the system
     */
    virtual InterleaveGroup max_interleave_group() = 0;

    /**
     * @brief Returns the nearest interleave group to the node the calling 
     * process is running on.
     */
    virtual InterleaveGroup nearest_ig() = 0;

    virtual ~Topology() = 0;
};


} // namespace alps


#endif // _ALPS_PEGASUS_TOPOLOGY_HH_
