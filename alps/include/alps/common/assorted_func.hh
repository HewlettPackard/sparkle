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

#ifndef _ALPS_COMMON_ASSORTED_FUNC_HH_
#define _ALPS_COMMON_ASSORTED_FUNC_HH_

#include <list>
#include <vector>

#include "assert_nd.hh"

#ifdef __GNUC__  // for get_pretty_type_name()
#include <cxxabi.h>
#endif  // __GNUC__
#include <string>

namespace alps {

/**
 * Thread-safe strerror(errno). We might do some trick here for portability, too.
 * @ingroup ASSORTED
 */
std::string os_error();

/**
 * This version receives errno.
 * @ingroup ASSORTED
 */
std::string os_error(int error_number);


/**
 * @brief Demangle the given C++ type name \e if possible (otherwise the original string).
 * @ingroup ASSORTED
 */
std::string demangle_type_name(const char* mangled_name);


/**
 * @brief Returns the name of the C++ type as readable as possible.
 * @ingroup ASSORTED
 * @tparam T the type
 */
template <typename T>
std::string get_pretty_type_name() {
  return demangle_type_name(typeid(T).name());
}


/**
 * @brief Returns the smallest multiply of ALIGNMENT that is equal or larger than the given number.
 * @ingroup ASSORTED
 * @tparam T integer type
 * @tparam ALIGNMENT alignment size. must be power of two
 * @details
 * In other words, round-up. For example of 8-alignment, 7 becomes 8, 8 becomes 8, 9 becomes 16.
 * @see https://en.wikipedia.org/wiki/Data_structure_alignment
 * @see Hacker's Delight 2nd Ed. Chap 3-1.
 */
template <typename T, uint64_t ALIGNMENT>
inline T align(T value) {
  uint64_t left = (value + ALIGNMENT - 1);
  uint64_t right = -ALIGNMENT;
  uint64_t result = left & right;
  ASSERT_ND(result >= static_cast<uint64_t>(value));
  ASSERT_ND(result % ALIGNMENT == 0);
  ASSERT_ND(result < static_cast<uint64_t>(value) + ALIGNMENT);
  return static_cast<T>(result);
}

const size_t kCacheLineSize = 64;


/**
 * @brief Convenient way of writing hex integers to stream.
 * @ingroup ASSORTED
 * @details
 * Use it as follows.
 * @code{.cpp}
 * std::cout << Hex(1234) << ...
 * // same output as:
 * // std::cout << "0x" << std::hex << std::uppercase << 1234 << std::nouppercase << std::dec << ...
 * @endcode
 */
struct Hex {
  template<typename T>
  Hex(T val, int fix_digits = -1) : val_(static_cast<uint64_t>(val)), fix_digits_(fix_digits) {}

  uint64_t    val_;
  int         fix_digits_;
  friend std::ostream& operator<<(std::ostream& o, const Hex& v);
};


/**
 * @brief Equivalent to std::hex in case the stream doesn't support it.
 * @ingroup ASSORTED
 * @details
 * Use it as follows.
 * @code{.cpp}
 * std::cout << Hex("aabc") << ...
 * // will output "0x61616263".
 * @endcode
 */
struct HexString {
  HexString(const std::string& str, uint32_t max_bytes = 64U)
    : str_(str), max_bytes_(max_bytes) {}

  std::string str_;
  uint32_t    max_bytes_;
  friend std::ostream& operator<<(std::ostream& o, const HexString& v);
};


/**
 * @brief Convert a byte size \a str into an integer and scale size by any 
 * unit included in \a str (e.g. K, M, G)
 * 
 * @details
 * For example, the following string will expand as follows:
 * - 128K == 131072
 */
uint64_t string_to_size(std::string str);


/** 
 * @brief Returns a vector of the words of string \a s delimited by \a delimiter.
 */ 
std::vector<std::string> split_string(std::string s, std::string delimiter);


/**
 * @brief Expands and returns a range description \a range as a list of integers.
 * 
 * @details
 * For example, the following range will expand as follows:
 *  - 1-5,8 == [1,2,3,4,5,8]
 */
std::list<size_t> expand_range(std::string range);


/**
 * @brief Round non-negative @a x up to the nearest multiple of 
 * non-negative @a multiple
 */
static inline size_t round_up(size_t x, size_t multiple)
{
    return ((x+multiple-1)/multiple)*multiple;
}


/**
 * @brief Round non-negative @a x down to the nearest multiple of 
 * non-negative @a multiple
 */
static inline size_t round_down(size_t x, size_t multiple)
{
    return (x/multiple)*multiple;
}


/**
 * @brief Check whether pointer address is aligned at 
 * non-negative @a multiple.
 */
static inline bool is_aligned(void* ptr, size_t multiple)
{
    return (uintptr_t(ptr) % multiple) == 0;
}


/**
 * @brief Check whether positive @x is a power of two.
 */
static inline bool is_power_of_two (size_t x)
{
    while (((x & 1) == 0) && x > 1) { /* While x is even and > 1 */
        x >>= 1;
    }
    return (x == 1);
}


/**
 * @def INSTANTIATE_ALL_TYPES(M)
 * @brief A macro to explicitly instantiate the given template for all types we care.
 * @ingroup ASSORTED
 * @details
 * M is the macro to explicitly instantiate a template for the given type.
 * This macro explicitly instantiates the template for bool, float, double, all integers
 * (signed/unsigned), and std::string.
 * This is useful when \e definition of the template class/method involve too many details
 * and you rather want to just give \e declaration of them in header.
 *
 * Use this as follows. In header file.
 * @code{.h}
 * template <typename T> void cool_func(T arg);
 * @endcode
 * Then, in cpp file.
 * @code{.cpp}
 * template <typename T> void cool_func(T arg) {
 *   ... (implementation code)
 * }
 * #define EXPLICIT_INSTANTIATION_COOL_FUNC(x) template void cool_func< x > (x arg);
 * INSTANTIATE_ALL_TYPES(EXPLICIT_INSTANTIATION_COOL_FUNC);
 * @endcode
 * Remember, you should invoke this macro in cpp, not header, otherwise you will get
 * multiple-definition errors.
 * @note Doxygen doesn't understand template explicit instantiation, giving warnings. Not a big
 * issue, but you should shut up the Doxygen warnings by putting cond/endcond. See
 * externalizable.cpp for example.
 */
/**
 * @def INSTANTIATE_ALL_NUMERIC_TYPES(M)
 * @brief INSTANTIATE_ALL_TYPES minus std::string.
 * @ingroup ASSORTED
 */
/**
 * @def INSTANTIATE_ALL_INTEGER_PLUS_BOOL_TYPES(M)
 * @brief INSTANTIATE_ALL_TYPES minus std::string/float/double.
 * @ingroup ASSORTED
 */
/**
 * @def INSTANTIATE_ALL_INTEGER_TYPES(M)
 * @brief INSTANTIATE_ALL_NUMERIC_TYPES minus bool/double/float.
 * @ingroup ASSORTED
 */
#define INSTANTIATE_ALL_INTEGER_TYPES(M) M(int64_t);  /** NOLINT(readability/function) */\
  M(int32_t); M(int16_t); M(int8_t); M(uint64_t);  /** NOLINT(readability/function) */\
  M(uint32_t); M(uint16_t); M(uint8_t); /** NOLINT(readability/function) */

#define INSTANTIATE_ALL_INTEGER_PLUS_BOOL_TYPES(M) INSTANTIATE_ALL_INTEGER_TYPES(M);\
  M(bool); /** NOLINT(readability/function) */

#define INSTANTIATE_ALL_NUMERIC_TYPES(M) INSTANTIATE_ALL_INTEGER_TYPES(M);\
  M(bool); M(float); M(double); /** NOLINT(readability/function) */

#define INSTANTIATE_ALL_TYPES(M) INSTANTIATE_ALL_NUMERIC_TYPES(M);\
  M(std::string);  /** NOLINT(readability/function) */




} // namespace alps

#endif // _ALPS_COMMON_ASSORTED_FUNC_HH_
