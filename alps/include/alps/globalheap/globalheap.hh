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

#ifndef _ALPS_GLOBALHEAP_GLOBALHEAP_HH_
#define _ALPS_GLOBALHEAP_GLOBALHEAP_HH_

#include <iostream>
#include <pthread.h>
#include <vector>
#include <mutex>

#include "../pegasus/pegasus.hh"
#include "../pegasus/relocatable_region.hh"
#include "memattrib.hh"

namespace alps {

// forward declarations
class GlobalHeapInternal;


/**
 * @brief Global Symmetric Heap public interface
 */
class GlobalHeap
{
public:
    typedef uint64_t InstanceId;

public:
    /**
     * @brief Creates a global symmetric persistent heap of size @a size.
     *
     * @details
     * The space for the heap is allocated immediately. Its initial
     * contents are all zeros.
     *
     * @param path The pathname to the region file backing the heap.
     * @param heap_size The total size of the heap.
     * @param metazone_size The size of each zone.
     * @param[out] heap A pointer to a handle referencing the heap.
     * @return Zero if success, or error code otherwise.
     */
    static int create(const char* path, size_t heap_size, size_t metazone_size, GlobalHeap** heap);

    /**
     * @brief Creates a global symmetric persistent heap of size @a size
     * partitioned in @a npaths files.
     *
     * @details
     * The space for the heap is allocated immediately. Its initial
     * contents are all zeros.
     *
     * @param paths An array of pathnames to the region files backing the heap.
     * @param npaths The number of pathnames.
     * @param heap_size The total size of the heap.
     * @param metazone_size The size of each zone.
     * @param[out] heap A pointer to a handle referencing the heap.
     * @return Zero if success, or error code otherwise.
     */
    static int create(const char** paths, int npaths, size_t heap_size, size_t metazone_size, GlobalHeap** heap);

    /**
     * @brief Opens a global symmetric persistent heap and returns a handle 
     * referencing the heap.
     *
     * @param path The pathname to the region file backing the heap.
     * @param[out] heap A pointer to a handle referencing the heap.
     * @return Zero if success, or error code otherwise.
     */
    static int open(const char* path, GlobalHeap** heap);

    /**
     * @brief Opens a global symmetric persistent heap comprising multiple 
     * files \a paths and returns a handle 
     *
     * @param[in] paths The pathnames to the region files backing the heap.
     * @param[in] npaths The number of region files backing the heap.
     * @param[out] heap A pointer to a handle referencing the heap.
     * @return Zero if success, or error code otherwise.
     */
    static int open(const char** paths, int npaths, GlobalHeap** heap);

    /**
     * @brief Formats the global symmetric persistent heap.
     *
     * Formatting a heap essentially zeros all heap metadata.
     * 
     * @param[in] path The pathname to the region file backing the heap.
     * @return Zero if success, or error code otherwise.
     */
    static int format(const char* path);

    /**
     * @brief Formats the global symmetric persistent heap comprising
     * multiple files.
     *
     * Formatting a heap essentially zeros all heap metadata.
     * 
     * @param[in] paths The pathnames to the region files backing the heap.
     * @param[in] npaths The number of region files backing the heap.
     * @return Zero if success, or error code otherwise.
     */
    static int format(const char** paths, int npaths);

    /**
     * @brief Formats zones owned by heap instance @a instance_id.
     *
     * @param[in] path The pathname to the region file backing the heap.
     * @param[in] instance_id Persistent identifier identifying heap instance.
     * @return Zero if success, or error code otherwise.
     */
    static int format_instance(const char* path, InstanceId instance_id);

    /**
     * @brief Formats zones owned by heap instance @a instance_id.
     *
     * @param[in] paths The pathnames to the region files backing the heap.
     * @param[in] npaths The number of region files backing the heap.
     * @param[in] instance_id Persistent identifier identifying heap instance.
     * @return Zero if success, or error code otherwise.
     */
    static int format_instance(const char** paths, int npaths, InstanceId instance_id);

    /**
     * @brief Returns a persistent identifier identifying the current heap 
     * instance.
     */
    InstanceId instance();

    /**
     * @brief Returns the heap root pointer.
     */
    template<typename RootT>
    RRegion::TPtr<RootT> root();

    /**
     * @brief Sets the heap root pointer.
     */
    template<typename RootT>
    void set_root(RRegion::TPtr<RootT> root);

    /**
     * @brief Allocates @a size bytes and returns a pointer handle to 
     * the allocated memory.
     *
     * @param size The number of bytes to allocate.
     * @return A pointer handle to the allocated memory on success, 
     * or the null pointer handle on error.
     */
    RRegion::TPtr<void> malloc(size_t size);

    /**
     * @brief Allocates @a size bytes associated with memory attribytes 
     * @a memattrib and returns a pointer handle to the allocated memory.
     *
     * @param size The number of bytes to allocate.
     * @param memattrib The memory attributes of the allocated memory.
     * @return A pointer handle to the allocated memory on success, 
     * or the null pointer handle on error.
     */
    RRegion::TPtr<void> malloc(size_t size, const MemAttrib& memattrib);

    /**
     * @brief Frees the memory space pointed to by the pointer handle @a ptr.
     *
     * @param ptr A pointer handle pointing to the memory space to free.
     *
     * @todo Currently, a free of a memory location owned by another process
     * will fail.
     */
    void free(RRegion::TPtr<void> ptr);

    /**
     * @brief Changes the size of the memory block pointed to by @a ptr 
     * to @a size bytes. 
     *
     * It maintains memory attributes. 
     *
     * @param ptr The pointer to the memory block to change size of.
     * @param size The number of bytes to allocate.
     * @return A pointer handle to the allocated memory on success, 
     * or the null pointer handle on error.
     */
    RRegion::TPtr<void> realloc(RRegion::TPtr<void> ptr, size_t size);

    /**
     * @brief Closes the heap.
     */
    int close();

    /**
     * @brief Returns total size of the heap
     */
    size_t size();

    /**
     * @brief Returns pointer to the descriptor of the region backing the heap
     */
    RRegion* region();

private:
    GlobalHeap(GlobalHeapInternal* globalheap_internal)
        : globalheap_internal_(globalheap_internal)
    { }

    RRegion::TPtr<void> __root();
    void __set_root(RRegion::TPtr<void> root);

private:

    GlobalHeapInternal* globalheap_internal_;
};


template<typename RootT>
RRegion::TPtr<RootT> GlobalHeap::root()
{
    return __root();
}

template<typename RootT>
void GlobalHeap::set_root(RRegion::TPtr<RootT> root)
{
    RRegion::TPtr<void> void_root = static_cast<RRegion::TPtr<void>>(root);
    __set_root(void_root);
}



} // namespace alps

#endif // _ALPS_GLOBALHEAP_GLOBALHEAP_HH_
