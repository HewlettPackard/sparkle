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

#include "alps/layers/bits/extentmap.hh"

#include <stddef.h>
#include <stdint.h>

#include <map>

#include "alps/common/debug.hh"
#include "alps/common/error_code.hh"
#include "alps/layers/bits/extentinterval.hh"

namespace alps {

std::pair<ExtentMap::MapAddr::iterator, ExtentInterval*> ExtentMap::prev_addr(MapAddr::iterator hint, const ExtentInterval& e)
{
    ExtentInterval* ex_prev = NULL;
    MapAddr::iterator it_prev = hint;

    if (it_prev == map_addr_.begin()) {
        it_prev = map_addr_.end();
        ex_prev = NULL;
    } else if (map_addr_.size() > 0) {
        if (it_prev == map_addr_.end()) {
            it_prev--;
            ex_prev = it_prev->second;
        } else {
            ex_prev = it_prev->second;
            if (ex_prev->start() >= e.start()) {
                it_prev--;
                ex_prev = it_prev->second;
                assert(ex_prev->start() <= e.start());
            }
        }
    }
    return std::pair<MapAddr::iterator, ExtentInterval*>(it_prev, ex_prev);
}

std::pair<ExtentMap::MapAddr::iterator, ExtentInterval*> ExtentMap::prev_addr(const ExtentInterval& e)
{
    MapAddr::iterator hint = map_addr_.lower_bound(e.start());
    return prev_addr(hint, e);
}

std::pair<ExtentMap::MapAddr::iterator, ExtentInterval*> ExtentMap::next_addr(MapAddr::iterator hint, const ExtentInterval& e)
{
    ExtentInterval* ex_next = NULL;
    MapAddr::iterator it_next = hint;

    if (it_next != map_addr_.end()) {
        ex_next = it_next->second;
        if (ex_next->start() <= e.start()) {
            it_next++;
            if (it_next != map_addr_.end()) {
                ex_next = it_next->second;
            } else {
                ex_next = NULL;
            }
        }
    }
    return std::make_pair(it_next, ex_next);
}

std::pair<ExtentMap::MapAddr::iterator, ExtentInterval*> ExtentMap::next_addr(const ExtentInterval& e)
{
    MapAddr::iterator hint = map_addr_.lower_bound(e.start());
    return next_addr(hint, e);
}

void ExtentMap::insert(const ExtentInterval& e) 
{
    LOG(info) << "INSERT: " << e;

    MapAddr::iterator it_hint = map_addr_.lower_bound(e.start());

    std::pair<MapAddr::iterator, ExtentInterval*> prev = prev_addr(it_hint, e);
    MapAddr::iterator it_prev = prev.first;
    ExtentInterval* ex_prev = prev.second;

    std::pair<MapAddr::iterator, ExtentInterval*> next = next_addr(it_hint, e);
    MapAddr::iterator it_next = next.first;
    ExtentInterval* ex_next = next.second;

    if (ex_prev) {
        LOG(info) << "EX_PREV: " << *ex_prev;
    }
    if (ex_next) {
        LOG(info) << "EX_NEXT: " << *ex_next;
    }

    // We check for overlap with neighboring extents (prev and next) and 
    // merge with overlapped extents. We update indexes MapAddr and MapLen 
    // by removing merged extents and inserting a new single extent.
    //
    // We rely on the following important property:
    //   Map has the important property that inserting a new element into 
    //   a map does not invalidate iterators that point to existing 
    //   elements. Erasing an element from a map also does not invalidate 
    //   any iterators, except, of course, for iterators that actually point 
    //   to the element that is being erased. 
    //
    // TODO: Consider speeding up management of MapLen by keeping iterators
    // (in the extent) to the key-val pairs to removed on merge.
    bool merged = false;
    if (ex_prev && ex_next) {
        if (e.overlaps(ex_prev) && e.overlaps(ex_next)) {
            map_len_.erase(MapLenKey(ex_prev->len(), ex_prev->start()));
            map_len_.erase(MapLenKey(ex_next->len(), ex_next->start()));

            ex_prev->merge(&e);
            ex_prev->merge(ex_next);
            map_addr_.erase(it_next);
            delete(ex_next);
            merged = true;
        
            map_len_.insert(MapLenKeyVal(MapLenKey(ex_prev->len(), ex_prev->start()), ex_prev));
        } else if (e.overlaps(ex_prev)) {
            map_len_.erase(MapLenKey(ex_prev->len(), ex_prev->start()));

            ex_prev->merge(&e);
            merged = true;

            map_len_.insert(MapLenKeyVal(MapLenKey(ex_prev->len(), ex_prev->start()), ex_prev));
        } else if (e.overlaps(ex_next)) {
            map_len_.erase(MapLenKey(ex_next->len(), ex_next->start()));

            ex_next->merge(&e);
            map_addr_.erase(it_next);
            map_addr_.insert(MapAddrKeyVal(ex_next->start(), ex_next));
            merged = true;

            map_len_.insert(MapLenKeyVal(MapLenKey(ex_next->len(), ex_next->start()), ex_next));
        }
    } else if (ex_prev) {
        if (ex_prev->overlaps(&e)) {
            map_len_.erase(MapLenKey(ex_prev->len(), ex_prev->start()));

            ex_prev->merge(&e);
            merged = true;

            map_len_.insert(MapLenKeyVal(MapLenKey(ex_prev->len(), ex_prev->start()), ex_prev));
        }
    } else if (ex_next) {
        if (e.overlaps(ex_next)) {
            map_len_.erase(MapLenKey(ex_next->len(), ex_next->start()));

            ex_next->merge(&e);
            map_addr_.erase(it_next);
            map_addr_.insert(MapAddrKeyVal(ex_next->start(), ex_next));
            merged = true;

            map_len_.insert(MapLenKeyVal(MapLenKey(ex_next->len(), ex_next->start()), ex_next));
        }
    }
    if (!merged) {
        ExtentInterval* ex = new ExtentInterval(e.start(), e.len());
        map_addr_.insert(MapAddrKeyVal(ex->start(), ex));
        map_len_.insert(MapLenKeyVal(MapLenKey(ex->len(), ex->start()), ex));
    }

    assert(verify_maplen_equivalent_to_mapaddr() == true);
}

int ExtentMap::find_ge(size_t len, ExtentInterval* nex)
{
    if (len < 1) {
        return -1;
    }
    MapLen::iterator it = map_len_.lower_bound(MapLenKey(len, 0));
    if (it == map_len_.end()) {
        return -1;
    }
    ExtentInterval* ex = it->second;
    *nex = *ex;
    return 0;
}

int ExtentMap::remove_ge(size_t len, ExtentInterval* nex)
{
    MapLen::iterator it = map_len_.lower_bound(MapLenKey(len, 0));
    if (it == map_len_.end()) {
        return -1;
    }
    ExtentInterval* ex = it->second;

    MapAddr::iterator ita = map_addr_.find(ex->start());
    assert(ita != map_addr_.end());
    map_len_.erase(it);
    map_addr_.erase(ita);
    *nex = *ex;
    delete ex;
    return 0;
}

void ExtentMap::stream_to(std::ostream& os) const
{
    for (MapAddr::const_iterator it = map_addr_.begin(); 
         it != map_addr_.end(); it++) 
    {
        ExtentInterval* ex = it->second;
        os << *ex << std::endl;
    }
}

void ExtentMap::stream_to_orderbylen(std::ostream& os) const
{
    for (MapLen::const_iterator it = map_len_.begin(); 
         it != map_len_.end(); it++) 
    {
        ExtentInterval* ex = it->second;
        os << *ex << std::endl;
    }
}

bool ExtentMap::operator==(const ExtentMap& other)
{
    if (size() != other.size()) {
        return false;
    }
    for (MapAddr::const_iterator it = map_addr_.begin(),
         it_other = other.map_addr_.begin();
         it != map_addr_.end() && it_other != other.map_addr_.end();
         it++, it_other++) 
    {
        ExtentInterval* ex = it->second;
        ExtentInterval* ex_other = it->second;
        if (*ex != *ex_other) {
            return false;
        }
    }
    return true;
}


// The two indexes mapaddr and maplen must contain references to 
// the same extents
bool ExtentMap::verify_maplen_equivalent_to_mapaddr()
{
    if (map_addr_.size() != map_len_.size()) {
        return false;
    }
    for (MapLen::const_iterator it = map_len_.begin(); 
         it != map_len_.end(); it++) 
    {
        ExtentInterval* ex = it->second;
        MapAddr::const_iterator ita = map_addr_.find(ex->start());
        if (ita == map_addr_.end()) {
            return false;
        }
    }
    return true;
}

} // namespace alps
