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

#ifndef _ALPS_PEGASUS_FSTYPE_FACTORY_HH_
#define _ALPS_PEGASUS_FSTYPE_FACTORY_HH_

#include "alps/common/assert_nd.hh"
#include "alps/common/error_code.hh"
#include "alps/pegasus/pegasus_options.hh"

namespace alps {

/**
 * @brief A factory class for constructing objects based on the type of 
 * the underlying file system.
 */
template<typename ObjectType>
class FileSystemTypeFactory {
public:
    typedef ObjectType* (*ConstructCallback)(const boost::filesystem::path& pathname, const PegasusOptions& pegasus_options);

public:
    FileSystemTypeFactory(const PegasusOptions& pegasus_options);
    ErrorCode construct(const boost::filesystem::path& pathname, ObjectType** object);
    ErrorCode register_fstype(const std::string& fstype, ConstructCallback);

private:
    typedef std::map<std::string, ConstructCallback> ConstructCallbackMap;

private:
    std::string fstype(const boost::filesystem::path& pathname);

protected:
    ConstructCallbackMap callbacks_;
    PegasusOptions       pegasus_options_;
};

} // namespace alps

#include "fstype_factory.tcc"

#endif // _ALPS_PEGASUS_FSTYPE_FACTORY_HH_
