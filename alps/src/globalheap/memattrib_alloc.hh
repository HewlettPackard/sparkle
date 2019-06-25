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

#ifndef _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_ALLOCATOR_HH_
#define _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_ALLOCATOR_HH_

#include <map>

#include "alps/globalheap/memattrib.hh"

#include "globalheap/rwlock.hh"

namespace alps {

template<typename AllocatorT>
class MemAttribAllocators
{
public:
    typedef typename std::map<MemAttrib, AllocatorT*>::iterator iterator;

public:
    MemAttribAllocators();
    AllocatorT* find(const MemAttrib& memattrib);
    void bind(const MemAttrib& memattrib, AllocatorT* allocator);

    iterator begin()
    {
        return allocators_.begin();
    }

    iterator end()
    {
        return allocators_.end();
    }

    void lock_read() { rwlock_.lock_read(); }
    void unlock_read() { rwlock_.unlock_read(); }
    void lock_write() { rwlock_.lock_write(); }
    void unlock_write() { rwlock_.unlock_write(); }

private:
    typedef std::map<MemAttrib, AllocatorT*> AllocatorIndex;
    typedef std::pair<MemAttrib, AllocatorT*> AllocatorIndexPair;

private:
    ReaderWriterLock rwlock_;
    AllocatorIndex   allocators_; // index of memory-attribute to associated allocator
    MemAttrib        last_used_memattrib_; // hint to last used memory attribute
    AllocatorT*      last_used_allocator_; // hint to last used allocator
};

template<typename AllocatorT>
MemAttribAllocators<AllocatorT>::MemAttribAllocators()
    : last_used_allocator_(NULL)
{ 

}

template<typename AllocatorT>
AllocatorT* MemAttribAllocators<AllocatorT>::find(const MemAttrib& memattrib)
{
    if (memattrib == last_used_memattrib_) {
        return last_used_allocator_;
    }

    typename AllocatorIndex::iterator it = allocators_.find(memattrib);
    if (it == allocators_.end()) {
        return NULL;
    }
    last_used_memattrib_ = memattrib;
    last_used_allocator_ = it->second;
    return last_used_allocator_;
}

template<typename AllocatorT>
void MemAttribAllocators<AllocatorT>::bind(const MemAttrib& memattrib, AllocatorT* allocator)
{
    allocators_.insert(AllocatorIndexPair(memattrib, allocator));

    last_used_memattrib_ = memattrib;
    last_used_allocator_ = allocator;
}




} // namespace alps

#endif // _ALPS_GLOBALHEAP_MEMORY_ATTRIBUTE_HH_
