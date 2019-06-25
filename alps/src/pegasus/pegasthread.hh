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

#ifndef _ALPS_PEGASUS_THREAD_HH_
#define _ALPS_PEGASUS_THREAD_HH_

#include <stdint.h>

namespace alps {

//forward declaration
class Region;
class VmArea;

class PegasThread {
public:
    PegasThread()
        : vmarea_version_(0),
          vmarea_(0x0)
    { }

    void set_active_pregion(Region* region)
    {
        region_ = region;
    }

public:

    // pointer to the currently active persistent region used by a
    // persistent/transient pointer during translation
    Region* region_;

    uint64_t vmarea_version_;
    VmArea*  vmarea_;
};

// thread local storage declaration
//extern __thread PegasThread pegas_thread;
extern thread_local PegasThread pegas_thread;

} // namespace alps

#endif // _ALPS_PEGASUS_THREAD_HH_
