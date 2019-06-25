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

#ifndef __ALPS_ASSERT_ND_HH
#define __ALPS_ASSERT_ND_HH

#include <string>

/**
 * @def ASSERT_ND(x)
 * @ingroup IDIOMS
 * @brief A warning-free wrapper macro of assert() that has no performance effect in release
 * mode \e even \e when 'x' is not a simple variable.
 * @details
 * The standard assert() in release mode often results in compiler warnings for unused variables.
 * A common mistake is to wrap it like this:
 * @code{.cpp}
 * #define ASSERT_INCORRECT(x) do { (void)(x); } while(0)
 * @endcode
 * This can cause a performance issue when 'x' is a function call etc because the compiler might
 * not be able to confidently get rid of the code.
 * Instead, we use the idea of (void) sizeof(x) trick in the following URL.
 * @see http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
 */
/**
 * @def UNUSED_ND(var)
 * @ingroup IDIOMS
 * @brief Cross-compiler UNUSED macro for the same purpose as ASSERT_ND(x).
 */
namespace alps {
  /** Helper function to report what assertion failed. */
  std::string print_assert(const char* file, const char* func, int line, const char* description);
  /** Prints out backtrace. This method is best-effort, maybe do nothing in some compiler/OS. */
  std::string print_backtrace();

  /**
   * print_assert() + print_backtrace().
   * It also leaves the info in a global variable so that signal handler can show that later.
   */
  void print_assert_backtrace(
    const char* file,
    const char* func,
    int line,
    const char* description);

  /** Retrieves the info left by print_assert_backtrace(). Called by signal handler */
  std::string get_recent_assert_backtrace();
}  // namespace alps

#ifdef NDEBUG
#define ASSERT_ND(x) do { (void) sizeof(x); } while (0)
#define UNUSED_ND(var) ASSERT_ND(var)
#else  // NDEBUG
#include <cassert>
#ifndef ASSERT_ND_NOBACKTRACE
#define ASSERT_QUOTE(str) #str
#define ASSERT_EXPAND_AND_QUOTE(str) ASSERT_QUOTE(str)
#define ASSERT_ND(x) do { if (!(x)) { \
  alps::print_assert_backtrace(__FILE__, __FUNCTION__, __LINE__, ASSERT_QUOTE(x)); \
  assert(x); \
  } } while (0)
#else  // ASSERT_ND_NOBACKTRACE
#define ASSERT_ND(x) assert(x)
#endif  // ASSERT_ND_NOBACKTRACE
#define UNUSED_ND(var) (void) (var)
#endif  // NDEBUG

#endif  // __ALPS_ASSERT_ND_HH
