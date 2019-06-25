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

#ifndef _ALPS_GLOBALHEAP_GLOBALHEAP_INTERNAL_HH_
#define _ALPS_GLOBALHEAP_GLOBALHEAP_INTERNAL_HH_

#include <iostream>
#include <pthread.h>
#include <vector>
#include <boost/filesystem.hpp>

#include "alps/pegasus/pegasus.hh"
#include "alps/pegasus/relocatable_region.hh"
#include "alps/globalheap/memattrib.hh"

#include "pegasus/topology.hh"
#include "pegasus/topology_factory.hh"
#include "globalheap/layout.hh"
#include "globalheap/memattrib_alloc.hh"
#include "globalheap/memattrib_heap.hh"

namespace alps {

typedef size_t ZoneId;


/**
 * @brief Global Symmetric Heap public interface implementation
 */
class GlobalHeapInternal
{
public:
    typedef uint64_t InstanceId;

public:
    static int create(const char* path, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap);
    static int create(const char** paths, int npaths, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap);
    static int open(const char* path, GlobalHeapInternal** heap);
    static int open(const char** paths, int npaths, GlobalHeapInternal** heap);
    static int format(const char* path);
    static int format(const char** paths, int npaths);
    static int format_instance(const char* path, InstanceId instance_id);
    static int format_instance(const char** paths, int npaths, InstanceId instance_id);

    static int create(const boost::filesystem::path& pathname, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap);
    static int create(const std::vector<boost::filesystem::path>& pathnames, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap);
    static int open(const boost::filesystem::path& pathname, GlobalHeapInternal** heap);
    static int open(const std::vector<boost::filesystem::path>& pathnames, GlobalHeapInternal** heap);
    static int format(const boost::filesystem::path& pathname);
    static int format_instance(const boost::filesystem::path& pathname, InstanceId instance_id);
    static int format(const boost::filesystem::path& pathname, ZoneId zone_id);
    static int format(const boost::filesystem::path& pathname, std::vector<ZoneId> zone_ids);
    static int format(const std::vector<boost::filesystem::path>& pathnames);
    static int format_instance(const std::vector<boost::filesystem::path>& pathnames, InstanceId instance_id);
    static int format(const std::vector<boost::filesystem::path>& pathnames, ZoneId zone_id);
    static int format(const std::vector<boost::filesystem::path>& pathnames, std::vector<ZoneId> zone_ids);

    int init();
    InstanceId instance();
    template<typename RootT> RRegion::TPtr<RootT> root();
    template<typename RootT> void set_root(RRegion::TPtr<RootT> root);
    RRegion::TPtr<void> malloc(size_t size);
    RRegion::TPtr<void> malloc(size_t size, const MemAttrib& memattrib);
    void free(RRegion::TPtr<void> ptr);
    RRegion::TPtr<void> realloc(RRegion::TPtr<void> ptr, size_t size);
    int close();
    
    size_t size()
    {
        return size_;
    }

    RRegion::TPtr<nvHeap> nvheap() {
        return nvheap_;
    }   

    RRegion* region() {
        return region_;
    }

private:
    GlobalHeapInternal(const std::vector<boost::filesystem::path>& pathnames, RRegion* region, RRegion::TPtr<nvHeap> nvheap);
    static int format_zone(RRegion::TPtr<nvHeap> nvheap, ZoneId zone_id, bool format_heap_header);
    int teardown();
    MemAttribHeap* find_or_bind(const MemAttrib& memattrib);

protected:

    std::vector<boost::filesystem::path>  pathnames_;
    RRegion*                              region_;
    RRegion::TPtr<nvHeap>                 nvheap_;
    size_t                                size_;
    Generation                            generation_;
    MemAttribAllocators<MemAttribHeap>    memattrib_heaps_;
    Topology*                             topology_;
};


template<typename RootT>
RRegion::TPtr<RootT> GlobalHeapInternal::root()
{
    return nvheap_->root();
}

template<typename RootT>
void GlobalHeapInternal::set_root(RRegion::TPtr<RootT> root)
{
    RRegion::TPtr<void> void_root = static_cast<RRegion::TPtr<void>>(root);
    nvheap_->set_root(void_root);
}

} // namespace alps

#endif // _ALPS_GLOBALHEAP_GLOBALHEAP_INTERNAL_HH_
