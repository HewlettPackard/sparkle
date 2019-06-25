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

#include <stdlib.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include "alps/layers/bits/bitmap.hh"

#include "test_layers_common.hh"

using namespace alps;

typedef nvBitMap<Context> nvBitMap_t;

void test_make(Context& ctx, int bitmap_len)
{
    char buf[64];
  
    // set all buffer bits to one 
    memset(buf, 0xff, sizeof(buf));

    nvBitMap_t* bm = nvBitMap_t::make(ctx, bitmap_len, buf);

    for (int i=0; i<bitmap_len; i++) {
        EXPECT_EQ(0, bm->is_set(ctx, i));
    }
}

TEST(BitMap, make_len4)
{
    Context ctx;
    test_make(ctx, 4);
}

TEST(BitMap, make_len8)
{
    Context ctx;
    test_make(ctx, 8);
}

TEST(BitMap, make_len12)
{
    Context ctx;
    test_make(ctx, 12);
}

TEST(BitMap, make_len16)
{
    Context ctx;
    test_make(ctx, 16);
}

TEST(BitMap, make_len256)
{
    Context ctx;
    test_make(ctx, 256);
}

TEST(BitMap, set)
{
    Context ctx;
    char buf[64];
    int bitmap_len = 256;

    nvBitMap_t* bm = nvBitMap_t::make(ctx, bitmap_len, buf);
  
    for (int i=0; i<bitmap_len; i++) {
        bm->set(ctx, i);
        EXPECT_EQ(1, bm->is_set(ctx, i));
    }
}

void test_clear(int bitmap_len)
{
    Context ctx;
    char buf[64];
  
    nvBitMap_t* bm = nvBitMap_t::make(ctx, bitmap_len, buf);

    for (int i=0; i<bitmap_len; i++) {
        bm->set(ctx, i);
        EXPECT_EQ(1, bm->is_set(ctx, i));
    }
 
    for (int i=0; i<bitmap_len; i++) {
        bm->clear(ctx, i);
        EXPECT_EQ(0, bm->is_set(ctx, i));
    }
}

TEST(BitMap, clear)
{
    test_clear(4);
    test_clear(8);
    test_clear(12);
    test_clear(16);
    test_clear(256);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

