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
#include <iostream>
#include <cinttypes>
#include <vector>
#include <thread>

#include <boost/interprocess/offset_ptr.hpp>
#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/continuous_interval.hpp>
#include <boost/icl/right_open_interval.hpp>
#include <boost/icl/left_open_interval.hpp>
#include <boost/icl/closed_interval.hpp>
#include <boost/icl/open_interval.hpp>
#include <boost/icl/rational.hpp>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/icl/separate_interval_set.hpp>

#include "gtest/gtest.h"
#include "alps/globalheap/globalheap.hh"

#include "globalheap/globalheap_internal.hh"
#include "test_common.hh"
#include "test_heap_fixture.hh"

using namespace alps;


// keep track of live allocated blocks
// to be able to randomly pick a live block, we maintain a vector of live blocks from which we draw a block 
// the position of the block in the vector is also stored in the interval tree so that we can locate the 
// block in the vector upon removal from the interval tree
class LiveBlocks
{
public:
    typedef boost::icl::interval_map<uint64_t, uint64_t>::iterator iterator;

public:
    void insert_block(uint64_t base, size_t sz)
    {
        size_t pos = vec_.size();
        vec_.push_back(base);
        map_ += std::make_pair(boost::icl::discrete_interval<uint64_t>(base, base + sz - 1, boost::icl::interval_bounds::closed()), pos+1);
    }

    void insert_block(void* base, size_t sz)
    {
        insert_block((uint64_t) base, sz);
    }

    int remove_block(uint64_t base)
    {
        // remove the interval entry corresponding to the block from both the interval map and the vector
        boost::icl::interval_map<uint64_t, uint64_t>::const_iterator iter = map_.find(base);
        if (iter == map_.end()) {
            return -1;
        }
        if (base != iter->first.lower()) {
            return -1;
        }
        uint64_t pos = iter->second;
        uint64_t sz = iter->first.upper() - iter->first.lower() + 1;
        map_ -= std::make_pair(boost::icl::discrete_interval<uint64_t>(base, base + sz - 1, boost::icl::interval_bounds::closed()), pos);
        assert(vec_[pos-1] == base);
        if (pos == vec_.size()) {
            vec_.pop_back();
        } else {
        // swap the last entry in the vector with the removed one and reset the 
        // interval map entry to point to the new position in the vector
        uint64_t last_base = vec_.back();
        iter = map_.find(last_base);
        base = iter->first.lower();
        sz = iter->first.upper() - iter->first.lower() + 1;
        map_.set(std::make_pair(boost::icl::discrete_interval<uint64_t>(base, base + sz - 1, boost::icl::interval_bounds::closed()), pos));
        vec_[pos-1] = base;
        vec_.pop_back();
        }
        return 0; 
    }

    int remove_block(void* base)
    {
        return remove_block((uint64_t) base);
    }

    uint64_t pick_block_random(unsigned int *seedp)
    {
        unsigned int idx = rand_r(seedp) % vec_.size();
        return vec_[idx];
    }

    bool overlaps(uint64_t base, uint64_t sz)
    {
        boost::icl::interval_map<uint64_t, uint64_t>::const_iterator iter = map_.find(boost::icl::discrete_interval<uint64_t>(base, base+sz-1, boost::icl::interval_bounds::closed()));
        if (iter == map_.end()) {
            return false;
        }
        return true;
    }

    uint64_t size()
    {
        return vec_.size();
    }

    iterator begin()
    {
        return map_.begin();
    }

    iterator end()
    {
        return map_.end();
    }

    void print()
    {
        for (boost::icl::interval_map<uint64_t, uint64_t>::iterator iter = map_.begin(); iter != map_.end(); iter++) {
            printf("%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n", iter->first.lower(), iter->first.upper(), iter->second);
        }

    }

private:
    boost::icl::interval_map<uint64_t, uint64_t> map_; // interval map of live blocks
    std::vector<uint64_t> vec_; // vector of live blocks that enables picking a block at random
};

class UniformDistribution
{
public:
    const int granularity = 1000; // 1000 sample points per block size
public:
    UniformDistribution(size_t min, size_t max, size_t incr)
    {
        for (size_t blk_sz = min; blk_sz<=max; blk_sz += incr)
        {
            for (int i=0; i<granularity; i++) {
                inverse_cdf_.push_back(blk_sz);
            }
        }
     
    }

    UniformDistribution(std::vector<size_t> block_sizes)
    {
        for (std::vector<size_t>::iterator iter = block_sizes.begin();
             iter != block_sizes.end();
             iter++)
        {
            for (int i=0; i<granularity; i++) {
                size_t blk_sz = *iter;
                inverse_cdf_.push_back(blk_sz);
            }
        }
    }

    size_t random(unsigned int *seedp)
    {
        unsigned int idx = rand_r(seedp) % inverse_cdf_.size();
        return inverse_cdf_[idx];
    }


private:
    std::vector<size_t> inverse_cdf_;
};


void write_pattern(void* ptr, size_t size, uint64_t pattern)
{
    for (size_t i=0; i<size/sizeof(pattern); i++) {
        static_cast<uint64_t*>(ptr)[i] = pattern;
    }
}


bool verify_pattern(void* ptr, size_t size, uint64_t pattern)
{
    for (size_t i=0; i<size/sizeof(pattern); i++) {
        if (static_cast<uint64_t*>(ptr)[i] != pattern) {
            std::cout << "EXPECTED: " << std::hex << uint64_t(pattern) << " " 
                      << "ACTUAL[" << std::dec << i << "]: " << std::hex 
                      << uint64_t(static_cast<uint64_t*>(ptr)[i]) << std::dec << std::endl;
            std::cout << "ptr==" << &static_cast<uint64_t*>(ptr)[i] << std::endl;
            std::cout << "tptr==" << RRegion::TPtr<uint64_t>(&static_cast<uint64_t*>(ptr)[i]) << std::endl;
            return false;
        }
    }
    return true;
}


void random_alloc_free(GlobalHeapInternal* heap, 
                       UniformDistribution block_dist,
                       unsigned int seed, 
                       unsigned int allocated_blocks_low_watermark, 
                       unsigned int allocated_blocks_high_watermark,
                       int nops,
                       int perc_alloc,
                       bool do_pattern_check, 
                       uint64_t pattern)
{
    LiveBlocks  live_blocks;
    unsigned int gseed = seed;
    unsigned int opseed = seed;

    // initialize with some allocations
    for (unsigned int i=0; i < allocated_blocks_low_watermark; i++)
    {
        size_t block_size = block_dist.random(&seed);
        RRegion::TPtr<void> ptr = heap->malloc(block_size);
        ASSERT_NE(null_ptr, ptr);
        ASSERT_EQ(0, live_blocks.overlaps(ptr.offset(), block_size));
        live_blocks.insert_block((uint64_t) ptr.get(), block_size);
        if (do_pattern_check) {
            write_pattern(ptr.get(), block_size, pattern);
        }
    }

    //live_blocks.print();

    // do a bunch of malloc and frees
    for (int i=0; i < nops; i++)
    {
        unsigned int op = rand_r(&opseed) % 100;

        bool do_alloc;

        if (op < 50) {
            // do alloc depends on the high water mark?
            if (live_blocks.size() < allocated_blocks_high_watermark) {
                do_alloc = true;
            } else {
                do_alloc = false;
            }
        } else {
            // do free depends on the low water mark
            if (live_blocks.size() < allocated_blocks_low_watermark) {
                do_alloc = true;
            } else {
                do_alloc = false;
            }
        } 

        if (do_alloc) {
            // malloc of random size
            size_t block_size = block_dist.random(&gseed);
            RRegion::TPtr<void> ptr = heap->malloc(block_size);
            ASSERT_NE(null_ptr, ptr);
            ASSERT_EQ(0, live_blocks.overlaps(ptr.offset(), block_size));
            live_blocks.insert_block((uint64_t) ptr.get(), block_size);
            if (do_pattern_check) {
                write_pattern(ptr.get(), block_size, pattern);
            }
        } else {
            // free
            uint64_t vaddr = live_blocks.pick_block_random(&gseed);
            RRegion::TPtr<void> ptr = RRegion::TPtr<void>((void*) vaddr);
            heap->free(ptr);
            live_blocks.remove_block(vaddr);
        }
    }

    if (do_pattern_check) {
        for (LiveBlocks::iterator iter = live_blocks.begin(); iter != live_blocks.end(); iter++) {
            uint64_t vaddr = iter->first.lower();
            RRegion::TPtr<void> ptr = RRegion::TPtr<void>((void*) vaddr);
            ASSERT_EQ(true, verify_pattern(ptr.get(), iter->first.upper() - iter->first.lower() + 1, pattern));
        }
    }

    // free all allocated blocks
    while (live_blocks.size() > 0) {
        uint64_t vaddr = live_blocks.pick_block_random(&gseed);
        live_blocks.remove_block(vaddr);
        RRegion::TPtr<void> ptr = RRegion::TPtr<void>((void *) vaddr);
        heap->free(ptr);
    }
}

////////////////////////////////////////////////////////////////////////////

class StressTest: public AutoGlobalHeapTest { 
public:

    void SetUp() {
        global_heap_size = 64*1024*1024LLU;
        AutoGlobalHeapTest::SetUp();
    }
};

class StressTestLarge: public AutoGlobalHeapTest { 
public:

    void SetUp() {
        global_heap_size = 2*1024*1024*1024LLU;
        AutoGlobalHeapTest::SetUp();
    }
};


TEST_F(StressTest, random_alloc_free1)
{
    UniformDistribution block_dist({65536, 98304});

    random_alloc_free(heap_, block_dist, 1, 64, 65, 128, 50, true, 0x1);
}

TEST_F(StressTest, random_alloc_free2)
{
    UniformDistribution block_dist({65536, 98304});

    random_alloc_free(heap_, block_dist, 1, 64, 96, 256, 50, true, 0x1);
}

TEST_F(StressTest, random_alloc_free_multithread1)
{
    UniformDistribution block_dist({65536, 98304});

    std::thread t0 = std::thread(random_alloc_free, heap_, block_dist, 1, 64, 96, 256, 50, true, 0x1);
    std::thread t1 = std::thread(random_alloc_free, heap_, block_dist, 1, 64, 96, 256, 50, true, 0x2);
    t0.join();
    t1.join();
}

TEST_F(StressTest, random_alloc_free_multithread2)
{
    UniformDistribution block_dist({4096});

    std::vector<std::thread> threads;
    for (int i=0; i<8; i++) {
        std::thread t(random_alloc_free, heap_, block_dist, 1, 64, 96, 256, 50, true, 0x1 << i);
        threads.push_back(std::move(t));
    }
    
    for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); it++) {
        it->join();
    }
}


TEST_F(StressTestLarge, random_alloc_free_multithread1)
{
    UniformDistribution block_dist({512*1024});

    std::vector<std::thread> threads;
    for (int i=0; i<32; i++) {
        std::thread t(random_alloc_free, heap_, block_dist, 1, 64, 65, 256, 50, true, 0x1 << i);
        threads.push_back(std::move(t));
    }
    
    for (std::vector<std::thread>::iterator it = threads.begin(); it != threads.end(); it++) {
        it->join();
    }
}


int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
