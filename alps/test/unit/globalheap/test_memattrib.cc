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

#include "gtest/gtest.h"
#include "alps/globalheap/memattrib.hh"

#include "globalheap/memattrib_alloc.hh"

using namespace alps;

class DummyAllocator
{
public:
    DummyAllocator(int id)
        : id_(id)
    { }

    int id() {
        return id_;
    }

private:
    int id_;
};


TEST(MemAttrib, bind_and_find)
{
    DummyAllocator* alloc;
    MemAttribAllocators<DummyAllocator> allocators;

    MemAttrib ma1(1);
    MemAttrib ma2(2);

    allocators.bind(ma1, new DummyAllocator(1));
    allocators.bind(ma2, new DummyAllocator(2));

    EXPECT_NE((DummyAllocator*)0, alloc = allocators.find(ma1));
    EXPECT_EQ(1, alloc->id());

    EXPECT_NE((DummyAllocator*)0, alloc = allocators.find(ma2));
    EXPECT_EQ(2, alloc->id());
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

