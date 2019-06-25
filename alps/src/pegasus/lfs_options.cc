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

#include "alps/pegasus/lfs_options.hh"

namespace alps {

ErrorStack LfsOptions::load(YAML::Node* ymlnode, bool ignore_missing) {
    EXTERNALIZE_LOAD_ELEMENT(ymlnode, ignore_missing, node);
    EXTERNALIZE_LOAD_ELEMENT(ymlnode, ignore_missing, node_count);
    EXTERNALIZE_LOAD_SIZE_ELEMENT(ymlnode, ignore_missing, book_size_bytes);
    return kRetOk;
}

ErrorStack LfsOptions::save(YAML::Emitter* out) const {
    return kRetOk;
}

ErrorStack LfsOptions::add_command_options(CommandOptionList* cmdopt) {
    return kRetOk;
};

} // namespace alps
