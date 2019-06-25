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

#include "alps/common/error_stack.hh"

#include <iostream>
#include <sstream>
#include <string>

#include "alps/common/assert_nd.hh"
#include "alps/common/assorted_func.hh"

#include "debug.hh"

namespace alps {
void ErrorStack::output(std::ostream* ptr) const {
  std::ostream &o = *ptr;  // just to workaround non-const reference rule.
  if (!is_error()) {
    o << "No error";
  } else {
    o << get_error_name(error_code_) << "(" << error_code_ << "):" << get_message();
    if (os_errno_ != 0) {
      o << " (Latest system call error=" << os_error(os_errno_) << ")";
    }
    if (get_custom_message()) {
      o << " (Additional message=" << get_custom_message() << ")";
    }

    for (uint16_t stack_index = 0; stack_index < get_stack_depth(); ++stack_index) {
      o << std::endl << "  " << get_filename(stack_index)
        << ":" << get_linenum(stack_index) << ": ";
      if (get_func(stack_index) != nullptr) {
        o << get_func(stack_index) << "()";
      }
    }
    if (get_stack_depth() >= alps::ErrorStack::kMaxStackDepth) {
      o << std::endl << "  .. and more. Increase kMaxStackDepth to see full stacktraces";
    }
  }
}

/**
 * Leaves recent dump information in a static global variable so that a signal handler can pick it.
 */
std::string static_recent_dump_and_abort;

void ErrorStack::dump_and_abort(const char *abort_message) const {
  std::stringstream str;
  str << "alps::ErrorStack::dump_and_abort: " << abort_message << std::endl << *this << std::endl;
  str << print_backtrace();

  static_recent_dump_and_abort += str.str();
  LOG(fatal) << str.str();
  ASSERT_ND(false);
  std::cout.flush();
  std::cerr.flush();
  std::abort();
}

std::string ErrorStack::get_recent_dump_and_abort() {
  return static_recent_dump_and_abort;
}

std::ostream& operator<<(std::ostream& o, const ErrorStack& obj) {
  obj.output(&o);
  return o;
}

}  // namespace alps

