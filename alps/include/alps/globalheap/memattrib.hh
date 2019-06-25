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

#ifndef _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HH_
#define _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HH_

#include <iostream>

#include "../pegasus/interleave_group.hh"

namespace alps {

class MemAttrib {
public:
    MemAttrib()
        : ig(0)
    { }

    MemAttrib(InterleaveGroup _ig)
        : ig(_ig)
    { } 

    bool operator==(const MemAttrib& other) const
    {
        return ig == other.ig;
    }

    bool operator!=(const MemAttrib& other) const
    {
        return ig != other.ig;
    }

    bool operator<(const MemAttrib& other) const 
    {
        return ig < other.ig;
    }

    void stream_to(std::ostream& os) const 
    {
        os << "ig: " << static_cast<size_t>(ig);
    } 

    InterleaveGroup ig;
};

static inline std::ostream& operator<<(std::ostream& os, const MemAttrib& memattrib)
{
    memattrib.stream_to(os);
    return os;
}

} // namespace alps

#endif // _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HH_
