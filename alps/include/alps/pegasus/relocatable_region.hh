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

#ifndef _ALPS_PEGASUS_RELOCATABLE_REGION_HH_
#define _ALPS_PEGASUS_RELOCATABLE_REGION_HH_

#include "pointer.hh"
#include "region.hh"
#include "mappable.hh"
#include "bits/segmap.hh"

namespace alps {

using RRegion = Mappable<Region, SegmentMap, BaseRelativePointer>;

} // namespace alps

#endif // _ALPS_PEGASUS_RELOCATABLE_REGION_HH_
