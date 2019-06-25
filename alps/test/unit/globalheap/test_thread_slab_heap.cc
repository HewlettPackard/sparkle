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
#include "alps/pegasus/address_space.hh"
#include "alps/pegasus/relocatable_region.hh"
#include "globalheap/process_slab_heap.hh"
#include "globalheap/thread_slab_heap.hh"
#include "test_common.hh"
#include "test_heap_fixture.hh"

using namespace alps;

class ThreadSlabHeapTest: public HeapTest {};

TEST_F(ThreadSlabHeapTest, test1)
{
    ProcessSlabHeap process_slab_heap(extentheap());
    ThreadSlabHeap thread_slab_heap(&process_slab_heap);

    thread_slab_heap.malloc(1024);
    thread_slab_heap.malloc(1024);
    thread_slab_heap.malloc(1024);
    thread_slab_heap.malloc(128);

    std::cout << thread_slab_heap << std::endl;

    close_heap();

    std::cout << "===============" << std::endl;

    open_heap(test_path("globalheap0"));
    ProcessSlabHeap shadow_process_slab_heap(extentheap());
    ThreadSlabHeap shadow_thread_slab_heap(&shadow_process_slab_heap);


    std::cout << "---------------" << std::endl;

    std::cout << shadow_process_slab_heap << std::endl;

    std::cout << shadow_thread_slab_heap << std::endl;

    shadow_thread_slab_heap.malloc(1024);
    shadow_thread_slab_heap.malloc(2048);
    shadow_thread_slab_heap.malloc(128);

    std::cout << shadow_process_slab_heap << std::endl;

    std::cout << shadow_thread_slab_heap << std::endl;
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
