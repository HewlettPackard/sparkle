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

#include <math.h>
#include <assert.h>

#include <algorithm>

#include "common/debug.hh"
#include "globalheap/slab.hh"
#include "globalheap/slab_heap.hh"

namespace alps {

void Slab::init()
{   
    pthread_mutex_init(&pin_mutex_, NULL);
    if (block_size()) {
        free_list_.clear();
        for (size_t i=0; i<nblocks(); i++) {
            if (nvslab_->is_free(i)) {
                free_list_.push_back(i);
            }
        }
    }
}

void Slab::init(int sizeclass)
{
    nvSlab::make(nvslab_, sizeclass);
    init();
}

RRegion::TPtr<void> Slab::alloc_block() 
{
    RRegion::TPtr<void> ptr;

    if (!free_list_.empty()) {
        int bid = free_list_.front();
        free_list_.pop_front();
        nvslab_->set_alloc(bid);
        ptr = nvslab_->block(bid);
        LOG(info) << "Allocate block: " << "nvslab: " << nvslab_ << " block: " << bid;
    } else {
        LOG(info) << "Allocate block: FAILED: no free space";
        ptr = null_ptr;
    }
    return ptr;
}


void Slab::free_block(RRegion::TPtr<void> ptr)
{
    size_t bid = nvslab_->block_id(ptr);

    LOG(info) << "Free block: " << "nvslab: " << nvslab_ << " block: " << bid;
    assert(nvslab_->is_free(bid) == false);
    free_list_.push_front(bid);
    nvslab_->set_free(bid);
}

} // namespace alps
