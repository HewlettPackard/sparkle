#ifndef _ALPS_PEGASUS_FSTYPE_FACTORY_TCC_
#define _ALPS_PEGASUS_FSTYPE_FACTORY_TCC_

#include "common/debug.hh"
#include "common/os.hh"

namespace alps {

template<typename ObjectType>
FileSystemTypeFactory<ObjectType>::FileSystemTypeFactory(const PegasusOptions& pegasus_options)
    : pegasus_options_(pegasus_options)
{ }


template<typename ObjectType>
std::string FileSystemTypeFactory<ObjectType>::fstype(const boost::filesystem::path& pathname)
{
    return os_fstype(pathname.c_str());
}


template<typename ObjectType>
ErrorCode FileSystemTypeFactory<ObjectType>::construct(const boost::filesystem::path& pathname, ObjectType** object)
{
    boost::filesystem::path parent_path = pathname.parent_path();
    if (!boost::filesystem::exists(parent_path)) {
        LOG(error) << "Directory " << parent_path << " does not exist.";
        return kErrorCodeFsNoDirectory;
    }

    std::string fst = fstype(pathname);

    ConstructCallback ccb;
    typename ConstructCallbackMap::const_iterator it = callbacks_.find(fst);
    typename ConstructCallbackMap::const_iterator it_any = callbacks_.find("any");
    if (it == callbacks_.end() && it_any == callbacks_.end()) {
        LOG(error) << "Unknown file system type " << fst
                   << ". Check whether file system is known to os_fstype.";
        return kErrorCodeFsUnknownFSType;
    } else {
        if (it == callbacks_.end()) {
            ccb = it_any->second;
        } else {
            ccb = it->second;
        }
    }
    ObjectType* obj;
    if ((obj = ccb(pathname, pegasus_options_)) == NULL) {
        return kErrorCodeOutofmemory;
    }
    *object = obj;
    return kErrorCodeOk;
}


template<typename ObjectType>
ErrorCode FileSystemTypeFactory<ObjectType>::register_fstype(const std::string& fstype, ConstructCallback ccb)
{
    callbacks_[fstype] = ccb;
    return kErrorCodeOk;
}


} // namespace alps

#endif // _ALPS_PEGASUS_FSTYPE_FACTORY_TCC_
