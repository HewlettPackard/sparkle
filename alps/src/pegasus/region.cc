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

#include <sys/types.h>

#include "alps/common/error_code.hh"
#include "alps/pegasus/region.hh"

#include "pegasus/region_file.hh"
#include "pegasus/region_file_factory.hh"

namespace alps {


ErrorCode Region::set_interleave_group(loff_t offset, loff_t length, const std::vector<InterleaveGroup>& vig)
{
    return file_->set_interleave_group(offset, length, vig);
}


ErrorCode Region::interleave_group(loff_t offset, loff_t length, std::vector<InterleaveGroup>* vig)
{
    return file_->interleave_group(offset, length, vig);
}


} // namespace alps
