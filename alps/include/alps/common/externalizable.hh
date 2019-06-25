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

#ifndef _ALPS_COMMON_EXTERNALIZABLE_HPP_
#define _ALPS_COMMON_EXTERNALIZABLE_HPP_

#include <stdint.h>

#include <iosfwd>
#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/program_options.hpp" 

#include "cxx11.hh"
#include "error_stack.hh"
#include "assorted_func.hh"

// yaml-cpp forward declarations
namespace YAML {
    class Node;
    class Emitter;
}

namespace fs = boost::filesystem;

namespace alps {

struct CommandOption {
    virtual void add_option(boost::program_options::options_description& desc) = 0;
    virtual std::string argname() = 0;
    virtual void set_value(const boost::program_options::variable_value& val) = 0;
    
};

template<typename T>
struct CommandOptionT: CommandOption
{
    CommandOptionT(std::string argname, T* out, std::string desc)
        : argname_(argname),
          out_(out),
          desc_(desc)
    { }

    std::string argname() {
        return argname_;
    }

    void add_option(boost::program_options::options_description& desc)
    {
        desc.add_options()
            (argname_.c_str(), boost::program_options::value<T>(), desc_.c_str());
    }

    void set_value(const boost::program_options::variable_value& val)
    {
        *out_ =  val.as<T>();
    }

    std::string argname_;
    T*          out_;
    std::string desc_;
};

typedef std::list<CommandOption*> CommandOptionList;


/**
 * @brief Represents an object that can be written to and read from files/bytes in YAML format.
 * @ingroup EXTERNALIZE
 * @details
 * Derived classes must implement load() and save().
 */
struct Externalizable {
  /**
   * @brief Reads the content of this object from the given YAML element.
   * @param[in] node the YAML node that represents this object
   * @details
   * Expect errors due to missing-elements, out-of-range values, etc.
   */
  virtual ErrorStack load(YAML::Node* node, bool ignore_missing = false) = 0;

  /**
   * @brief Writes the content of this object to the given YAML element.
   * @param[in] node the YAML node that represents this object
   * @details
   * Expect only out-of-memory error.
   * We receive the YAML node this object will represent, so this method does not determine
   * the YAML node name of itself. The parent object determines children's tag names
   * because one parent object might have multiple child objects of the same type with different
   * YAML element name.
   */
  virtual ErrorStack save(YAML::Emitter* out) const = 0;

  /**
   * @brief Adds command options for parsing.
   */
  virtual ErrorStack add_command_options(CommandOptionList* cmdopt) = 0; 

  /**
   * @brief Returns a YAML tag name for this object as a root element.
   * @details
   * We might want to give a different name for same externalizable objects,
   * so this is used only when it is the root element of yaml.
   */
  virtual const char* get_tag_name() const = 0;

  /**
   * @brief Polymorphic assign operator. This should invoke operator= of the derived class.
   * @param[in] other assigned value. It must be dynamic-castable to the assignee class.
   */
  virtual void assign(const alps::Externalizable *other) = 0;

  /**
   * @brief Load the content of this object from the given YAML string.
   * @param[in] yaml YAML string.
   * @param[in] ignore_missing whether to ignore missing options.
   * @details
   * Expect errors due to missing-elements, out-of-range values, etc.
   */
  ErrorStack  load_from_string(const std::string& yaml, bool ignore_missing = false);

  /**
   * @brief Invokes save() and directs the resulting YAML text to the given stream.
   * @param[in] ptr ostream to write to.
   */
  void        save_to_stream(std::ostream* ptr) const;

  /**
   * @brief Load the content of this object from the specified YAML file.
   * @param[in] path path of the YAML file.
   * @param[in] ignore_missing whether to ignore missing options.
   * @details
   * Expect errors due to missing-elements, out-of-range values, etc.
   */
  ErrorStack  load_from_file(const fs::path &path, bool ignore_missing = false);

  /**
   * @brief Atomically and durably writes out this object to the specified YAML file.
   * @param[in] path path of the YAML file.
   * @details
   * If the file exists, this method atomically overwrites it via POSIX's atomic rename semantics.
   * If the parent folder doesn't exist, this method automatically creates the folder.
   * Expect errors due to file-permission (and other file I/O issue), out-of-memory, etc.
   */
  ErrorStack  save_to_file(const fs::path &path) const;

  /**
   * @brief Load content from command line options.
   * @param[in] argc number of strings that make up the command line.
   * @param[in] argv arguments passed to a program through the command line.
   * @details
   * Expect errors due to missing-elements, out-of-range values, etc.
   */
  ErrorStack load_from_command_options(int argc, char* argv[]);

  // convenience methods
  static ErrorStack insert_comment(YAML::Emitter* out, const std::string& comment);
  static ErrorStack append_comment(YAML::Emitter* parent, const std::string& comment);

  /**
   * Only declaration in header. Explicitly instantiated in cpp for each type this func handles.
   */
  template <typename T>
  static ErrorStack add_element(YAML::Emitter* out, const std::string& tag,
                  const std::string& comment, T value, bool seq = false);

  /** 
   * vector version 
   *
   * Only declaration in header. Explicitly instantiated in cpp for each type this func handles.
   */
  template <typename T>
  static ErrorStack add_element(YAML::Emitter* out, const std::string& tag,
            const std::string& comment, const std::vector< T >& value, bool seq = true);

  /** enum version */
  template <typename ENUM>
  static ErrorStack add_enum_element(YAML::Emitter* out, const std::string& tag,
                const std::string& comment, ENUM value) 
  {
    return add_element(out, tag, comment, static_cast<int64_t>(value));
  }

  /** child Externalizable version */
  static ErrorStack add_child_element(YAML::Node* parent, const std::string& tag,
                  const std::string& comment, const Externalizable& child);

  /**
   * Only declaration in header. Explicitly instantiated in cpp for each type this func handles.
   */
  template <typename T>
  static ErrorStack get_element(YAML::Node* parent, const std::string& tag,
                  T* out, bool ignore_missing = false, bool optional = false, T value = 0);
  /** string type is bit special. */
  static ErrorStack get_element(YAML::Node* parent, const std::string& tag,
                  std::string* out, bool ignore_missing = false, bool optional = false, const char* value = "");

  /** enum version */
  template <typename ENUM>
  static ErrorStack get_enum_element(YAML::Node* parent, const std::string& tag,
          ENUM* out, bool ignore_missing = false, bool optional = false, ENUM default_value = static_cast<ENUM>(0)) 
  {
    // enum might be signged or unsigned, 1 byte, 2 byte, or 4 byte.
    // But surely it won't exceed int64_t range.
    int64_t tmp;
    CHECK_ERROR(get_element<int64_t>(parent, tag, &tmp, ignore_missing, optional, default_value));
    if (static_cast<int64_t>(static_cast<ENUM>(tmp)) != tmp) {
      return ERROR_STACK_MSG(kErrorCodeConfValueOutofrange, tag.c_str());
    }
    *out = static_cast<ENUM>(tmp);
    return kRetOk;
  }

  /** size version */
  template <typename SIZE>
  static ErrorStack get_size_element(YAML::Node* parent, const std::string& tag,
          SIZE* out, bool ignore_missing = false, bool optional = false, SIZE default_value = static_cast<SIZE>(0)) 
  {
    std::string tmp;
    std::string tmp_default_value = std::to_string(default_value);
    CHECK_ERROR(get_element<std::string>(parent, tag, &tmp, ignore_missing, optional, tmp_default_value));
    *out = string_to_size(tmp);
    return kRetOk;
  }


  /**
   * vector version.
   * Only declaration in header. Explicitly instantiated in cpp for each type this func handles.
   */
  template <typename T>
  static ErrorStack get_element(YAML::Node* parent, const std::string& tag,
            std::vector< T >* out, bool ignore_missing = false, bool optional = false);

  /** child Externalizable version */
  static ErrorStack get_child_element(YAML::Node* parent, const std::string& tag,
            Externalizable* child, bool ignore_missing = false, bool optional = false);


  template <typename T>
  static ErrorStack add_command_option(CommandOptionList* cmdlist, const std::string& tag,
                   T* out, std::string desc)
  {
    cmdlist->push_back(new CommandOptionT<T>(tag, out, desc));
    return kRetOk;
  }


};

}  // namespace alps

// A bit tricky to get "a" from a in C macro.
#define EX_QUOTE(str) #str
#define EX_EXPAND(str) EX_QUOTE(str)

/**
 * @def EXTERNALIZE_SAVE_ELEMENT(element, attribute, comment)
 * @ingroup EXTERNALIZE
 * @brief Adds an yaml element to represent a member variable of \e this object.
 * @param[in] element the current YAML element that represents \e this, in other words parent
 * of the new element.
 * @param[in] attribute the member variable of \e this to save. This is also used as tag name.
 * @param[in] comment this is output as an YAML comment.
 */
#define EXTERNALIZE_SAVE_ELEMENT(emitter, attribute, comment) \
  CHECK_ERROR(add_element(emitter, EX_EXPAND(attribute), comment, attribute))
/**
 * @def EXTERNALIZE_SAVE_ENUM_ELEMENT(element, attribute, comment)
 * @ingroup EXTERNALIZE
 * @copydoc EXTERNALIZE_SAVE_ELEMENT(element, attribute, comment)
 * @details
 * For enums, use this one.
 */
#define EXTERNALIZE_SAVE_ENUM_ELEMENT(emitter, attribute, comment) \
  CHECK_ERROR(add_enum_element(emitter, EX_EXPAND(attribute), comment, attribute))

/**
 * @def EXTERNALIZE_LOAD_ELEMENT(element, ignore_missing, attribute)
 * @ingroup EXTERNALIZE
 * @brief Reads a child yaml element to load a member variable of \e this object.
 * @param[in] element the current YAML element that represents \e this, in other words parent
 * of the element to read.
 * @param[in] ignore_missing a boolean whether to ignore a missing attribute
 * @param[in] attribute the member variable of \e this to save. This is also used as tag name.
 */
#define EXTERNALIZE_LOAD_ELEMENT(element, ignore_missing, attribute) \
  CHECK_ERROR(get_element(element, EX_EXPAND(attribute), & attribute, ignore_missing))
/**
 * @def EXTERNALIZE_LOAD_ELEMENT_OPTIONAL(element, attribute, default_value)
 * @ingroup EXTERNALIZE
 * @copydoc EXTERNALIZE_LOAD_ELEMENT(element, ignore_missing, attribute)
 * @param[in] default_value If the element doesn't exist, this value is set to the variable.
 * @details
 * For optional elements, use this.
 */
#define EXTERNALIZE_LOAD_ELEMENT_OPTIONAL(element, attribute, default_value) \
  CHECK_ERROR(get_element(element, EX_EXPAND(attribute), & attribute, false, true, default_value))

/**
 * @def EXTERNALIZE_LOAD_ENUM_ELEMENT(element, ignore_missing, attribute, dst)
 * @ingroup EXTERNALIZE
 * @copydoc EXTERNALIZE_LOAD_ELEMENT(element, ignore_missing, attribute, dst)
 * @details
 * For enum, use this one.
 */
#define EXTERNALIZE_LOAD_ENUM_ELEMENT(element, ignore_missing, attribute) \
  CHECK_ERROR(get_enum_element(element, EX_EXPAND(attribute), & attribute, ignore_missing))
/**
 * @def EXTERNALIZE_LOAD_ENUM_ELEMENT_OPTIONAL(element, attribute, default_value)
 * @ingroup EXTERNALIZE
 * @copydoc EXTERNALIZE_LOAD_ELEMENT_OPTIONAL(element, attribute, default_value)
 * @details
 * For optional enum, use this one.
 */
#define EXTERNALIZE_LOAD_ENUM_ELEMENT_OPTIONAL(element, attribute, default_value) \
  CHECK_ERROR(get_enum_element(element, EX_EXPAND(attribute), & attribute, false, true, default_value))

/**
 * @def EXTERNALIZE_LOAD_SIZE_ELEMENT(element, ignore_missing, attribute, dst)
 * @ingroup EXTERNALIZE
 * @copydoc EXTERNALIZE_LOAD_ELEMENT(element, ignore_missing, attribute, dst)
 * @details
 * For size, use this one.
 */
#define EXTERNALIZE_LOAD_SIZE_ELEMENT(element, ignore_missing, attribute) \
  CHECK_ERROR(get_size_element(element, EX_EXPAND(attribute), & attribute, ignore_missing))
/**
 * @def EXTERNALIZE_LOAD_SIZE_ELEMENT_OPTIONAL(element, attribute, default_value)
 * @ingroup EXTERNALIZE
 * @copydoc EXTERNALIZE_SIZE_ELEMENT_OPTIONAL(element, attribute, default_value)
 * @details
 * For optional size, use this one.
 */
#define EXTERNALIZE_LOAD_SIZE_ELEMENT_OPTIONAL(element, attribute, default_value) \
  CHECK_ERROR(get_enum_element(element, EX_EXPAND(attribute), & attribute, false, true, default_value))

/**
 * @def EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, attribute, desc)
 * @ingroup EXTERNALIZE
 * @brief Adds a parser for a command line option.
 * @param[in] cmdlist the current list that holds the command line options.
 * @param[in] attribute the member variable of \e this to add. This is also used as tag name.
 * @param[in] desc this is output in the usage message as a command option description.
 */
#define EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, attribute, desc) \
  CHECK_ERROR(add_command_option(cmdlist, EX_EXPAND(attribute), &attribute, desc))


/**
 * @def EXTERNALIZABLE(clazz)
 * @ingroup EXTERNALIZE
 * @brief Macro to declare/define essential methods for an externalizable class.
 * @details
 * Each externalizable class should invoke this macro in public scope of class definition.
 * Then, it should define load() and save() in cpp.
 */
#define COMPLETE_EXTERNALIZABLE(clazz) \
  ErrorStack load(YAML::Node* node, bool ignore_missing) CXX11_OVERRIDE;\
  ErrorStack save(YAML::Node* node) const CXX11_OVERRIDE;\
  const char* get_tag_name() const CXX11_OVERRIDE { return EX_EXPAND(clazz); }\
  void assign(const alps::Externalizable *other) CXX11_OVERRIDE {\
    *this = *dynamic_cast< const clazz * >(other);\
  }\
  friend std::ostream& operator<<(std::ostream& o, const clazz & v) {\
    v.save_to_stream(&o);\
    return o;\
  }

#define EXTERNALIZABLE(clazz) \
  ErrorStack load(YAML::Node* node, bool ignore_missing) CXX11_OVERRIDE;\
  ErrorStack save(YAML::Emitter* out) const;\
  ErrorStack add_command_options(CommandOptionList* cmdopt); \
  const char* get_tag_name() const CXX11_OVERRIDE { return EX_EXPAND(clazz); }\
  void assign(const alps::Externalizable *other) {\
    *this = *dynamic_cast< const clazz * >(other);\
  }\
  friend std::ostream& operator<<(std::ostream& o, const clazz & v) {\
    return o;\
  }

#endif  // _ALPS_COMMON_EXTERNALIZABLE_HH_
