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

#include "test_layers_common.hh"

using namespace alps;

typedef nvSlab<Context, TPtr> nvSlab_t;

typedef Slab<Context, TPtr, PPtr> Slab_t;

template<template<typename> class TPtr>
class SlabTestT: public ::testing::Test {
public:
    template<typename T>
    TPtr<T> alloc(size_t size)
    {
        return (T*) malloc(size);
    }
};


typedef SlabTestT<TPtr> SlabTest;

const size_t slab_size = 256*1024;

TEST_F(SlabTest, nvslab)
{
    Context ctx;

    TPtr<nvSlab_t> nvslab = alloc<nvSlab_t>(256*1024);

    int szcl_128K = sizeclass(128*1024);   
    int szcl_32K = sizeclass(32*1024);   
    int szcl_1K = sizeclass(1024);   

    nvSlab_t::make(ctx, nvslab, slab_size, szcl_128K);
    EXPECT_EQ(1U, nvslab->nblocks());
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
    EXPECT_EQ(0U, nvslab->header.size() % kCacheLineSize);
 
    nvSlab_t::make(ctx, nvslab, slab_size, szcl_32K);
    EXPECT_EQ(7U, nvslab->nblocks());
    for (size_t i = 0; i<nvslab->nblocks(); i++) {
        EXPECT_EQ(i, nvslab->block_id(nvslab->block(i)));
    }
    for (size_t i = 0; i<nvslab->nblocks(); i++) {
        for (size_t j = 0; j<nvslab->nblocks(); j++) {
            if (i != j ) {
                EXPECT_NE(nvslab->block(i), nvslab->block(j));
            }
        }
    }
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
    EXPECT_EQ(0U, nvslab->header.size() % kCacheLineSize);
    
    nvSlab_t::make(ctx, nvslab, slab_size, szcl_1K);
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
    
    nvSlab_t::make(ctx, nvslab, slab_size, 0);
    EXPECT_EQ(1U, nvslab->block_id(nvslab->block(1)));
    EXPECT_EQ(nvslab->nblocks()-1, nvslab->block_id(nvslab->block(nvslab->nblocks()-1)));
}

TEST_F(SlabTest, nvslab_alloc_block_130)
{
    Context ctx;
    char pattern[1024];
    memset(pattern, 0x0, 1024);

    TPtr<nvSlab_t> nvslab = alloc<nvSlab_t>(256*1024);

    int szcl = sizeclass(130);   
    int blksz = size_from_class(szcl);

    nvSlab_t::make(ctx, nvslab, slab_size, szcl);

    // allocate block 0 and write pattern
    TPtr<void> blk0 = nvslab->block(0);
    EXPECT_EQ(1, nvslab->is_free(ctx, 0)); 
    memcpy(blk0.get(), pattern, blksz);
    nvslab->set_alloc(ctx, 0);
    EXPECT_EQ(0, memcmp(blk0.get(), pattern, blksz));
    EXPECT_EQ(0, nvslab->is_free(ctx, 0));

    // allocate max block
    int max_block_id = nvSlabHeader<Context,TPtr>::max_nblocks(256*1024, blksz) - 1;
    EXPECT_EQ(1, nvslab->is_free(ctx, max_block_id)); 
    nvslab->set_alloc(ctx, max_block_id);
    EXPECT_EQ(0, memcmp(blk0.get(), pattern, blksz));
    EXPECT_EQ(0, nvslab->is_free(ctx, max_block_id)); 
}

TEST_F(SlabTest, nvslab_alloc_block_1K)
{
    Context ctx;
    TPtr<nvSlab_t> nvslab = alloc<nvSlab_t>(256*1024);

    int szcl = sizeclass(1024);   

    nvSlab_t::make(ctx, nvslab, slab_size, szcl);

    EXPECT_EQ(1, nvslab->is_free(ctx, 0)); 
    nvslab->set_alloc(ctx, 0);
    EXPECT_EQ(0, nvslab->is_free(ctx, 0)); 


    EXPECT_EQ(1, nvslab->is_free(ctx, 1)); 
    nvslab->set_alloc(ctx, 1);
    EXPECT_EQ(0, nvslab->is_free(ctx, 1)); 
}

TEST_F(SlabTest, slab_load)
{
    Context ctx;
    TPtr<nvSlab_t> nvslab = alloc<nvSlab_t>(256*1024);

    int szcl_1K = sizeclass(1024);   

    nvSlab_t::make(ctx, nvslab, slab_size, szcl_1K);

    nvslab->set_alloc(ctx, 0);
    nvslab->set_alloc(ctx, 1);
    nvslab->set_alloc(ctx, 3);

    Slab_t* slab= Slab_t::load(ctx, nvslab);
    EXPECT_EQ(slab->nblocks() - 3, slab->nblocks_free());
}

TEST_F(SlabTest, slab_alloc_block)
{
    Context ctx;
    TPtr<nvSlab_t> nvslab = alloc<nvSlab_t>(256*1024);

    int szcl_1K = sizeclass(1024);   

    nvSlab_t::make(ctx, nvslab, slab_size, szcl_1K);

    Slab_t* slab = Slab_t::load(ctx, nvslab);
    slab->alloc_block(ctx);
    slab->alloc_block(ctx);
    slab->alloc_block(ctx);
    EXPECT_EQ(slab->nblocks() - 3, slab->nblocks_free());

    Slab_t* shadow_slab = Slab_t::load(ctx, nvslab);
    EXPECT_EQ(slab->nblocks_free(), shadow_slab->nblocks_free());
    delete slab;
    delete shadow_slab;
}


#if 0
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
#endif

int main(int argc, char** argv)
{
    //::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
