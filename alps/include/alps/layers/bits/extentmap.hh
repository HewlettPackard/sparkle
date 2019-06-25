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

#ifndef _ALPS_LAYERS_BITS_EXTENTMAP_HH_
#define _ALPS_LAYERS_BITS_EXTENTMAP_HH_

#include <stddef.h>
#include <stdint.h>

#include <iostream>
#include <map>

#include "alps/layers/bits/extentinterval.hh"

namespace alps {

class ExtentMap {
public:
    typedef size_t MapAddrKey;
    typedef ExtentInterval* MapAddrVal;
    typedef std::map<MapAddrKey, MapAddrVal> MapAddr;
    typedef std::pair<MapAddrKey, MapAddrVal> MapAddrKeyVal;

    // MapLenKey pair: <length, start address>
    typedef std::pair<size_t, size_t> MapLenKey; 
    typedef ExtentInterval* MapLenVal;
    typedef std::map<MapLenKey, MapLenVal> MapLen;
    typedef std::pair<MapLenKey, MapLenVal> MapLenKeyVal;
    
public:
    /**
     * @brief Insert an extent inteval
     */
    void insert(const ExtentInterval& extentinterval);

    /** 
     * @brief Find an extent of at least length len (greater or equal to len) 
     *        that has the lowest address
     *
     * @details
     * Previous work on phkmalloc has shown the prefering low addresses during reuse 
     * leads to predictable low fragmentation. This is because it helps keeping the 
     * brk limit as low as possible, which in turn helps minimizing the heap size.
     * Need to evaluate whether this policy is useful with a zoned-based persistent heap.
     */
    int find_ge(size_t len, ExtentInterval* nex);

    /** 
     * @brief Remove an extent of at least length len (greater or equal to len) 
     *        that has the lowest address
     */
    int remove_ge(size_t len, ExtentInterval* nex);

    /**
     * @brief Returns the number of extents indexed by this ExtentMap
     */
    size_t size() const { return map_addr_.size();  }

    /**
     * @brief Stream the list of extents ordered by start address to os 
     */
    void stream_to(std::ostream& os) const;

    /** 
     * @brief Stream the list of extents ordered by (length, start address) to os 
     */
    void stream_to_orderbylen(std::ostream& os) const;

    /** 
     * @brief Return a string representation of the extent index
     */
    std::string to_string() const {
        std::stringstream ss;
        stream_to(ss);
        return ss.str();
    }

    /**
     * @brief Checks whether two ExtentMap objects contain exactly the same extents 
     */
    bool operator==(const ExtentMap& other);

private:
    std::pair<MapAddr::iterator, ExtentInterval*> prev_addr(MapAddr::iterator hint, const ExtentInterval& e);
    std::pair<MapAddr::iterator, ExtentInterval*> prev_addr(const ExtentInterval& e);
    std::pair<MapAddr::iterator, ExtentInterval*> next_addr(MapAddr::iterator hint, const ExtentInterval& e);
    std::pair<MapAddr::iterator, ExtentInterval*> next_addr(const ExtentInterval& e);

    bool verify_maplen_equivalent_to_mapaddr();

protected:
    MapAddr map_addr_; // map of extents indexed by start address
    MapLen  map_len_;  // map of extents indexed by (length, start address)
};


inline std::ostream& operator<<(std::ostream& os, const ExtentMap& exmap)
{
    exmap.stream_to(os);
    return os;
}


} // namespace alps

#endif // _ALPS_LAYERS_BITS_EXTENTMAP_HH_
