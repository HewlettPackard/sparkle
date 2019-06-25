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

#include "sizeclass.hh"

namespace alps {

size_t size_table[kSizeClasses] = {8LLU, 16LLU, 24LLU, 32LLU, 40LLU, 48LLU, 56LLU, 64LLU, 72LLU, 80LLU, 88LLU, 96LLU, 104LLU, 112LLU, 120LLU, 128LLU, 136LLU, 144LLU, 152LLU, 160LLU, 168LLU, 176LLU, 184LLU, 192LLU, 200LLU, 208LLU, 216LLU, 224LLU, 232LLU, 240LLU, 248LLU, 256LLU, 264LLU, 272LLU, 280LLU, 288LLU, 296LLU, 304LLU, 312LLU, 320LLU, 328LLU, 336LLU, 344LLU, 352LLU, 360LLU, 368LLU, 376LLU, 384LLU, 392LLU, 400LLU, 408LLU, 416LLU, 424LLU, 432LLU, 440LLU, 448LLU, 456LLU, 464LLU, 472LLU, 480LLU, 488LLU, 496LLU, 504LLU, 512LLU, 576LLU, 640LLU, 704LLU, 768LLU, 832LLU, 896LLU, 960LLU, 1024LLU, 1536LLU, 2048LLU, 2560LLU, 3072LLU, 3584LLU, 4096LLU, 4608LLU, 5120LLU, 5632LLU, 6144LLU, 6656LLU, 7168LLU, 7680LLU, 8192LLU, 9216LLU, 10240LLU, 11264LLU, 12288LLU, 13312LLU, 14336LLU, 15360LLU, 16384LLU, 18432LLU, 20480LLU, 22528LLU, 24576LLU, 26624LLU, 28672LLU, 30720LLU, 16384LLU, 32768LLU, 49152LLU, 65536LLU, 81920LLU, 98304LLU, 114688LLU, 131072LLU, 147456LLU, 163840LLU, 180224LLU, 196608LLU, 212992LLU, 229376LLU, 245760LLU};

} // namespace alps
