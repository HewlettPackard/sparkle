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
#include "alps/layers/bits/freespacemap.hh"
#include "alps/layers/pointer.hh"
#include "test_common.hh"

using namespace alps;

TEST(FreeSpaceMapTest, alloc_extent)
{
    FreeSpaceMap<TPtr> fsmap;

    size_t LEN = 64;
    size_t MAX_BLOCK = LEN-1;

    fsmap.free_extent(ExtentInterval(0, LEN));

    ExtentInterval e1;
    ExtentInterval e2;
    ExtentInterval e3;
    ExtentInterval e4;
    ExtentInterval e5;

    EXPECT_EQ(0, fsmap.alloc_extent(1, &e1));
    EXPECT_EQ(0, fsmap.alloc_extent(2, &e2));
    EXPECT_EQ(0, fsmap.alloc_extent(3, &e3));

    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(ExtentInterval(6, MAX_BLOCK-6));
        EXPECT_EQ_VERBOSE(fsmap_expect, fsmap);
    }

    fsmap.free_extent(e1);

    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(ExtentInterval(0, 1));
        fsmap_expect.insert(ExtentInterval(6, MAX_BLOCK-6));
        EXPECT_EQ_VERBOSE(fsmap_expect, fsmap);
    }

    EXPECT_EQ(0, fsmap.alloc_extent(4, &e4));

    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(ExtentInterval(0, 1));
        fsmap_expect.insert(ExtentInterval(10, MAX_BLOCK-10));
        EXPECT_EQ_VERBOSE(fsmap_expect, fsmap);
    }

    EXPECT_EQ(0, fsmap.alloc_extent(1, &e5));

    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(ExtentInterval(10, MAX_BLOCK-10));
        EXPECT_EQ_VERBOSE(fsmap_expect, fsmap);
    }

    UNUSED_ND(e2);
    UNUSED_ND(e3);
    UNUSED_ND(e4);
    UNUSED_ND(e5);
}

TEST(FreeSpaceMapTest, alloc_extent2)
{
    FreeSpaceMap<TPtr> fsmap;

    size_t LEN = 64;

    fsmap.free_extent(ExtentInterval(0, LEN));

    ExtentInterval e1;
    ExtentInterval e2;
    ExtentInterval e3;
    ExtentInterval e4;
    ExtentInterval e5;

    EXPECT_EQ(0, fsmap.alloc_extent(LEN-2, &e1));
    EXPECT_NE(0, fsmap.alloc_extent(3, &e2));
    EXPECT_EQ(0, fsmap.alloc_extent(2, &e2));
}


int main(int argc, char** argv)
{
    //::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
