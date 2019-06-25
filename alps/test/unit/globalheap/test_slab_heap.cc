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
#include "globalheap/slab_heap.hh"
#include "test_common.hh"

using namespace alps;

// Test SlabHeap functionality with no extent heap and zone heap for 
// allocating space for non-volatile slabs. Instead provide a method 
// that mocks allocation of non-volatile slabs.
class SlabHeapTest: public RegionTest {
    const size_t slab_size = 256*1024;  
public:
    RRegion::TPtr<nvSlab> alloc_nvslab(size_t block_size, unsigned int perc_full) {
        RRegion::TPtr<nvSlab> nvslab = alloc(slab_size);
        nvSlab::make(nvslab, block_size);
        for (size_t i=0, nalloc=0; i<nvslab->nblocks(); i++) {
            if (100*nalloc/nvslab->nblocks() < perc_full) {
                nvslab->set_alloc(i);
                nalloc++;
            } else {
                break;
            }
        }
        return nvslab;
    }

};

TEST_F(SlabHeapTest, insert)
{
    SlabHeap slabheap(NULL);

    Slab* slab0 = slabheap.insert_slab(alloc_nvslab(71, 0));
    Slab* slab1 = slabheap.insert_slab(alloc_nvslab(71, 1));
    Slab* slab2 = slabheap.insert_slab(alloc_nvslab(71, 50));
    Slab* slab3 = slabheap.insert_slab(alloc_nvslab(71, 99));

    UNUSED_ND(slab0);
    UNUSED_ND(slab1);
    UNUSED_ND(slab2);

    Slab* slab4 = slabheap.find_slab(71);
    
    EXPECT_EQ(slab3, slab4);
    
    std::cout << slabheap << std::endl; 

    slabheap.alloc_block(slab4);
    slabheap.alloc_block(slab4);
    slabheap.alloc_block(slab4);
    
    std::cout << slabheap << std::endl; 
} 


int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
