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

#include "alps/pegasus/pegasus_options.hh"

namespace alps {


// template-ing just for const/non-const
template <typename PARENT_OPTION_PTR, typename CHILD_PTR>
std::vector< CHILD_PTR > get_children_impl(PARENT_OPTION_PTR option) {
    std::vector< CHILD_PTR > children;
    children.push_back(&option->debug_options);
    children.push_back(&option->lfs_options);
    children.push_back(&option->tmpfs_options);
    return children;
}

std::vector<Externalizable* > get_children(PegasusOptions* option) {
    return get_children_impl<PegasusOptions*, Externalizable*>(option);
}

std::vector< const Externalizable* > get_children(const PegasusOptions* option) {
    return get_children_impl<const PegasusOptions*, const Externalizable*>(option);
}


ErrorStack PegasusOptions::load(YAML::Node* node, bool ignore_missing) {
    for (Externalizable* child : get_children(this)) {
        CHECK_ERROR(get_child_element(node, child->get_tag_name(), child, ignore_missing));
    }
    return kRetOk;
}


ErrorStack PegasusOptions::save(YAML::Emitter* out) const {
    return kRetOk;
}

ErrorStack PegasusOptions::add_command_options(CommandOptionList* cmdopt) {
    return kRetOk;
};

} // namespace alps
