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

#ifndef _ALPS_LAYERS_SIZE_CLASS_HH_
#define _ALPS_LAYERS_SIZE_CLASS_HH_

#include <sys/types.h>

namespace alps {

const int kSizeClasses = 116;

extern size_t size_table[kSizeClasses];

inline int sizeclass(size_t sz)
{
    int sizeclass = 0;
    while (size_table[sizeclass] < sz) {
        sizeclass++;
    }
    return sizeclass;
}

inline size_t size_from_class(const int sizeclass)
{
    return size_table[sizeclass];
}

} // namespace alps

#endif // _ALPS_LAYERS_SIZE_CLASS_HH_
