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

#include "alps/common/externalizable.hh"

#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>

#include "yaml-cpp/yaml.h"
#include "alps/common/assorted_func.hh"

#include "common/os.hh"

namespace alps {

ErrorStack Externalizable::load_from_string(const std::string& yaml, bool ignore_missing) {
    try {
        YAML::Node node = YAML::Load(yaml);
        CHECK_ERROR(load(&node, ignore_missing));
    } catch (std::exception& e) {
        std::stringstream custom_message;
        custom_message << "problematic yaml=" << yaml;
        return ERROR_STACK_MSG(kErrorCodeConfParseFailed, custom_message.str().c_str());
    }
    return kRetOk;
}

void Externalizable::save_to_stream(std::ostream* ptr) const {
    std::ostream &o = *ptr;
    YAML::Emitter out;
    out << YAML::BeginMap;
    ErrorStack error_stack = save(&out);
    out << YAML::EndMap;
    if (error_stack.is_error()) {
        o << "Failed during Externalizable::save_to_stream(): " << error_stack;
        return;
    }
    o << out.c_str();
}

ErrorStack Externalizable::load_from_file(const fs::path& path, bool ignore_missing) {
    if (!fs::exists(path)) {
        return ERROR_STACK_MSG(kErrorCodeConfFileNotFound, path.c_str());
    }

    try {
        YAML::Node node = YAML::LoadFile(path.c_str());
        CHECK_ERROR(load(&node, ignore_missing));
    } catch (std::exception& e) {
        std::stringstream custom_message;
        custom_message << "problematic file=" << path;
        return ERROR_STACK_MSG(kErrorCodeConfParseFailed, custom_message.str().c_str());
    }
    return kRetOk;
}

ErrorStack Externalizable::save_to_file(const fs::path& path) const {
    // construct the YAML in memory
    YAML::Emitter out;
    out << YAML::BeginMap;
    ErrorStack error_stack = save(&out);
    out << YAML::EndMap;
    if (error_stack.is_error()) {
        std::stringstream custom_message;
        custom_message << "Failed during Externalizable::save()" << error_stack;
        return ERROR_STACK_MSG(kErrorCodeConfCouldNotWrite, custom_message.str().c_str());
    }

    fs::path folder = path.parent_path();
    // create the folder if not exists
    if (!fs::exists(folder)) {
        if (!fs_create_directories(folder, true)) {
            std::stringstream custom_message;
            custom_message << "file=" << path << ", folder=" << folder
                << ", err=" << os_error();
            return ERROR_STACK_MSG(kErrorCodeConfMkdirsFailed, custom_message.str().c_str());
        }
    }

    // To atomically save a file, we write to a temporary file and call sync, then use POSIX rename.
    fs::path tmp_path(path);
    tmp_path += ".tmp_";
    tmp_path += fs_unique_name("%%%%%%%%");

    try {
        std::ofstream yaml_out;
        yaml_out.open(tmp_path.c_str());
        yaml_out << out.c_str();
        yaml_out.close();
    } catch (std::exception& e) {
        std::stringstream custom_message;
        custom_message << "problematic file=" << path;
        return ERROR_STACK_MSG(kErrorCodeConfCouldNotWrite, custom_message.str().c_str());
    }

    if (!fs_durable_atomic_rename(tmp_path, path)) {
        std::stringstream custom_message;
        custom_message << "dest file=" << path << ", src file=" << tmp_path
            << ", err=" << os_error();
        return ERROR_STACK_MSG(kErrorCodeConfCouldNotRename, custom_message.str().c_str());
    }
    return kRetOk;
}

ErrorStack Externalizable::load_from_command_options(int argc, char* argv[]) 
{
    try {
        namespace po = boost::program_options; 
        po::options_description desc("Options"); 
        CommandOptionList cmdlist;
        CHECK_ERROR(add_command_options(&cmdlist));

        desc.add_options()("help", "Print help messages");

        for (CommandOptionList::iterator it = cmdlist.begin();
             it != cmdlist.end();
             it++) 
        {
            (*it)->add_option(desc);
        }

        po::variables_map vm; 

        try 
        { 
            po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc) 
                                  .allow_unregistered().run(); 
            po::store(parsed, vm);

            if (vm.count("help")) { 
                std::cout << "Usage: util [options] command" << std::endl 
                          << desc << std::endl;
                return kRetOk; 
            } 
 
            po::notify(vm); // throws on error, so do after help in case 
                            // there are any problems 

            for (CommandOptionList::iterator it = cmdlist.begin();
                 it != cmdlist.end();
                 it++) 
            {
                CommandOption* cmdopt = *it;
                if (vm.count(cmdopt->argname())) {
                    cmdopt->set_value(vm[cmdopt->argname()]);
                }
            }
        }
        catch(po::error& e) 
        { 
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
            std::cerr << desc << std::endl; 
            return kRetOk; 
        } 

    } catch (std::exception& e) {
        std::stringstream custom_message;
        custom_message << "problematic command line options";
        return ERROR_STACK_MSG(kErrorCodeConfParseFailed, custom_message.str().c_str());
    }
    return kRetOk;
}


ErrorStack insert_comment_impl(YAML::Emitter* out, const std::string& comment) {
    if (comment.size() > 0) {
        (*out) << YAML::Comment(comment);
    }
    return kRetOk;
}


ErrorStack Externalizable::insert_comment(YAML::Emitter* out,
                      const std::string& comment) 
{
    return insert_comment_impl(out, comment);
}


ErrorStack Externalizable::append_comment(YAML::Emitter* out,
                      const std::string& comment) 
{
    if (comment.size() > 0) {
        (*out) << YAML::Comment(comment);
    }
    return kRetOk;
}


template <typename T>
ErrorStack Externalizable::add_element(YAML::Emitter* out,
                const std::string& tag, const std::string& comment, T value, bool seq) 
{
    (*out) << YAML::Key << tag;
    (*out) << YAML::Value << value;
    if (comment.size() > 0) {
        CHECK_ERROR(insert_comment_impl(out,
            tag + " (type=" + get_pretty_type_name<T>() + "): " + comment));
    }
    return kRetOk;
}


// Explicit instantiations for each type
// @cond DOXYGEN_IGNORE
#define EXPLICIT_INSTANTIATION_ADD(x) template ErrorStack Externalizable::add_element< x > \
  (YAML::Emitter* emitter, const std::string& tag, const std::string& comment, x value, bool seq)
INSTANTIATE_ALL_TYPES(EXPLICIT_INSTANTIATION_ADD);
// @endcond


template <typename T>
ErrorStack Externalizable::add_element(YAML::Emitter* out, const std::string& tag,
        const std::string& comment, const std::vector< T >& value, bool seq) 
{
    *out << YAML::Key << tag;
    if (comment.size() > 0) {
      CHECK_ERROR(append_comment(out,
        tag + " (type=" + get_pretty_type_name< std::vector< T > >()
          + "): " + comment));
    }
    *out << YAML::Value << value;
    return kRetOk;
}

// Explicit instantiations for each type
// @cond DOXYGEN_IGNORE
#define EXPLICIT_INSTANTIATION_ADD_VECTOR(x) template ErrorStack Externalizable::add_element< x > \
  (YAML::Emitter* emitter, const std::string& tag, const std::string& comment, const std::vector<x>& value, bool seq)
INSTANTIATE_ALL_TYPES(EXPLICIT_INSTANTIATION_ADD_VECTOR);
// @endcond




#if 0
ErrorStack Externalizable::add_child_element(tinyxml2::XMLElement* parent, const std::string& tag,
                     const std::string& comment, const Externalizable& child) {
  tinyxml2::XMLElement* element = parent->GetDocument()->NewElement(tag.c_str());
  CHECK_OUTOFMEMORY(element);
  parent->InsertEndChild(element);
  CHECK_ERROR(insert_comment_impl(element, comment));
  CHECK_ERROR(child.save(element));
  return kRetOk;
}

#endif

template <typename T>
ErrorStack Externalizable::get_element(YAML::Node* parent, const std::string& tag,
                      T* out, bool ignore_missing, bool optional, T default_value) 
{
    if ((*parent)[tag]) {
        *out = (*parent)[tag].as<T>();
        return kRetOk;
    } else if (optional) {
        *out = default_value;
        return kRetOk;
    }
    if (ignore_missing) {
        return kRetOk;
    }
    return ERROR_STACK_MSG(kErrorCodeConfMissingElement, tag.c_str());
}

// Explicit instantiations for each type
// @cond DOXYGEN_IGNORE
#define EXPLICIT_INSTANTIATION_GET(x) template ErrorStack Externalizable::get_element< x > \
  (YAML::Node* parent, const std::string& tag, x * out, bool ignore_missing, bool optional, x default_value)
INSTANTIATE_ALL_TYPES(EXPLICIT_INSTANTIATION_GET);
// @endcond

ErrorStack Externalizable::get_element(YAML::Node* parent, const std::string& tag,
                  std::string* out, bool ignore_missing, bool optional, const char* default_value) {
    return get_element<std::string>(parent, tag, out, ignore_missing, optional, std::string(default_value));
}

template <typename T>
ErrorStack Externalizable::get_element(YAML::Node* parent, const std::string& tag,
                      std::vector<T> * out, bool ignore_missing, bool optional) 
{
  out->clear();
  if ((*parent)[tag]) {
    YAML::Node vec_node = (*parent)[tag];
    for (YAML::const_iterator it=vec_node.begin();it!=vec_node.end();++it) {
      T tmp = it->as<T>();
      out->push_back(tmp);
    }
  }
  if (out->size() == 0 && !optional && !ignore_missing) {
    return ERROR_STACK_MSG(kErrorCodeConfMissingElement, tag.c_str());
  }
  return kRetOk;
}

// Explicit instantiations for each type
// @cond DOXYGEN_IGNORE
#define EXPLICIT_INSTANTIATION_GET_VECTOR(x) template ErrorStack Externalizable::get_element< x > \
  (YAML::Node* parent, const std::string& tag, std::vector< x > * out, bool ignore_missing, bool optional)
INSTANTIATE_ALL_TYPES(EXPLICIT_INSTANTIATION_GET_VECTOR);
// @endcond

ErrorStack Externalizable::get_child_element(YAML::Node* parent, const std::string& tag,
                     Externalizable* child, bool ignore_missing, bool optional) 
{
  YAML::Node element = (*parent)[tag];
  if (element) {
    return child->load(&element);
  } else {
    if (optional) {
      return kRetOk;
    } 
    else if (ignore_missing) {
      return kRetOk;
    }  
    else {
      return ERROR_STACK_MSG(kErrorCodeConfMissingElement, tag.c_str());
    }
  }
}

}  // namespace alps
