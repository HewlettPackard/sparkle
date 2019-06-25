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

#ifndef _ALPS_DEBUG_OPTIONS_HH_
#define _ALPS_DEBUG_OPTIONS_HH_

#include "alps/common/externalizable.hh"

namespace alps {

struct DebugOptions: public Externalizable {
    std::string kDefaultLogFilename = "sample.log";
    std::string kDefaultLogLevel = "error";

    /**
     * Constructs option values with default values
     */
    DebugOptions() {
        log_filename = kDefaultLogFilename;
        log_level = kDefaultLogLevel;
    }

    /** 
     * Store log messages to this file
     */
    std::string log_filename;

    /** 
     * Log messages at or above this level
     */
    std::string log_level;

    EXTERNALIZABLE(DebugOptions)
};


} //namespace alps

#endif // _ALPS_DEGUG_OPTIONS_HH_
