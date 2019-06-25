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

#include "alps/globalheap/globalheap.hh"

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
#include "globalheap/extentheap.hh"
#include "globalheap/layout.hh"
#include "globalheap/globalheap_internal.hh"


namespace alps {


int GlobalHeap::create(const char* path, size_t heap_size, size_t metazone_size, GlobalHeap** heap)
{
    int ret;
    GlobalHeapInternal* iheap;

    if ((ret = GlobalHeapInternal::create(path, heap_size, metazone_size, &iheap)) != 0) {
        return ret;
    }
    
    GlobalHeap* gh = new GlobalHeap(iheap);
    *heap = gh;
    
    return 0;
}

int GlobalHeap::create(const char** paths, int npaths, size_t heap_size, size_t metazone_size, GlobalHeap** heap)
{
    int ret;
    GlobalHeapInternal* iheap;

    if ((ret = GlobalHeapInternal::create(paths, npaths, heap_size, metazone_size, &iheap)) != 0) {
        return ret;
    }
    
    GlobalHeap* gh = new GlobalHeap(iheap);
    *heap = gh;

    return 0;
}

int GlobalHeap::open(const char* path, GlobalHeap** heap)
{
    int ret;
    GlobalHeapInternal* iheap;

    if ((ret = GlobalHeapInternal::open(path, &iheap)) != 0) {
        return ret;
    }

    GlobalHeap* gh = new GlobalHeap(iheap);
    *heap = gh;

    return 0;
}

int GlobalHeap::open(const char** paths, int npaths, GlobalHeap** heap)
{
    int ret;
    GlobalHeapInternal* iheap;

    if ((ret = GlobalHeapInternal::open(paths, npaths, &iheap)) != 0) {
        return ret;
    }

    GlobalHeap* gh = new GlobalHeap(iheap);
    *heap = gh;

    return 0;
}

int GlobalHeap::format(const char* path)
{
    return GlobalHeapInternal::format(path);
}

int GlobalHeap::format(const char** paths, int npaths)
{
    return GlobalHeapInternal::format(paths, npaths);
}

int GlobalHeap::format_instance(const char* path, InstanceId instance_id)
{
    return GlobalHeapInternal::format_instance(path, instance_id);
}

int GlobalHeap::format_instance(const char** paths, int npaths, InstanceId instance_id)
{
    return GlobalHeapInternal::format_instance(paths, npaths, instance_id);
}

int GlobalHeap::close()
{
    return globalheap_internal_->close();
}

size_t GlobalHeap::size()
{
    return globalheap_internal_->size();
}

GlobalHeap::InstanceId GlobalHeap::instance()
{
    return globalheap_internal_->instance();
}

RRegion::TPtr<void> GlobalHeap::__root()
{
    return globalheap_internal_->root<void>();
}

void GlobalHeap::__set_root(RRegion::TPtr<void> root)
{
    globalheap_internal_->set_root<void>(root);
}

RRegion::TPtr<void> GlobalHeap::malloc(size_t size)
{
    return globalheap_internal_->malloc(size);
}

RRegion::TPtr<void> GlobalHeap::malloc(size_t size, const MemAttrib& memattrib)
{
    return globalheap_internal_->malloc(size, memattrib);
}

void GlobalHeap::free(RRegion::TPtr<void> ptr)
{
    globalheap_internal_->free(ptr);
}

RRegion::TPtr<void> GlobalHeap::realloc(RRegion::TPtr<void> ptr, size_t size)
{
    return globalheap_internal_->realloc(ptr, size);
}

RRegion* GlobalHeap::region()
{
    return globalheap_internal_->region();
}

} // namespace alps
