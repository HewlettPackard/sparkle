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

#ifndef _ALPS_TEST_HEAP_FIXTURE_HH_
#define _ALPS_TEST_HEAP_FIXTURE_HH_

#include "globalheap/extentheap.hh"
#include "globalheap/globalheap_internal.hh"
#include "globalheap/process_slab_heap.hh"
#include "globalheap/zone_heap.hh"
#include "globalheap/zone.hh"
#include "test_common.hh"

namespace alps {

class GlobalHeapTest: public RegionFileTest { 
public:
    size_t global_heap_size = 2 * booksize();
    size_t global_metazone_size = booksize();

    void SetUp() {
        RegionFileTest::SetUp();
    }

    void TearDown() {

    }
};


// A fixture class that opens/creates a heap before the test starts 
// and closes the heap after the test ends.
class AutoGlobalHeapTest: public GlobalHeapTest {
public:
    void SetUp() {
        GlobalHeapTest::SetUp();
        EXPECT_EQ(0, GlobalHeapInternal::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap_));
        ASSERT_EQ(0, heap_->close());
        EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap_));
    }
 
    void TearDown() {
        ASSERT_EQ(0, heap_->close());
    }

protected:
    GlobalHeapInternal* heap_;
};


class HeapTest: public RegionFileTest {
public:
    size_t global_heap_size = 2 * booksize();
    size_t global_metazone_size = booksize();

public:
    void SetUp() {
        RegionFileTest::SetUp();
        create_heap(test_path("globalheap0"), global_heap_size, global_metazone_size);
        close_heap();
        open_heap(test_path("globalheap0"));
    }

    void create_heap(const boost::filesystem::path& pathname, size_t heap_size, size_t metazone_size)
    {
        std::vector<boost::filesystem::path> pv{pathname};
        assert(create_region(pv, heap_size, metazone_size) == kErrorCodeOk);
        assert(map_region(pv, &region_) == kErrorCodeOk);
        nvheap_ = nvHeap::make(region_->base<nvHeap>(0x0), heap_size, metazone_size);
    }

    void open_heap(const boost::filesystem::path& pathname)
    {
        std::vector<boost::filesystem::path> pv{pathname};
        ASSERT_EQ(kErrorCodeOk, map_region(pv, &region_));
        nvheap_ = region_->base<nvHeap>(0x0);
        extentheap_ = new ExtentHeap(nvheap_, 1);
        extentheap_->init();
    }

    void close_heap()
    {
        assert(Pegasus::address_space()->unmap(region_) == kErrorCodeOk);
    }

    void TearDown() {
        close_heap();
    }

    ZoneHeap* zoneheap() {
        return static_cast<ZoneHeap*>(extentheap_);
    }

    ExtentHeap* extentheap() {
        return extentheap_;
    }

public:
    RRegion::TPtr<nvHeap> nvheap_;
    RRegion* region_;
    ExtentHeap* extentheap_;
    ProcessSlabHeap* process_slab_heap_;
};

} // namespace alps

#endif // _ALPS_TEST_HEAP_FIXTURE_HH_
