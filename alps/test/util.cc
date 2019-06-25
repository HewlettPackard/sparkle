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

#include <unistd.h> 
#include <iostream>
#include <iomanip>  
#include <string> 
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "alps/pegasus/pegasus.hh"

#include "test_common.hh"

using namespace alps;

#define PROGNAME argv[0]

int usage(std::string progname, boost::program_options::options_description desc, int rc = 0)
{
    std::cerr << "Usage: " << progname << " [options] command" << std::endl 
              << desc << std::endl;
    return rc; 
}

/******************************************************************************
 * COMMAND: cleanup  
 ******************************************************************************/

int cleanup(std::string test_dir)
{
    int ret;

    TestOptions test_options;
    test_options.test_dir = test_dir;

    TestEnvironment* env = new TestEnvironment(test_options);
    ::testing::AddGlobalTestEnvironment(env);

    std::cout << "Cleaning up " << test_dir << " ..." << std::endl;
    env->cleanup_fs(); 

    return 0;
}

int cmd_cleanup(std::string progname, boost::program_options::parsed_options& parsed, boost::program_options::variables_map& vm)
{
    namespace po = boost::program_options; 
    po::options_description desc("create options");
    try {
        size_t heap_size = 0;
        size_t metazone_size = 0;
        desc.add_options()
            ("test_dir", po::value<std::string>()->required(), "Directory to cleanup");

        if (vm.count("help")) {
            return usage(progname, desc, 0);
        }

        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());
        po::store(po::command_line_parser(opts).options(desc).run(), vm);
        po::notify(vm); // throws on error, so do after help in case 
                        // there are any problems 
        std::string test_dir = vm["test_dir"].as<std::string>();
        return cleanup(test_dir);
    }
    catch(po::error& e) 
    { 
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
        return usage(progname, desc, 0);
    } 
    return -1;
}

/******************************************************************************
 ******************************************************************************
 ******************************************************************************/

int main(int argc, char** argv) 
{ 
    namespace po = boost::program_options; 
    std::string progname = PROGNAME;
    po::options_description desc("Options"); 

    try 
    { 
        int ret;
        desc.add_options() 
            ("help", "Print help messages") 
            ("config", po::value<std::string>(), "File to load Alps/Pegasus configuration options from") 
            ("log_level", po::value<std::string>()->default_value("warning"), "Log messages at or above this level: INFO, WARNING, ERROR, and FATAL")
            ("command", po::value<std::string>()->required(), "Command to execute: [cleanup]")
            ("subargs", po::value<std::vector<std::string> >(), "Arguments for command");

        po::positional_options_description positionalOptions; 
        positionalOptions.add("command", 1); 
        positionalOptions.add("subargs", -1); 
 
        po::variables_map vm; 
        try 
        { 
            po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc) 
                                          .positional(positionalOptions).allow_unregistered().run(); 
            po::store(parsed, vm);

            if (vm.count("help") && !vm.count("command")) { 
                return usage(progname, desc);
            } 
 
            po::notify(vm); // throws on error, so do after help in case 
                            // there are any problems 
             
            std::string cmd = vm["command"].as<std::string>();

            // Initialize Pegasus
            PegasusOptions pgopt;
            const char* config_file = NULL;
            if (vm.count("config")) {
                config_file = vm["config"].as<std::string>().c_str();
            } 
            Pegasus::load_options(config_file, true, true, &pgopt);
            if (vm.count("log_level")) {
                std::string log_level = vm["log_level"].as<std::string>();
                std::transform(log_level.begin(), log_level.end(), log_level.begin(), ::tolower);
                pgopt.debug_options.log_level = log_level;
            }
            Pegasus::init(pgopt);

            if (cmd == "cleanup") {
                ret = cmd_cleanup(progname, parsed, vm);
            } else {
                std::cout << "ERROR: Unrecognized command " << cmd << std::endl;
                ret = -1;
            }

            if (ret) {
                std::cout << "ERROR: Command " << cmd << " failed" << std::endl;
                return -1;
            }
        }
        catch(po::error& e) 
        { 
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
            return usage(progname, desc, -1);
        } 
    } 
    catch(std::exception& e) 
    { 
        std::cerr << "Unhandled Exception reached the top of main: " 
                  << e.what() << ", application will now exit" << std::endl; 
        return usage(progname, desc, -1);
    } 
    return 0; 
}
