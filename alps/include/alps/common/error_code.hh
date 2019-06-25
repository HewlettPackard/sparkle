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

#ifndef __ALPS_COMMON_ERROR_CODE_HH
#define __ALPS_COMMON_ERROR_CODE_HH

#include "compiler.hh"

namespace alps {

/**
 * @defgroup ERRORCODES Error codes, messages, and stacktraces
 * @ingroup IDIOMS
 * @brief Error codes (alps::ErrorCode), their error messages defined in error_code.xmacro, and
 * stacktrace information (ErrorStack) returned by our API functions.
 * @details
 * @par What it is
 * We define all error codes and their error messages here.
 * Whenever you want a new error message, add a new line in error_code.xmacro like existing lines.
 * This file is completely independent and header-only. Just include this file to use.
 *
 * @par X-Macros
 * To concisely define error codes, error names, and error messages,
 * we use the so-called "X Macro" style, which doesn't require any code generation.
 * @see http://en.wikipedia.org/wiki/X_Macro
 * @see http://www.drdobbs.com/the-new-c-x-macros/184401387
 *
 * @par ErrorCode vs ErrorStack
 * alps::ErrorCode is merely an integer to identify the type of error.
 * You can get a correponding error message and name of the error via
 * get_error_name() and get_error_message(), but you can't get stacktrace information.
 * For lightweight functions used internally, it might be enough.
 * However, public API methods might need stacktrace information for ease of use.
 * In that case, you should return ErrorStack, which additionally contains stacktrace and
 * custom error message.
 * ErrorStack is much more costly if it returns an error (if it's kErrorCodeOk, very efficient)
 * and especially when it contains a custom error message (See ErrorStack for more details).
 *
 * @par How to use ErrorStack
 * To use ErrorStack, you should be familiar with how to use the following macros:
 * alps::kRetOk, CHECK_ERROR_CODE(x), CHECK_ERROR(x), ERROR_STACK(e), COERCE_ERROR(x),
 * and a few others. For example, use it as follows:
 * @code{.cpp}
 * ErrorStack your_func() {
 *   if (out-of-memory-observed) {
 *      return ERROR_STACK(kErrorCodeOutofmemory);
 *   }
 *   CHECK_ERROR_CODE(another_func());
 *   CHECK_ERROR_CODE(yet_another_func());
 *   return kRetOk;
 * }
 * @endcode
 *
 * @par Current List of ErrorCode
 * See alps::ErrorCode.
 */
/**
 * @file error_code.xmacro
 * @ingroup ERRORCODES
 * @brief Error code/message definition in X-Macro style.
 */

#define X(a, b, c) /** b: c. */ a = b,
/**
 * @var ErrorCode
 * @ingroup ERRORCODES
 * @brief Enum of error codes defined in error_code.xmacro.
 * @details
 * This is often used as a return value of lightweight functions.
 * If you need more informative information, such as error stack, use ErrorStack.
 * But, note that returning this value is MUCH more efficient.
 */
enum ErrorCode {
  /** 0 means no-error. */
  kErrorCodeOk = 0,
#include "error_code.xmacro" // NOLINT
};
#undef X

/**
 * @brief Returns the names of ErrorCode enum defined in error_code.xmacro.
 * @ingroup ERRORCODES
 */
const char* get_error_name(ErrorCode code);

/**
 * @brief Returns the error messages corresponding to ErrorCode enum defined in error_code.xmacro.
 * @ingroup ERRORCODES
 */
const char* get_error_message(ErrorCode code);

// A bit tricky to get "a" from a in C macro.
#define X_QUOTE(str) #str
#define X_EXPAND_AND_QUOTE(str) X_QUOTE(str)
#define X(a, b, c) case a: return X_EXPAND_AND_QUOTE(a);
inline const char* get_error_name(ErrorCode code) {
  switch (code) {
    case kErrorCodeOk: return "kErrorCodeOk";
#include "error_code.xmacro" // NOLINT
  }
  return "Unexpected error code";
}
#undef X
#undef X_EXPAND_AND_QUOTE
#undef X_QUOTE

#define X(a, b, c) case a: return c;
inline const char* get_error_message(ErrorCode code) {
  switch (code) {
    case kErrorCodeOk: return "no_error";
#include "error_code.xmacro" // NOLINT
  }
  return "Unexpected error code";
}
#undef X

}  // namespace alps

/**
 * @def CHECK_ERROR_CODE(x)
 * @ingroup ERRORCODES
 * @brief
 * This macro calls \b x and checks its returned error code.  If the code is NOT kErrorCodeOk, it
 * immediately returns from the current function or method, returning the error code code.
 * For example, use it as follows:
 * @code{.cpp}
 * ErrorCode another_func();
 * ErrorCode yet_another_func();
 * ErrorCode your_func() {
 *   CHECK_ERROR_CODE(another_func());
 *   CHECK_ERROR_CODE(yet_another_func());
 *   return kErrorCodeOk;
 * }
 * @endcode
 *
 * This macro is used in performance-critical functions that do not return ErrorStack but returns
 * ErrorCode to save overheads. For a function that is called billion times per second, ErrorStack
 * \b does cause bottleneck, especially because it requires to allocate hundreds bytes on stack,
 * which would purge other data from cache lines. We actually did observe such situations in
 * a few experiments. If your CPU profiling tells that ErrorStack-related methods cause more than
 * 10% cpu costs, replace ErrorStack with ErrorCode.
 * @see WRAP_ERROR_CODE(x)
 */
#define CHECK_ERROR_CODE(x)\
{\
  alps::ErrorCode __e = x;\
  if (UNLIKELY(__e != kErrorCodeOk)) {\
    return __e;\
  }\
}


/**
 * @def CHECK_ERROR_CODE2(x,cleanup)
 * @ingroup ERRORCODES
 * @brief
 * This macro calls \b x and checks its returned error code.  If the code is NOT kErrorCodeOk, it
 * executes the associated cleanup method, and immediately returns from the current function or 
 * method, returning the error code code.
 * For example, use it as follows:
 * @code{.cpp}
 * ErrorCode another_func();
 * ErrorCode yet_another_func();
 * ErrorCode your_func() {
 *   CHECK_ERROR_CODE2(another_func(),cleanup_statement);
 *   CHECK_ERROR_CODE2(yet_another_func(),cleanup_statement);
 *   return kErrorCodeOk;
 * }
 * @endcode
 *
 * This macro is used in performance-critical functions that do not return ErrorStack but returns
 * ErrorCode to save overheads. For a function that is called billion times per second, ErrorStack
 * \b does cause bottleneck, especially because it requires to allocate hundreds bytes on stack,
 * which would purge other data from cache lines. We actually did observe such situations in
 * a few experiments. If your CPU profiling tells that ErrorStack-related methods cause more than
 * 10% cpu costs, replace ErrorStack with ErrorCode.
 * @see WRAP_ERROR_CODE(x)
 */
#define CHECK_ERROR_CODE2(x,cleanup)\
{\
  alps::ErrorCode __e = x;\
  if (UNLIKELY(__e != kErrorCodeOk)) {\
    cleanup; \
    return __e;\
  }\
}

#endif
