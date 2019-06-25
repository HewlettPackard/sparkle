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

#ifndef _ALPS_COMMON_LOG_H_
#define _ALPS_COMMON_LOG_H_

#define BOOST_LOG_DYN_LINK 1 // necessary when linking the boost_log library dynamically
 
// trivial severity levels:
//    trace,
//    debug,
//    info,
//    warning,
//    error,
//    fatal

#include <pthread.h>

#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

#include "alps/common/debug_options.hh"
 
// just log messages with severity >= SEVERITY_THRESHOLD are written
#define SEVERITY_THRESHOLD logging::trivial::warning
 
#define LOG(severity) BOOST_LOG_SEV(logger, boost::log::trivial::severity) << "(" << __FILE__ << ", " << __LINE__ << ") "
//#define LOG(severity) BOOST_LOG_SEV(logger, boost::log::trivial::severity) << "(" << __FILE__ << ", " << __LINE__ << ", T." << pthread_self() << ") "

namespace alps {

extern boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger;

void init_log(const DebugOptions& options);

} // namespace alps

#endif // _ALPS_COMMON_LOG_H_
