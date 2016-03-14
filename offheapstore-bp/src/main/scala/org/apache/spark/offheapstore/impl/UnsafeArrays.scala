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
 *
 */

package org.apache.spark.offheapstore.impl

import org.apache.spark.unsafe.PlatformDependent

class UnsafeArrays() extends UnsafeArray {


  def alloc(memsize: Int): Unit = {
    this.memsize = memsize
    addr = PlatformDependent.UNSAFE.allocateMemory(memsize)
  }
  def set (pos: Int, arr: UnsafeArray) = {
    PlatformDependent.UNSAFE.putInt(null, addr + pos, arr.size)
    PlatformDependent.UNSAFE.putInt(null, addr + pos + 4L, arr.count)
    PlatformDependent.copyMemory(null, arr.addr, null, addr + pos + 8L, arr.memsize)
  }
  def getSize (pos: Int) = {
    PlatformDependent.UNSAFE.getInt(null, addr + pos)
  }
  def getCount (pos: Int) = {
    PlatformDependent.UNSAFE.getInt(null, addr + pos + 4L)
  }
  def getAddr (pos: Int): Long = {
    addr + pos + HEADER_SIZE
  }


}
