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

#ifndef _ALPS_GLOBALHEAP_HELPER_HH_
#define _ALPS_GLOBALHEAP_HELPER_HH_

#include <vector>
#include <boost/filesystem.hpp>

#include "alps/common/error_code.hh"
#include "alps/pegasus/relocatable_region.hh"

#include "pegasus/helper_functions.hh"
#include "globalheap/extent.hh"
#include "globalheap/layout.hh"

namespace alps {

size_t find_extent(RRegion::TPtr<nvZone> nvzone, size_t start, size_t end, Extent* extent, bool* extent_is_free);
ErrorCode create_region(const std::vector<boost::filesystem::path>& pathnames, size_t region_size, size_t interleave_size);
ErrorCode map_region(const std::vector<boost::filesystem::path>& pathnames, RRegion** pregion);

} // namespace alps

#endif // _ALPS_GLOBALHEAP_HELPER_HH_
