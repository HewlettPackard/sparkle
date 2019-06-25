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

#ifndef _ALPS_LAYERS_BITS_EXTENT_INTERVAL_HH_
#define _ALPS_LAYERS_BITS_EXTENT_INTERVAL_HH_

#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <sstream>

namespace alps {

struct ExtentInterval {
public:
    ExtentInterval(size_t start, size_t len)
        : start_(start), 
          len_(len)
    { }

    ExtentInterval()
        : start_(0),
          len_(0)
    { }

    size_t start() const { return start_; }
    size_t len() const { return len_; }
    size_t end() const { return start_ + len_; }

    void merge(const ExtentInterval* ex);
    bool overlaps(const ExtentInterval* ex) const;

    bool operator==(const ExtentInterval& other) const
    {
        return start_ == other.start_ && len_ == other.len_;
    }

    bool operator!=(const ExtentInterval& other) const
    {
        return !(*this == other);
    }

    void stream_to(std::ostream& os) const {
        os << "(" << start() << ", " << len() << ")";
    }

    std::string to_string() const {
        std::stringstream ss;
        stream_to(ss);
        return ss.str();
    }

private:
    size_t start_;
    size_t len_;
};

inline void ExtentInterval::merge(const ExtentInterval* ex) {
    if (start() <= ex->start() && ex->start() <= end()) {
        if (end() <= ex->end()) {
            // CASE: partially overlap with prev
            // [------)
            //    [--------)
            len_ += (ex->end() - end());
        }
    } else if (ex->start() <= start() && start() <= ex->end()) {
        if (ex->end() <= end()) {
            // CASE: partially overlap with next
            //    [--------)
            // [-----)
            len_ += start() - ex->start();
            start_ = ex->start();
        } else {
            // CASE: completely overlap with next
            //    [--------)
            // [-------------)
            start_ = ex->start();
            len_ = ex->len();
        }
    } 
}

inline bool ExtentInterval::overlaps(const ExtentInterval* ex) const {
    if (start() <= ex->start() && ex->start() <= end()) {
        // [------)
        //    [--------)
        // 
        // [------)
        //    [--)
        return true;
    } else if (ex->start() <= start() && start() <= ex->end()) {
        //    [--------)
        // [-----)
        // 
        //    [--------)
        // [-------------)
        return true;
    } 
    return false;
}


inline std::ostream& operator<<(std::ostream& os, const ExtentInterval& ex)
{
    ex.stream_to(os);
    return os;
}


} // namespace alps

#endif // _ALPS_LAYERS_BITS_EXTENT_INTERVAL_HH_
