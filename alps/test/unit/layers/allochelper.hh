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

#ifndef _ALPS_TEST_ALLOC_HELPER_HH_
#define _ALPS_TEST_ALLOC_HELPER_HH_

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

using namespace alps;


// Helper class that keeps track of blocks.
// To be able to randomly pick a block, we maintain a vector of blocks from which we draw a block. 
// The position of the block in the vector is also stored in the interval tree so that we can locate the 
// block in the vector upon removal from the tree.
class BlockSet
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
            printf("0x%" PRIx64 "\t0x%" PRIx64 "\t%" PRIu64 "\n", iter->first.lower(), iter->first.upper(), iter->second);
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
            std::cout << "tptr==" << TPtr<uint64_t>(&static_cast<uint64_t*>(ptr)[i]) << std::endl;
            return false;
        }
    }
    return true;
}

template<typename Heap>
void random_alloc_free(Heap* heap, 
                       UniformDistribution block_dist,
                       unsigned int seed, 
                       unsigned int allocated_blocks_low_watermark, 
                       unsigned int allocated_blocks_high_watermark,
                       int nops,
                       int perc_alloc,
                       bool do_pattern_check, 
                       uint64_t pattern)
{
    BlockSet  live_blocks;
    unsigned int gseed = seed;
    unsigned int opseed = seed;

    // initialize with some allocations
    for (unsigned int i=0; i < allocated_blocks_low_watermark; i++)
    {
        size_t block_size = block_dist.random(&seed);
        TPtr<void> ptr = heap->malloc(block_size);
        ASSERT_NE(null_ptr, ptr);
        ASSERT_EQ(0, live_blocks.overlaps((uint64_t)ptr.get(), block_size));
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
            // allocate if we do not pass the high water mark
            if (live_blocks.size() < allocated_blocks_high_watermark) {
                do_alloc = true;
            } else {
                do_alloc = false;
            }
        } else {
            // free if we do not drop below the low water mark
            if (live_blocks.size() < allocated_blocks_low_watermark) {
                do_alloc = true;
            } else {
                do_alloc = false;
            }
        } 

        if (do_alloc) {
            // malloc of random size
            size_t block_size = block_dist.random(&gseed);
            TPtr<void> ptr = heap->malloc(block_size);
            ASSERT_NE(null_ptr, ptr);
            ASSERT_EQ(0, live_blocks.overlaps((uint64_t) ptr.get(), block_size));
            live_blocks.insert_block((uint64_t) ptr.get(), block_size);
            if (do_pattern_check) {
                write_pattern(ptr.get(), block_size, pattern);
            }
        } else {
            // free
            uint64_t vaddr = live_blocks.pick_block_random(&gseed);
            TPtr<void> ptr = TPtr<void>((void*) vaddr);
            heap->free(ptr);
            live_blocks.remove_block(vaddr);
        }
    }

    if (do_pattern_check) {
        for (BlockSet::iterator iter = live_blocks.begin(); iter != live_blocks.end(); iter++) {
            uint64_t vaddr = iter->first.lower();
            TPtr<void> ptr = TPtr<void>((void*) vaddr);
            ASSERT_EQ(true, verify_pattern(ptr.get(), iter->first.upper() - iter->first.lower() + 1, pattern));
        }
    }

    // free all allocated blocks
    while (live_blocks.size() > 0) {
        uint64_t vaddr = live_blocks.pick_block_random(&gseed);
        live_blocks.remove_block(vaddr);
        TPtr<void> ptr = TPtr<void>((void *) vaddr);
        heap->free(ptr);
    }
}

#endif // _ALPS_TEST_ALLOC_HELPER_HH_
