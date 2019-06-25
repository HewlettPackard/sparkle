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

#ifndef _ALPS_PEGASUS_LFS_OPTIONS_HH_
#define _ALPS_PEGASUS_LFS_OPTIONS_HH_

#include "../common/externalizable.hh"

namespace alps {

struct LfsOptions: public Externalizable {
    unsigned int  kDefaultNode = 1;
    unsigned int  kDefaultNodeCount = 1;
    size_t kDefaultBookSizeBytes = 8*1024LLU*1024LLU*1024LLU;

    /**
     * Constructs option values with default values
     */
    LfsOptions() {
        node = kDefaultNode;
        node_count = kDefaultNodeCount;
        book_size_bytes = kDefaultBookSizeBytes;
    }

    unsigned int node; ///< the current node I am running on
    unsigned int node_count; ///< total number of nodes
    size_t book_size_bytes; ///< book size 

    EXTERNALIZABLE(LfsOptions)
};


} //namespace alps

#endif // _ALPS_PEGASUS_LFS_OPTIONS_HH_
