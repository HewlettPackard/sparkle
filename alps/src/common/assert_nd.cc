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

#include "alps/common/assert_nd.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "common/rich_backtrace.hh"

namespace alps {

std::string print_assert(const char* file, const char* func, int line, const char* description) {
  std::stringstream str;
  str << "***************************************************************************" << std::endl;
  str << "**** Assertion failed! \"" << description
    << "\" did not hold in " << func << "(): " << file << ":" << line << std::endl;
  str << "***************************************************************************" << std::endl;
  return str.str();
}

std::string print_backtrace() {
  std::vector<std::string> traces = get_backtrace(true);
  std::stringstream str;
  str << "=== Stack frame (length=" << traces.size() << ")" << std::endl;
  for (uint16_t i = 0; i < traces.size(); ++i) {
    str << "- [" << i << "/" << traces.size() << "] " << traces[i] << std::endl;
  }
  return str.str();
}

/**
 * Leaves recent crash information in a static global variable so that a signal handler can pick it.
 */
std::string static_recent_assert_backtrace;

void print_assert_backtrace(const char* file, const char* func, int line, const char* description) {
  std::string message = print_assert(file, func, line, description);
  message += print_backtrace();
  static_recent_assert_backtrace += message;
  std::cerr << message;
}


std::string get_recent_assert_backtrace() {
  return static_recent_assert_backtrace;
}

}  // namespace alps
