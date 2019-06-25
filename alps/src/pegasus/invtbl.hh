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

#ifndef _ALPS_PEGASUS_INVTBL_HH_
#define _ALPS_PEGASUS_INVTBL_HH_

#include <atomic>
#include <utility>

#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/continuous_interval.hpp>
#include <boost/icl/right_open_interval.hpp>
#include <boost/icl/left_open_interval.hpp>
#include <boost/icl/closed_interval.hpp>
#include <boost/icl/open_interval.hpp>
#include <boost/icl/rational.hpp>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/icl/separate_interval_set.hpp>

#include "common/debug.hh"
#include "pegasus/vmarea.hh"
#include "pegasus/pegasthread.hh"

namespace alps {

class InvertedTable
{
public:
    typedef boost::icl::interval_map<uintptr_t, VmArea*>::iterator       iterator;
    typedef boost::icl::interval_map<uintptr_t, VmArea*>::const_iterator const_iterator;

public:
    InvertedTable()
        : version_(0)
    {

    }

    void insert_vmarea(VmArea* vma)
    {
        version_++;
        map_.insert(std::make_pair(boost::icl::discrete_interval<uintptr_t>(vma->vm_start(), vma->vm_end()-1, boost::icl::interval_bounds::closed()), vma));
    }

    void remove_vmarea(VmArea* vma)
    {
        version_++;
        const_iterator it = map_.find(vma->vm_start());
        if (it == map_.end()) {
            return;
        }
        map_.erase(boost::icl::discrete_interval<uint64_t>(vma->vm_start(), vma->vm_end() - 1, boost::icl::interval_bounds::closed()));
        assert(map_.find(vma->vm_start()) == map_.end());
    }

    VmArea* find_vmarea(uintptr_t addr)
    {
        if (pegas_thread.vmarea_ && 
            pegas_thread.vmarea_version_ == version_ &&
            addr >= pegas_thread.vmarea_->vm_start() && addr < pegas_thread.vmarea_->vm_end())
        {
            return pegas_thread.vmarea_;
        }

        const_iterator it = map_.find(addr);
        if (it == map_.end()) {
            return NULL;
        }
        pegas_thread.vmarea_ = it->second;
        pegas_thread.vmarea_version_ = version_;
        return it->second;
    }

private:
    boost::icl::interval_map<uintptr_t, VmArea*> map_; 
    std::atomic<uint64_t>                        version_; // versioning to detect when vmarea objects become stale
};

} // namespace alps

#endif // _ALPS_PEGASUS_INVTBL_HH_
