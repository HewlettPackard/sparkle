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

#include "globalheap/globalheap_internal.hh"

#include <sys/stat.h>
#include <sys/file.h>
#include <numa.h>
#include <thread>
#include <mutex>
#include <assert.h>

#include "alps/common/assert_nd.hh"
#include "alps/pegasus/pegasus.hh"
#include "alps/pegasus/relocatable_region.hh"

#include "common/debug.hh"
#include "pegasus/region_file.hh"
#include "globalheap/extentheap.hh"
#include "globalheap/helper.hh"
#include "globalheap/layout.hh"
#include "globalheap/lease.hh"
#include "globalheap/process_slab_heap.hh"
#include "globalheap/thread_slab_heap.hh"


namespace alps {

std::string paths_to_string(const std::vector<boost::filesystem::path>& pathnames)
{
    std::stringstream ss;
    for (std::vector<boost::filesystem::path>::const_iterator it = pathnames.begin();
         it != pathnames.end();
         it++) 
    {
        ss << it->string() << " ";
    }
    return ss.str();
}

GlobalHeapInternal::GlobalHeapInternal(const std::vector<boost::filesystem::path>& pathnames, RRegion* region, RRegion::TPtr<nvHeap> nvheap)
    : pathnames_(pathnames),
      region_(region),
      nvheap_(nvheap)
{ 
    size_ = nvheap->size();
}

int GlobalHeapInternal::create(const char* pathname, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap)
{
    boost::filesystem::path p{pathname};
    return GlobalHeapInternal::create(p, heap_size, metazone_size, heap);
}

int GlobalHeapInternal::create(const char** pathnames, int npathnames, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap)
{
    std::vector<boost::filesystem::path> paths;

    for (int i=0; i<npathnames; i++) {
        paths.push_back(std::string(pathnames[i]));
    }

    return GlobalHeapInternal::create(paths, heap_size, metazone_size, heap);
}

int GlobalHeapInternal::create(const boost::filesystem::path& pathname, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap)
{
    std::vector<boost::filesystem::path> pv{pathname};
    return GlobalHeapInternal::create(pv, heap_size, metazone_size, heap);
}

void store_ig(RRegion* region, RRegion::TPtr<nvHeap> nvheap)
{
    size_t heap_size = nvheap->size();
    size_t metazone_size = nvheap->metazone_size();
    size_t nzones = nvheap->nzones();

    std::vector<InterleaveGroup> vig;
    region->interleave_group(0, heap_size, &vig);

    for (size_t i=0; i<nzones; i++) {
        size_t vig_idx = i*metazone_size / region->file()->booksize();
        nvHeap::zone(nvheap, i)->header.ig = vig[vig_idx];
    }
}

int GlobalHeapInternal::create(const std::vector<boost::filesystem::path>& pathnames, size_t heap_size, size_t metazone_size, GlobalHeapInternal** heap)
{
    RRegion* region;

    LOG(info) << "Create heap path: " << paths_to_string(pathnames) << " size: "<< heap_size << " metazone_size: " << metazone_size;

    COERCE_ERROR_CODE(create_region(pathnames, heap_size, metazone_size));
    COERCE_ERROR_CODE(map_region(pathnames, &region));
    RRegion::TPtr<nvHeap> nvheap = nvHeap::make(region->base<nvHeap>(0x0), heap_size, metazone_size);

    if (nvheap == null_ptr) {
        return -1;
    }

    // store interleave-group information into each zone
    store_ig(region, nvheap);

    // TODO: just for TM: persist heap metadata

    GlobalHeapInternal* gh = new GlobalHeapInternal(pathnames, region, nvheap);
    ASSERT_ND(gh->init() == 0);
    *heap = gh;

    return 0;
}

int GlobalHeapInternal::open(const char* pathname, GlobalHeapInternal** heap)
{
    boost::filesystem::path p{pathname};
    return GlobalHeapInternal::open(p, heap);
}

int GlobalHeapInternal::open(const char** pathnames, int npathnames, GlobalHeapInternal** heap)
{
    std::vector<boost::filesystem::path> paths;

    for (int i=0; i<npathnames; i++) {
        paths.push_back(std::string(pathnames[i]));
    }

    return GlobalHeapInternal::open(paths, heap);
}

int GlobalHeapInternal::open(const boost::filesystem::path& pathname, GlobalHeapInternal** heap)
{
    std::vector<boost::filesystem::path> pv{pathname};
    return GlobalHeapInternal::open(pv, heap);
}

int GlobalHeapInternal::open(const std::vector<boost::filesystem::path>& pathnames, GlobalHeapInternal** heap)
{
    RRegion* region;

    LOG(info) << "Open heap path: " << paths_to_string(pathnames);

    COERCE_ERROR_CODE(map_region(pathnames, &region));
    RRegion::TPtr<nvHeap> nvheap = region->base<nvHeap>(0x0);

    LOG(info) << "Opened heap path: " << paths_to_string(pathnames) << " size: " << nvheap->size();

    GlobalHeapInternal* gh = new GlobalHeapInternal(pathnames, region, nvheap);
    ASSERT_ND(gh->init() == 0);
    *heap = gh;

    return 0;
}

int GlobalHeapInternal::format(const char* pathname)
{
    boost::filesystem::path p{pathname};
    return GlobalHeapInternal::format(p);
}

int GlobalHeapInternal::format_instance(const char* pathname, InstanceId instance_id)
{
    boost::filesystem::path p{pathname};
    return GlobalHeapInternal::format_instance(p, instance_id);
}

int GlobalHeapInternal::format(const char** pathnames, int npathnames)
{
    std::vector<boost::filesystem::path> paths;

    for (int i=0; i<npathnames; i++) {
        paths.push_back(std::string(pathnames[i]));
    }

    return GlobalHeapInternal::format(paths);
}

int GlobalHeapInternal::format_instance(const char** pathnames, int npathnames, InstanceId instance_id)
{
    std::vector<boost::filesystem::path> paths;

    for (int i=0; i<npathnames; i++) {
        paths.push_back(std::string(pathnames[i]));
    }

    return GlobalHeapInternal::format_instance(paths, instance_id);
}

int GlobalHeapInternal::format(const boost::filesystem::path& pathname)
{
    std::vector<boost::filesystem::path> pv{pathname};
    return GlobalHeapInternal::format(pv);
}

int GlobalHeapInternal::format_instance(const boost::filesystem::path& pathname, InstanceId instance_id)
{
    std::vector<boost::filesystem::path> pv{pathname};
    return GlobalHeapInternal::format_instance(pv, instance_id);
}

int GlobalHeapInternal::format(const boost::filesystem::path& pathname, ZoneId zone_id)
{
    std::vector<boost::filesystem::path> pv{pathname};
    return GlobalHeapInternal::format(pv, zone_id);
}

int GlobalHeapInternal::format(const boost::filesystem::path& pathname, std::vector<ZoneId> zone_ids)
{
    std::vector<boost::filesystem::path> pv{pathname};
    return GlobalHeapInternal::format(pv, zone_ids);
}

int GlobalHeapInternal::format(const std::vector<boost::filesystem::path>& pathnames)
{
    int ret;
    RRegion* region;

    LOG(info) << "Format heap path: " << paths_to_string(pathnames);

    COERCE_ERROR_CODE(map_region(pathnames, &region));
    RRegion::TPtr<nvHeap> nvheap = region->base<nvHeap>(0x0);

    for (size_t zid=0; zid<nvheap->nzones(); zid++) {
        if ((ret = format_zone(nvheap, zid, true)) != 0) {
            assert(kErrorCodeOk == Pegasus::address_space()->unmap(region));
            return ret;
        }
    }

    assert(kErrorCodeOk == Pegasus::address_space()->unmap(region));
    return 0;
}

int GlobalHeapInternal::format_instance(const std::vector<boost::filesystem::path>& pathnames, InstanceId instance_id)
{
    int ret;
    RRegion* region;
 
    LOG(info) << "Format heap path: " << paths_to_string(pathnames) << " instance: " << instance_id;

    COERCE_ERROR_CODE(map_region(pathnames, &region));
    RRegion::TPtr<nvHeap> nvheap = region->base<nvHeap>(0x0);

    for (size_t zid=0; zid<nvheap->nzones(); zid++) {
        if (instance_id == nvheap->zone(zid)->header.lease.lock_status()) {
            if ((ret = format_zone(nvheap, zid, false)) != 0) {
                assert(kErrorCodeOk == Pegasus::address_space()->unmap(region));
                return ret;
            }
        }
    }

    assert(kErrorCodeOk == Pegasus::address_space()->unmap(region));
    return 0;
}

int GlobalHeapInternal::format(const std::vector<boost::filesystem::path>& pathnames, ZoneId zone_id)
{
    std::vector<ZoneId> zone_ids{zone_id};
    return GlobalHeapInternal::format(pathnames, zone_ids);
}

int GlobalHeapInternal::format(const std::vector<boost::filesystem::path>& pathnames, std::vector<ZoneId> zone_ids)
{
    int ret;
    RRegion* region;

    LOG(info) << "Format heap path: " << paths_to_string(pathnames);

    COERCE_ERROR_CODE(map_region(pathnames, &region));
    RRegion::TPtr<nvHeap> nvheap = region->base<nvHeap>(0x0);

    for (std::vector<ZoneId>::iterator it = zone_ids.begin(); it != zone_ids.end(); it++)
    {
        // dont' format the heap header as we are not formattting the complete heap
        if ((ret = format_zone(nvheap, *it, false)) != 0) {
            assert(kErrorCodeOk == Pegasus::address_space()->unmap(region));
            return ret;
        }
    }

    assert(kErrorCodeOk == Pegasus::address_space()->unmap(region));
    return 0;
}

int GlobalHeapInternal::format_zone(RRegion::TPtr<nvHeap> nvheap, ZoneId zone_id, bool format_heap_header)
{
    LOG(info) << "Format heap zone: " << zone_id;

    if (format_heap_header) {
        size_t heapsize = nvheap->size();
        size_t metazone_log2size = nvheap->metazone_log2size();
        nvHeapHeader::make(&nvheap->metazone(zone_id)->heapheader, heapsize, metazone_log2size); 
    }
    size_t metazone_size = nvheap->metazone_size();
    size_t metazone_log2size = nvheap->metazone_log2size();
    nvZone::make(nvheap->zone(zone_id), metazone_size, metazone_log2size); 

    return 0;
}

int GlobalHeapInternal::close()
{
    LOG(info) << "Close heap path: " << paths_to_string(pathnames_);

    assert(teardown() == 0);
    assert(kErrorCodeOk == Pegasus::address_space()->unmap(region_));
    assert(kErrorCodeOk == region_->file()->close());
    return 0;
}

int GlobalHeapInternal::init()
{
    if (Pegasus::topology_factory()->construct(pathnames_[0], &topology_) != kErrorCodeOk) {
        return -1;
    }
    generation_ = nvheap_->header()->lease_superblock.incr_generation();
    return 0;
}

int GlobalHeapInternal::teardown()
{
    for (MemAttribAllocators<MemAttribHeap>::iterator it = memattrib_heaps_.begin();
         it != memattrib_heaps_.end();
         it++) 
    {
        MemAttribHeap* heap = it->second;
        assert(heap->teardown() == 0);
        delete heap;
    }
    return 0;
}

GlobalHeapInternal::InstanceId GlobalHeapInternal::instance()
{
    return generation_;
}

MemAttribHeap* GlobalHeapInternal::find_or_bind(const MemAttrib& memattrib)
{
    memattrib_heaps_.lock_read();
    MemAttribHeap* heap = memattrib_heaps_.find(memattrib);
    memattrib_heaps_.unlock_read();
    if (!heap) {
        memattrib_heaps_.lock_write();
        // make sure no one else added the memory attrib in the meantime
        heap = memattrib_heaps_.find(memattrib);
        if (!heap) {
            heap = new MemAttribHeap(nvheap_, generation_, memattrib);
            if (heap->init() != kErrorCodeOk) {
                delete heap;
                memattrib_heaps_.unlock_write();
                return NULL;
            }
            memattrib_heaps_.bind(memattrib, heap);
        }
        memattrib_heaps_.unlock_write();
    }
    assert(heap != NULL);
    return heap;
}

RRegion::TPtr<void> GlobalHeapInternal::malloc(size_t size)
{
    InterleaveGroup ig;
    unsigned int i;

    LOG(info) << "Allocate block size: " << size;

    // Start with the nearest interleave group and if that fails try all
    // other interleave groups until we successfully allocate memory
    for (i = 0, ig = topology_->nearest_ig(); 
         i < static_cast<unsigned int>(topology_->max_interleave_group()+1); 
         i++, ig = ((ig+1) % (topology_->max_interleave_group()+1)) )
    {
        MemAttrib memattrib(ig);
        RRegion::TPtr<void> tptr;
        if ((tptr = malloc(size, memattrib)) != null_ptr) {
            LOG(info) << "Allocated block ptr: " << tptr << " size: " << size;
            return tptr;
        }
    }
    return null_ptr;
}

RRegion::TPtr<void> GlobalHeapInternal::malloc(size_t size, const MemAttrib& memattrib)
{
    LOG(info) << "Allocate block size: " << size << " " << memattrib;

    MemAttribHeap* heap = find_or_bind(memattrib);
    assert(heap != NULL);
    return heap->malloc(size);
}

void GlobalHeapInternal::free(RRegion::TPtr<void> ptr)
{
    RRegion::TPtr<nvZone> nvzone = nvheap_->zone(ptr);

    InterleaveGroup ig = nvzone->header.ig;
    MemAttrib memattrib(ig);

    LOG(info) << "Free ptr: " << ptr << " zone: " << nvZone::zone_id(nvheap_, nvzone) << " nvzone: " << nvzone << " " << memattrib;

    MemAttribHeap* heap = find_or_bind(memattrib);
    assert(heap != NULL);
    return heap->free(ptr);
}

RRegion::TPtr<void> GlobalHeapInternal::realloc(RRegion::TPtr<void> ptr, size_t size)
{
    RRegion::TPtr<nvZone> nvzone = nvheap_->zone(ptr);

    InterleaveGroup ig = nvzone->header.ig;
    MemAttrib memattrib(ig);
    MemAttribHeap* heap = find_or_bind(memattrib);
    assert(heap != NULL);

    LOG(info) << "Realloc ptr: " << ptr << " zone: " << nvZone::zone_id(nvheap_, nvzone) << " " << memattrib << " new_size: " << size;
    RRegion::TPtr<void> new_ptr = heap->malloc(size);
    if (new_ptr != null_ptr) {
        memcpy(new_ptr.get(), ptr.get(), size);
        heap->free(ptr);
    }
    return new_ptr;
}

} // namespace alps
