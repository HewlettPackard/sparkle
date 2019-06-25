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

#ifndef _ALPS_TEST_LAYERS_COMMON_HH_
#define _ALPS_TEST_LAYERS_COMMON_HH_

namespace alps {

class Context {
public:
    Context()
        : do_v(true),
          do_nv(true)
    { }

    void load(uint8_t* src, uint8_t *dest, size_t size)
    {
        memcpy(dest, src, size);    
    }

    void store(uint8_t* src, uint8_t *dest, size_t size)
    {
        memcpy(dest, src, size);    
    }

    bool do_v;
    bool do_nv;
};



} // namespace alps

#endif // _ALPS_TEST_LAYERS_COMMON_HH_
