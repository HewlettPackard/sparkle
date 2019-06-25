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
#include <sstream>

#include "gtest/gtest.h"
#include "alps/common/assorted_func.hh"

#include "alps/layers/bits/slab.hh"
#include "alps/layers/pointer.hh"
#include "alps/layers/slabheap.hh"
#include "alps/layers/extentheap.hh"
#include "alps/layers/hybridheap.hh"

#include "test_common.hh"
#include "test_layers_common.hh"

using namespace alps;

typedef SlabHeap<Context, TPtr, PPtr> SlabHeap_t;
typedef ExtentHeap<Context, TPtr, PPtr> ExtentHeap_t;
typedef HybridHeap<Context, TPtr, PPtr, SlabHeap_t, ExtentHeap_t> HybridHeap_t;

size_t region_size = 1024*1024;
size_t block_log2size = 13; // 4KB

TEST(HybridHeapTest, alloc_free)
{
    Context ctx;

    size_t slabsize = 1 << block_log2size ;
    size_t bigsize = slabsize;

    TPtr<void> region = malloc(region_size);

    ExtentHeap_t* exheap = ExtentHeap_t::make(region, region_size, block_log2size);
 
    SlabHeap_t slheap(slabsize, NULL, exheap);
    slheap.init(ctx);

    HybridHeap_t hheap(bigsize, &slheap, exheap);

    TPtr<void> ptr[16];
    EXPECT_EQ(kErrorCodeOk, hheap.malloc(ctx, bigsize*2, &ptr[0]));
    EXPECT_EQ(bigsize*2, hheap.getsize(ptr[0]));

    EXPECT_EQ(kErrorCodeOk, hheap.malloc(ctx, bigsize/4, &ptr[1]));
    EXPECT_EQ(bigsize/4, hheap.getsize(ptr[1]));

    hheap.free(ctx, ptr[0]);
    hheap.free(ctx, ptr[1]);
} 


int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
