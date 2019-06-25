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

#ifndef _ALPS_LAYERS_BITS_BITMAP_HH_
#define _ALPS_LAYERS_BITS_BITMAP_HH_

#include <stdint.h>

template<typename Context>
struct nvBitMap {
    static const int kEntrySizeLog2 = 3;
    static const int kEntrySize = 1 << kEntrySizeLog2;

    uint8_t  bv_[0];

    static nvBitMap* make(Context& ctx, size_t length, void* ptr)
    {
        nvBitMap* bm = static_cast<nvBitMap*>(ptr);
        // we have at least one entry
        for (unsigned int i=0; i<size_of(length); i++) {
            uint8_t tmp = 0;
            ctx.store(&tmp, &bm->bv_[i], sizeof(bm->bv_[i]));
        }
        return bm;  
    } 

    static nvBitMap* load(void* ptr)
    {
        nvBitMap* bm = static_cast<nvBitMap*>(ptr);
        return bm;  
    } 

    static size_t size_of(size_t bitmap_len)
    {
        if (bitmap_len % nvBitMap::kEntrySize > 0) {
            return 1 + bitmap_len / kEntrySize;
        } 
        return bitmap_len / kEntrySize;
    }

    size_t elt(int bit_index) 
    {
        return bit_index >> kEntrySizeLog2;
    }

    uint8_t mask(int bit_index) 
    {
        return 1 << (bit_index & ((1 << kEntrySizeLog2) - 1));
    }

    void clear(Context& ctx, int bit_index) 
    {
        // bv_[elt(bit_index)] = bv_[elt(bit_index)] & ~mask(bit_index);
        uint8_t tmp;
        ctx.load(&bv_[elt(bit_index)], &tmp, sizeof(bv_[elt(bit_index)]));
        tmp = tmp & ~mask(bit_index);
        ctx.store(&tmp, &bv_[elt(bit_index)], sizeof(bv_[elt(bit_index)]));
    }

    void set(Context& ctx, int bit_index) 
    {
        // bv_[elt(bit_index)] = bv_[elt(bit_index)] | mask(bit_index);  
        uint8_t tmp;
        ctx.load(&bv_[elt(bit_index)], &tmp, sizeof(bv_[elt(bit_index)]));
        tmp = tmp | mask(bit_index);
        ctx.store(&tmp, &bv_[elt(bit_index)], sizeof(bv_[elt(bit_index)]));
    }

    bool is_set(Context& ctx, int bit_index) 
    {
        //return (bv_[elt(bit_index)] & mask(bit_index)) != 0;
        uint8_t tmp;
        ctx.load(&bv_[elt(bit_index)], &tmp, sizeof(bv_[elt(bit_index)]));
        return (tmp & mask(bit_index)) != 0;
    }
};

#endif // _ALPS_LAYERS_BITS_BITMAP_HH_
