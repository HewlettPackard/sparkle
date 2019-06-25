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

#include "globalheap/nvslab.hh"
#include "globalheap/slab.hh"
#include "globalheap/slab_heap.hh"
#include "globalheap/size_class.hh"
#include "test_common.hh"
#include "test_heap_fixture.hh"

using namespace alps;

class SlabTest: public RegionTest { };

class ExtentHeapTest: public HeapTest {};

TEST_F(SlabTest, nvslab)
{
    RRegion::TPtr<nvSlab> nvslab = alloc(256*1024);

    int szcl_128K = sizeclass(128*1024);   
    int szcl_32K = sizeclass(32*1024);   
    int szcl_1K = sizeclass(1024);   

    nvSlab::make(nvslab, szcl_128K);
    EXPECT_EQ(1U, nvslab->nblocks());
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
    EXPECT_EQ(0U, nvslab->header.size() % kCacheLineSize);
 
    nvSlab::make(nvslab, szcl_32K);
    EXPECT_EQ(7U, nvslab->nblocks());
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
    EXPECT_EQ(0U, nvslab->header.size() % kCacheLineSize);
    
    nvSlab::make(nvslab, szcl_1K);
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
    
    nvSlab::make(nvslab, 0);
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
}

TEST_F(SlabTest, nvslab_alloc_block_130)
{
    char pattern[1024];
    memset(pattern, 0x0, 1024);

    RRegion::TPtr<nvSlab> nvslab = alloc(256*1024);

    int szcl = sizeclass(130);   
    int blksz = size_from_class(szcl);

    nvSlab::make(nvslab, szcl);

    // allocate block 0 and write pattern
    RRegion::TPtr<char> blk0 = nvslab->block(0);
    EXPECT_EQ(1, nvslab->is_free(0)); 
    memcpy(blk0.get(), pattern, blksz);
    nvslab->set_alloc(0);
    EXPECT_EQ(0, memcmp(blk0.get(), pattern, blksz));
    EXPECT_EQ(0, nvslab->is_free(0));

    // allocate max block
    int max_block_id = nvSlabHeader::max_nblocks(256*1024, blksz) - 1;
    EXPECT_EQ(1, nvslab->is_free(max_block_id)); 
    nvslab->set_alloc(max_block_id);
    EXPECT_EQ(0, memcmp(blk0.get(), pattern, blksz));
    EXPECT_EQ(0, nvslab->is_free(max_block_id)); 
}

TEST_F(SlabTest, nvslab_alloc_block_1K)
{
    RRegion::TPtr<nvSlab> nvslab = alloc(256*1024);

    int szcl = sizeclass(1024);   

    nvSlab::make(nvslab, szcl);

    EXPECT_EQ(1, nvslab->is_free(0)); 
    nvslab->set_alloc(0);
    EXPECT_EQ(0, nvslab->is_free(0)); 


    EXPECT_EQ(1, nvslab->is_free(1)); 
    nvslab->set_alloc(1);
    EXPECT_EQ(0, nvslab->is_free(1)); 
}




TEST_F(SlabTest, load_nvslab)
{
    RRegion::TPtr<nvSlab> nvslab = alloc(256*1024);

    int szcl_1K = sizeclass(1024);   

    nvSlab::make(nvslab, szcl_1K);

    nvslab->set_alloc(0);
    nvslab->set_alloc(1);
    nvslab->set_alloc(3);

    Slab slab(nvslab);
    EXPECT_EQ(slab.nblocks() - 3, slab.nblocks_free());
}

TEST_F(SlabTest, slab_alloc_block)
{
    RRegion::TPtr<nvSlab> nvslab = alloc(256*1024);

    int szcl_1K = sizeclass(1024);   

    nvSlab::make(nvslab, szcl_1K);

    Slab slab(nvslab);
    slab.alloc_block();
    slab.alloc_block();
    slab.alloc_block();
    EXPECT_EQ(slab.nblocks() - 3, slab.nblocks_free());

    Slab shadow_slab(nvslab);
    EXPECT_EQ(slab.nblocks_free(), shadow_slab.nblocks_free());
}

TEST_F(ExtentHeapTest, load_nvslab)
{
    RRegion::TPtr<void> e1 = extentheap()->malloc(256*1024);
    RRegion::TPtr<void> e2 = extentheap()->malloc(4*256*1024);
    RRegion::TPtr<void> e3 = extentheap()->malloc(256*1024);
    UNUSED_ND(e3);
    extentheap()->free(e2);
    int szcl_1K = sizeclass(1024);   
    nvSlab::make(e1, szcl_1K);

    close_heap();
    open_heap(test_path("globalheap0"));
    SlabHeap slabheap(extentheap());
    InsertSlabFunctor functor(&slabheap);
    extentheap()->more_space(1, functor);
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
