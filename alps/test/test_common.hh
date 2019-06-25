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

#ifndef _ALPS_TEST_COMMON_HH_
#define _ALPS_TEST_COMMON_HH_

#include <sys/vfs.h>

#include <chrono>
#include <iomanip>  
#include <string> 

#include <gtest/gtest.h>

#include "boost/program_options.hpp" 
#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"

#include "alps/layers/pointer.hh"

#include "alps/common/assert_nd.hh"
#include "alps/common/debug_options.hh"
#include "alps/common/error_code.hh"
#include "alps/common/error_stack.hh"
#include "alps/common/externalizable.hh"
#include "alps/common/log.hh"

#include "common/os.hh"

namespace alps {

class TestEnvironment;

static TestEnvironment* global_test_env;

struct TestOptions: public Externalizable {
    std::string kDefaultTestDir = "/dev/shm/nvm";
    std::string kDefaultLogLevel = "ERROR";
    size_t kBookSizeBytes = 0;
    std::string kConfigFile = "";
    bool kUseEnviron = false;

    /**
     * Constructs option values with default values
     */
    TestOptions() {
        test_dir = kDefaultTestDir;
        log_level = kDefaultLogLevel;
        book_size_bytes = kBookSizeBytes;
        config_file = kConfigFile;
        use_environ = kUseEnviron;
    }

    /** 
     * Directory where to create test region files
     */
    std::string test_dir;

    /** 
     * Store log messages to this file
     */
    std::string log_filename;

    /** 
     * Log messages at or above this level
     */
    std::string log_level;

    /** 
     * Book size used by the backing file system
     */
    size_t book_size_bytes;

    /** 
     * File to load configuration options from
     */
    std::string config_file;

    /**
     * Load configuration options from file declared in environment     
     * variable ALPS_CONF
     */
    bool use_environ;

    EXTERNALIZABLE(TestOptions)
};

ErrorStack TestOptions::load(YAML::Node* node, bool ignore_missing) {
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, log_level);
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, test_dir);
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, book_size_bytes);
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, use_environ);

    return kRetOk;
}

ErrorStack TestOptions::save(YAML::Emitter* out) const {
    return kRetOk;
}

ErrorStack TestOptions::add_command_options(CommandOptionList* cmdlist) {
    EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, log_level, "Log messages at or above this level: INFO, WARNING, ERROR, and FATAL");
    EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, test_dir, "Directory where to create test region files.");
    EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, book_size_bytes, "Override book size used by the backing file system.");
    EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, config_file, "File to load configuration options from.");
    EXTERNALIZE_ADD_COMMAND_OPTION(cmdlist, use_environ, "Load configuration options from file declared in environment variable ALPS_CONF.");

    return kRetOk;
}


class TestEnvironment: public ::testing::Environment {
public:
    const char* test_path_prefix = "alps.test";

public:
    TestEnvironment(TestOptions& test_options, bool do_cleanup_fs = true)
        : test_options_(test_options),
          do_cleanup_fs_(do_cleanup_fs)
    { 
    }

    void SetUp() 
    {
        DebugOptions debug_options;
        debug_options.log_level = test_options_.log_level;
        init_log(debug_options);


        if (do_cleanup_fs_) {
            cleanup_fs();
        }
    }

    void TearDown() { }

    std::string test_dir() 
    {
        return test_options_.test_dir;
    }

    std::string test_path(std::string name) 
    {
        std::string dir = test_dir();
        dir += dir.back() != '/' ? "/" : "";
        return dir + std::string(test_path_prefix) + "." + name ;
    }

    size_t booksize() 
    {
        assert(0 && "unimplemented");
    }

    void cleanup_fs() {
        std::string fstype = os_fstype(test_dir().c_str());
        if (fstype == "tmpfs") {
            cleanup_tmpfs();
            return;
        }
        if (fstype == "tmfs") {
            cleanup_tmfs();
            return;
        }
    }

private:

    void cleanup_tmpfs() {
        char buf[1024];
        

        sprintf(buf, "mkdir -p %s", test_dir().c_str());
        system(buf);

        fs_remove_matched(test_dir(), std::regex(std::string(test_path_prefix) + ".*"), false);
        system("rm -f /dev/shm/@@lockfile@@");
        system("umask 0");
        system("touch /dev/shm/@@lockfile@@");
    }

    void cleanup_tmfs() {
        // wait for tmfs to reclaim space
        //fs_remove_matched(test_dir(), std::regex(std::string(test_path_prefix) + ".*"), true);
        fs_remove_matched(test_dir(), std::regex(std::string(test_path_prefix) + ".*"), false);
    }

protected:
    std::vector<boost::filesystem::path> region_file_paths;
    const char**                         region_file_paths_c;
    TestOptions                          test_options_;
    bool                                 do_cleanup_fs_;
};

template<typename EnvType>
int init_test_env(int argc, char** argv)
{
    TestOptions test_options;
    test_options.load_from_command_options(argc, argv);

    EnvType* env = new EnvType(test_options);
    ::testing::AddGlobalTestEnvironment(env);
    global_test_env = env;
    return 0;
}

template<typename EnvType>
int init_integration_test_env(int argc, char** argv)
{
    TestOptions test_options;
    test_options.load_from_command_options(argc, argv);

    EnvType* env = new EnvType(test_options, false);
    ::testing::AddGlobalTestEnvironment(env);
    global_test_env = env;
    return 0;
}

class AnonymousRegionTest: public ::testing::Test {
public:
    TPtr<void> alloc(size_t size)
    {
        void* ptr = malloc(size);
        return ptr;
    }
};




template<typename T>
bool compare_output(const T& obj, std::stringstream& os)
{
    std::stringstream out;
    out << obj;
    return (out.str() == os.str());
} 


#define EXPECT_OUTPUT(actual, expected) \
  do { \
    bool r; \
    std::stringstream ss; \
    ss << expected; \
    EXPECT_EQ(true, r = compare_output(actual, ss)); \
    if (!r) { \
      std::cout << "ACTUAL:" << std::endl << actual << std::endl; \
      std::cout << "EXPECTED:" << std::endl << expected << std::endl; \
    } \
  } while (0); \


#define EXPECT_EQ_VERBOSE(expected, actual) \
  do { \
    bool r; \
    EXPECT_EQ(true, r = (actual == expected)); \
    if (!r) { \
      std::cout << "ACTUAL:" << std::endl << actual << std::endl; \
      std::cout << "EXPECTED:" << std::endl << expected << std::endl; \
    } \
  } while (0); \





} // namespace alps

#endif // _ALPS_TEST_COMMON_HH_
