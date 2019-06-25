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

#ifndef _ALPS_COMPILER_HH
#define _ALPS_COMPILER_HH

namespace alps {

/**
 * @defgroup COMPILER Compiler Specific Optimizations
 * @ingroup IDIOMS
 * @brief A few macros and functions to exploit compiler specific optimizations.
 * @details
 * This file contains a few macros and functions analogous to linux/compiler.h
 * @par Example: likely/unlikely macros
 * For example, Linux kernel wraps GCC's __builtin_expect as follows.
 * @code{.cpp}
 * // from linux/compiler.h.
 * #define likely(x)      __builtin_expect(!!(x), 1)
 * #define unlikely(x)    __builtin_expect(!!(x), 0)
 * ...
 * if (likely(var == 42)) {
 *   ...
 * }
 * @endcode
 * We provide macros/functions to do equivalent optimizations here.
 * \b However, we should minimize the use of such detailed and custom optimizations.
 * In general, compiler is good enough to predict branches/inlining, and, even if it isn't, we
 * should rather use -fprofile-arcs. So far, we use it only for a few places that are VERY
 * important and VERY easy to predict by human.
 *
 * @see http://blog.man7.org/2012/10/how-much-do-builtinexpect-likely-and.html
 */

/**
 * @def LIKELY(x)
 * @ingroup COMPILER
 * @brief Hints that x is highly likely true. GCC's __builtin_expect.
 */
/**
 * @def UNLIKELY(x)
 * @ingroup COMPILER
 * @brief Hints that x is highly likely false. GCC's __builtin_expect.
 */
/**
 * @def NO_INLINE
 * @ingroup COMPILER
 * @brief A function suffix to hint that the function should never be inlined. GCC's noinline.
 */
/**
 * @def ALWAYS_INLINE
 * @ingroup COMPILER
 * @brief A function suffix to hint that the function should always be inlined. GCC's always_inline.
 */
/**
 * @def ASSUME_ALIGNED(ptr, alignment)
 * @ingroup COMPILER
 * @brief Wraps GCC's __builtin_assume_aligned.
 */
/**
 * @def MAY_ALIAS
 * @ingroup COMPILER
 * @brief Wraps GCC's  __attribute__((__may_alias__)).
 * @details
 * This is \e QUITE important for us. This keyword is not for performance but for correctness.
 * I'm seeing weird behaviors even with -fno-strict-aliasing. This keyword might help.
 * Otherwise, we have to memcpy to type-pun everything. uggggrrr.
 */
/**
 * @def RESTRICT_ALIAS
 * @ingroup COMPILER
 * @brief Wraps GCC's  __restrict.
 * @details
 * OTOH, this explicitly helps compiler auto-vectorize and do other stuffs, saying that the
 * variable is never aliased in the function. Seems like older gcc ignored __restrict when
 * fno-strict-aliasing is specified, but recent gcc doesn't.
 * @attention DO NOT USE THIS if you don't know what __restrict means.
 */
#ifdef __INTEL_COMPILER
// ICC
#define LIKELY(x)      (x)
#define UNLIKELY(x)    (x)
#define NO_INLINE
#define ALWAYS_INLINE
#define ASSUME_ALIGNED(x, y) x
#define MAY_ALIAS
#define RESTRICT_ALIAS
#else  // __INTEL_COMPILER
#if defined(__GNUC__) || defined(__clang__)
// GCC and Clang (not sure Clang supports all of below, tho..)
#define LIKELY(x)      __builtin_expect(!!(x), 1)
#define UNLIKELY(x)    __builtin_expect(!!(x), 0)
#define NO_INLINE       __attribute__((noinline))
#define ALWAYS_INLINE   __attribute__((always_inline))
#define ASSUME_ALIGNED(x, y) __builtin_assume_aligned(x, y)
#define MAY_ALIAS __attribute__((__may_alias__))
#define RESTRICT_ALIAS __restrict
#else  // defined(__GNUC__) || defined(__clang__)
// Others. MSVC?
#define LIKELY(x)      (x)
#define UNLIKELY(x)    (x)
#define NO_INLINE
#define ALWAYS_INLINE
#define ASSUME_ALIGNED(x, y) x
#define MAY_ALIAS
#define RESTRICT_ALIAS
#endif   // __GNUC__
#endif  // __INTEL_COMPILER


} // namespace alps

#endif
