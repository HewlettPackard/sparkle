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

#include <fcntl.h>

#include "gtest/gtest.h"
#include "alps/globalheap/globalheap.hh"

#include "common/os.hh"
#include "globalheap/globalheap_internal.hh"
#include "test_heap_fixture.hh"

using namespace alps;

TEST_F(GlobalHeapTest, public_create)
{
    GlobalHeap* heap;  
    EXPECT_EQ(0, GlobalHeap::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
}

TEST_F(GlobalHeapTest, public_open)
{
    GlobalHeap* heap;  
    EXPECT_EQ(0, GlobalHeap::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
    EXPECT_EQ(0, GlobalHeap::open(test_path("globalheap0").c_str(), &heap));
    ASSERT_EQ(0, heap->close());
}

TEST_F(GlobalHeapTest, create)
{
    GlobalHeapInternal* heap;  
    EXPECT_EQ(0, GlobalHeapInternal::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
}

TEST_F(GlobalHeapTest, open)
{
    GlobalHeapInternal* heap;  
    EXPECT_EQ(0, GlobalHeapInternal::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap));
    ASSERT_EQ(0, heap->close());
}

TEST_F(GlobalHeapTest, check_mapping)
{
    GlobalHeapInternal* heap;  
    EXPECT_EQ(0, GlobalHeapInternal::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap));

    size_t range_start = reinterpret_cast<size_t>(heap->region()->base<void>(0).get());
    size_t range_end = reinterpret_cast<size_t>(heap->region()->base<void>(0).get()) + global_heap_size;

    // this check assumes the heap file is mapped as a single contiguous memory region
    ProcessMap pmap;
    std::pair<size_t, size_t> pmap_range = pmap.range(test_path("globalheap0"));

    EXPECT_EQ(range_start, pmap_range.first);
    EXPECT_EQ(range_end, pmap_range.second);

    ASSERT_EQ(0, heap->close());
}


// This test will fail if interleaving of books is not precise, meaning
// allocation of books to interleave groups does not happen precisely as
// requested. The precise allocation pattern is: 
//   0x000000...00 010101...01 .. 0N0N0N...ON 
// where N is the number of max inteleave group.
TEST_F(GlobalHeapTest, check_precise_interleaving)
{
    GlobalHeapInternal* heap;  
    Topology*           topology;

    ASSERT_EQ(kErrorCodeOk, Pegasus::topology_factory()->construct(test_path("globalheap0"), &topology));
    size_t num_avail_igs = topology->max_interleave_group() + 1;
 
    EXPECT_EQ(0, GlobalHeapInternal::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap));

    RRegion::TPtr<nvHeap> nvheap = heap->region()->base<nvHeap>(0x0);
    InterleaveGroup ig;
    size_t zid;
    for (zid=0, ig = 0; zid<nvheap->nzones(); zid++, ig = ((ig+1) % num_avail_igs)) {
        RRegion::TPtr<struct nvMetaZone> metazone = nvheap->metazone(zid);
        RRegion::TPtr<struct nvZone> nvzone = metazone->zone();
        EXPECT_EQ(ig, nvzone->header.ig);
    }

    ASSERT_EQ(0, heap->close());
}


TEST_F(GlobalHeapTest, format)
{
    GlobalHeapInternal::InstanceId instance_id;
    GlobalHeapInternal* heap;  
    RRegion::TPtr<void> p1;
    RRegion::TPtr<void> p2;
    RRegion::TPtr<void> p3;

    EXPECT_EQ(0, GlobalHeapInternal::create(test_path("globalheap0").c_str(), global_heap_size, global_metazone_size, &heap));
    ASSERT_EQ(0, heap->close());

    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap));
    instance_id = heap->instance();
    p1 = heap->malloc(1024);
    EXPECT_NE(null_ptr, p1);
    ASSERT_EQ(0, heap->close());
    //EXPECT_EQ(0, GlobalHeapInternal::format_instance(test_path("globalheap0").c_str(), instance_id));

    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap));
    instance_id = heap->instance();
    p2 = heap->malloc(1024);
    EXPECT_NE(null_ptr, p2);
    ASSERT_EQ(0, heap->close());
    //EXPECT_EQ(0, GlobalHeapInternal::format_instance(test_path("globalheap0").c_str(), instance_id));

    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap));
    instance_id = heap->instance();
    p3 = heap->malloc(1024);
    EXPECT_NE(null_ptr, p3);
    ASSERT_EQ(0, heap->close());
    //EXPECT_EQ(0, GlobalHeapInternal::format_instance(test_path("globalheap0").c_str(), instance_id));
}



TEST_F(AutoGlobalHeapTest, alloc)
{
    RRegion::TPtr<void> p1 = heap_->malloc(1024);
    RRegion::TPtr<void> p2 = heap_->malloc(2048);
    RRegion::TPtr<void> p3 = heap_->malloc(256*1024);
    RRegion::TPtr<void> p4 = heap_->malloc(512*1024);
    EXPECT_NE(null_ptr, p1);
    EXPECT_NE(null_ptr, p2);
    EXPECT_NE(null_ptr, p3);
    EXPECT_NE(null_ptr, p4);
}


// Disabling large unit test that takes long to run
#if 0
TEST_F(AutoGlobalHeapTest, alloc_many)
{
    size_t range_start = reinterpret_cast<size_t>(heap_->region()->base<void>(0).get());
    size_t range_end = reinterpret_cast<size_t>(heap_->region()->base<void>(0).get() + global_heap_size);

    for (int i=0; i<8192+2; i++) {
        RRegion::TPtr<void> p1 = heap_->malloc(302150);
        EXPECT_NE(null_ptr, p1);
        ASSERT_LE(range_start, reinterpret_cast<size_t>(p1.get()));   
        ASSERT_GT(range_end, reinterpret_cast<size_t>(p1.get()));   
        
    }
}
#endif

TEST_F(AutoGlobalHeapTest, alloc_free)
{
    RRegion::TPtr<void> p1 = heap_->malloc(1024);
    RRegion::TPtr<void> p2 = heap_->malloc(256*1024);
    EXPECT_NE(null_ptr, p1);
    EXPECT_NE(null_ptr, p2);

    heap_->free(p1);
    heap_->free(p2);
}

TEST_F(AutoGlobalHeapTest, alloc_reload)
{
#ifdef BASE_RELATIVE_POINTERS
    RRegion::TPtr<void> tp1 = heap_->malloc(1024);
    RRegion::TPtr<void> tp2 = heap_->malloc(2048);
    RRegion::TPtr<void> tp3 = heap_->malloc(256*1024);
    EXPECT_NE(null_ptr, tp1);
    EXPECT_NE(null_ptr, tp2);
    EXPECT_NE(null_ptr, tp3);

    RRegion::ZPtr<void> zp1 = tp1;
    RRegion::ZPtr<void> zp3 = tp3;

    heap_->close();
    EXPECT_EQ(0, GlobalHeapInternal::open(test_path("globalheap0").c_str(), &heap_));

    RRegion::TPtr<void> tp11 = zp1;
    heap_->free(tp11);

    RRegion::TPtr<void> tp13 = zp3;
    heap_->free(tp13);

    RRegion::TPtr<void> tp12 = heap_->malloc(1024);
    EXPECT_NE(null_ptr, tp12);
#endif
}

TEST_F(AutoGlobalHeapTest, memattrib_alloc)
{
    MemAttrib ma0(0);
    MemAttrib ma1(1);

    Topology* topology;
    ASSERT_EQ(kErrorCodeOk, Pegasus::topology_factory()->construct(test_path("dummy"), &topology));
    size_t num_avail_igs = topology->max_interleave_group() + 1;

    // only run if we are running on a machine with enough interleave groups
    if (num_avail_igs > 1) { 
            RRegion::TPtr<void> p1 = heap_->malloc(1024, ma1);
            RRegion::TPtr<void> p2 = heap_->malloc(2048, ma0);
            EXPECT_NE(null_ptr, p1);
            EXPECT_NE(null_ptr, p2);
    }
}

TEST_F(AutoGlobalHeapTest, memattrib_alloc_free)
{
    MemAttrib ma0(0);
    MemAttrib ma1(1);

    Topology* topology;
    ASSERT_EQ(kErrorCodeOk, Pegasus::topology_factory()->construct(test_path("dummy"), &topology));
    size_t num_avail_igs = topology->max_interleave_group() + 1;

    // only run if we are running on a machine with enough interleave groups
    if (num_avail_igs > 1) { 
        RRegion::TPtr<void> p1 = heap_->malloc(1024, ma1);
        RRegion::TPtr<void> p2 = heap_->malloc(2048, ma0);
        EXPECT_NE(null_ptr, p1);
        EXPECT_NE(null_ptr, p2);

        heap_->free(p1);
        heap_->free(p2);
    }
}

TEST_F(AutoGlobalHeapTest, root)
{
    RRegion::TPtr<int> tp1;
    RRegion::TPtr<int> tp2;

    heap_->set_root(tp1);
    tp2 = heap_->root<int>();
}


int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
