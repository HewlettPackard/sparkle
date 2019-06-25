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

#include <assert.h>
#include <fcntl.h>

#include "gtest/gtest.h"
#include "alps/globalheap/globalheap.hh"

#include "globalheap/globalheap_internal.hh"
#include "test_heap_fixture.hh"

#define NUM_VERTICES 32

using namespace alps;

struct Element {
};

struct Vertex {
    int data;
    RRegion::PPtr<Element> adj_list;
};

struct RootBlock {
    RRegion::PPtr<Vertex> vertices[NUM_VERTICES];
};

bool verify(std::string heap_path) {
    GlobalHeap* heap;
    assert(0 == GlobalHeap::open(heap_path.c_str(), &heap));
    RRegion::TPtr<RootBlock> rtp = heap->root<RootBlock>();
    for (int v=0; v<NUM_VERTICES; v++) {
        if (v+10 != rtp->vertices[v]->data) {
            return false;
        }
    }
    assert(0 == heap->close());
    return true;
}


TEST_F(RegionFileTest, root)
{
    GlobalHeap* heap;
    std::string heap_path = test_path("rootheap0");
    size_t heap_size = 16 * booksize();
    size_t metazone_size = booksize();

    // Create and Open a heap
    ASSERT_EQ(0, GlobalHeap::create(heap_path.c_str(), heap_size, metazone_size, &heap));
    ASSERT_EQ(0, heap->close());
    ASSERT_EQ(0, GlobalHeap::open(heap_path.c_str(), &heap));

    // Allocate a root block and set the heap root pointer to point to the block
    RRegion::TPtr<RootBlock> rtp = heap->malloc(sizeof(RootBlock));
    heap->set_root(rtp);

    // Ensure root pointer is stored (not necessary; just for illustration purposes)
    RRegion::TPtr<RootBlock> rrtp = heap->root<RootBlock>();
    assert(rtp == rrtp);

    // Initialize vertices array with pointers to each vertex
    for (int v=0; v<NUM_VERTICES; v++) {
        rtp->vertices[v] = heap->malloc(sizeof(Vertex));
        rtp->vertices[v]->data = (v+10);
    }

    // Safely close the heap
    assert(0 == heap->close());
    verify(heap_path.c_str());
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
