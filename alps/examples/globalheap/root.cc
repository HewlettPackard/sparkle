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

#include "boost/filesystem/path.hpp"
#include "alps/globalheap/globalheap.hh"


#define HEAP_PATH "/dev/shm/nvm/root" // can be also a path to a file in /lfs
#define HEAP_SIZE 128*1024*1024LLU // 128 MB
#define METAZONE_SIZE 8*1024*1024LLU // 8 MB (must be power of two)

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

void verify() {
    GlobalHeap* heap;
    assert(0 == GlobalHeap::open(HEAP_PATH, &heap));
    printf("Verifying data\n");
    RRegion::TPtr<RootBlock> rtp = heap->root<RootBlock>();
    for (int v=0; v<NUM_VERTICES; v++) {
        printf("arr[%d] = %d\n", v, rtp->vertices[v]->data);
    }
    assert(0 == heap->close());
}


int main()
{
    GlobalHeap* heap;

    // Initialize PEGASUS address space 
    Pegasus::init((const char*) NULL, false, false);

    // Create directory containing heap
    boost::filesystem::path heap_dir = boost::filesystem::path(HEAP_PATH).parent_path();
    boost::filesystem::create_directories(heap_dir);

    // Create and Open a heap
    assert(0 == GlobalHeap::create(HEAP_PATH, HEAP_SIZE, METAZONE_SIZE, &heap));
    assert(0 == heap->close());
    assert(0 == GlobalHeap::open(HEAP_PATH, &heap));

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

    for (int v=0; v<NUM_VERTICES; v++) {
      printf("arr[%d] = %d\n", v, rtp->vertices[v]->data);
    }

    // Safely close the heap
    assert(0 == heap->close());
    verify();
    return 0;
}

