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

#include "alps/common/assorted_func.hh"
#include "alps/pegasus/pegasus.hh"

#include "globalheap/globalheap_internal.hh"
#include "globalheap/layout.hh"
#include "globalheap/nvslab.hh"
#include "globalheap/size_class.hh"
#include "globalheap/zone.hh"

using namespace alps;

#define PROGNAME argv[0]

int usage(std::string progname, boost::program_options::options_description desc, int rc = 0)
{
    std::cerr << "Usage: " << progname << " [options] command" << std::endl 
              << desc << std::endl;
    return rc; 
}

std::list<ZoneId> expand_zones(GlobalHeapInternal* heap, std::string zones)
{

    if (zones == "all") {
        assert(heap);
        RRegion::TPtr<nvHeap> nvheap = heap->nvheap();
        size_t nzones = nvheap->nzones();
        std::stringstream ss;
        ss << "0-" << nzones-1;
        zones = ss.str();
    }

    return expand_range(zones);
}

boost::filesystem::path heap_partition_path(boost::filesystem::path heap_path_prefix, int partition)
{
    std::stringstream ss;
    ss << heap_path_prefix.string() << "-" << partition;
    return boost::filesystem::path(ss.str());
}

std::vector<boost::filesystem::path> heap_paths(boost::filesystem::path heap_path_prefix, int npartitions)
{
    std::vector<boost::filesystem::path> paths;

    if (npartitions > 1) {
        for (int i=0; i<npartitions; i++) {
            paths.push_back(heap_partition_path(heap_path_prefix, i));
        }
    } else {
        paths.push_back(heap_path_prefix);
    }

    return paths;
}

std::vector<boost::filesystem::path> find_existing_heap_paths(boost::filesystem::path heap_path_prefix)
{
    std::vector<boost::filesystem::path> paths;

    if (boost::filesystem::exists(heap_path_prefix)) {
        paths.push_back(heap_path_prefix);
    } else {
        for (int i=0;; i++) {
            boost::filesystem::path path = heap_partition_path(heap_path_prefix, i);
            if (!boost::filesystem::exists(path)) {
                break;
            }
            paths.push_back(path);
        }
    }
    return paths;
}

bool heap_exists(boost::filesystem::path heap_path_prefix)
{
    std::vector<boost::filesystem::path> existing_paths = find_existing_heap_paths(heap_path_prefix);
    return (existing_paths.size() > 0);
}

/******************************************************************************
 * CREATE HEAP
 ******************************************************************************/

int create_heap(std::string heap_path, size_t heap_size, size_t metazone_size, int npartitions)
{
    int ret;
    GlobalHeapInternal* heap;

    if (heap_exists(heap_path)) {
        std::cout << "ERROR: Heap " << heap_path << " already exists!" << std::endl;
        return -1;
    }

    std::cout << "Create heap: " << "heap_path=" << heap_path
              << ", heap_size=" << heap_size
              << ", metazone_size=" << metazone_size << std::endl;

    if (npartitions > 1) {
        std::vector<boost::filesystem::path> paths = heap_paths(heap_path, npartitions);
        if ((ret = GlobalHeapInternal::create(paths, heap_size, metazone_size, &heap)) != 0) {
            return ret;
        }
    } else {
        if ((ret = GlobalHeapInternal::create(heap_path, heap_size, metazone_size, &heap)) != 0) {
            return ret;
        }
    }
    return heap->close();
}

int cmd_create(std::string progname, boost::program_options::parsed_options& parsed, boost::program_options::variables_map& vm)
{
    namespace po = boost::program_options; 
    po::options_description desc("create options");
    try {
        size_t heap_size = 0;
        size_t metazone_size = 0;
        desc.add_options()
            ("size", po::value<std::string>()->required(), "Heap size in bytes")
            ("heappath", po::value<std::string>()->required(), "Heap path")
            ("metazone_size", po::value<std::string>()->default_value("8G"), "Metazone size in bytes (must be power of two)")
            ("partitions", po::value<int>()->default_value(1), "Partition the heap into so many files");

        if (vm.count("help")) {
            return usage(progname, desc, 0);
        }

        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());
        po::store(po::command_line_parser(opts).options(desc).run(), vm);
        po::notify(vm); // throws on error, so do after help in case 
                        // there are any problems 
        std::string heap_path = vm["heappath"].as<std::string>();
        heap_size = string_to_size(vm["size"].as<std::string>());
        metazone_size = string_to_size(vm["metazone_size"].as<std::string>());
        return create_heap(heap_path, heap_size, metazone_size, vm["partitions"].as<int>());
    }
    catch(po::error& e) 
    { 
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
        return usage(progname, desc, 0);
    } 
    return -1;
}



/******************************************************************************
 * REMOVE HEAP
 ******************************************************************************/

int remove_heap(std::string heap_path)
{
    if (!heap_exists(heap_path)) {
        std::cout << "ERROR: Heap " << heap_path << " does not exist!" << std::endl;
        return -1;
    }
    std::vector<boost::filesystem::path> paths = find_existing_heap_paths(heap_path);

    for (std::vector<boost::filesystem::path>::iterator it = paths.begin(); 
         it != paths.end(); 
         it++)
    {
        boost::filesystem::path path = *it;
        boost::filesystem::remove(path);
        std::cout << "Removing file: " << path.string() << std::endl;
        boost::filesystem::path xpath = boost::filesystem::path(path.string() + ".xattr");
        if (boost::filesystem::exists(xpath)) {
            std::cout << "Removing file: " << xpath.string() << std::endl;
            boost::filesystem::remove(xpath);
        }
    }

    return 0;
}

int cmd_remove(std::string progname, boost::program_options::parsed_options& parsed, boost::program_options::variables_map& vm)
{
    namespace po = boost::program_options; 
    po::options_description desc("format options");
    try {
        desc.add_options()
            ("heappath", po::value<std::string>()->required(), "Heap path");

        if (vm.count("help")) {
            return usage(progname, desc, 0);
        }

        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());
        po::store(po::command_line_parser(opts).options(desc).run(), vm);
        po::notify(vm); // throws on error, so do after help in case 
                        // there are any problems 
        std::string heappath = vm["heappath"].as<std::string>();
        return remove_heap(heappath);
    }
    catch(po::error& e) 
    { 
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
        return usage(progname, desc, 0);
    } 
    return -1;
}



/******************************************************************************
 * FORMAT HEAP
 ******************************************************************************/

int format_heap(std::string heap_path, std::string zones)
{
    if (!heap_exists(heap_path)) {
        std::cout << "ERROR: Heap " << heap_path << " does not exist!" << std::endl;
        return -1;
    }
    std::vector<boost::filesystem::path> paths = find_existing_heap_paths(heap_path);

    if (zones == "all") {
        return GlobalHeapInternal::format(paths);
    } else {
        std::list<ZoneId> zone_list = expand_zones(NULL, zones);
        std::vector<ZoneId> zone_vec{ std::begin(zone_list), std::end(zone_list) };
        return GlobalHeapInternal::format(paths, zone_vec);
    }    

    return 0;
}

int format_heap_instance(std::string heap_path, GlobalHeapInternal::InstanceId instance_id)
{
    if (!heap_exists(heap_path)) {
        std::cout << "ERROR: Heap " << heap_path << " does not exist!" << std::endl;
        return -1;
    }
    std::vector<boost::filesystem::path> paths = find_existing_heap_paths(heap_path);

    return GlobalHeapInternal::format_instance(paths, instance_id);
}

int cmd_format(std::string progname, boost::program_options::parsed_options& parsed, boost::program_options::variables_map& vm)
{
    namespace po = boost::program_options; 
    po::options_description desc("format options");
    try {
        desc.add_options()
            ("zones", po::value<std::string>()->default_value("all")->required(), "zones to format")
            ("heappath", po::value<std::string>()->required(), "Heap path")
            ("instance", po::value<size_t>(), "instance to format");

        if (vm.count("help")) {
            return usage(progname, desc, 0);
        }

        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());
        po::store(po::command_line_parser(opts).options(desc).run(), vm);
        po::notify(vm); // throws on error, so do after help in case 
                        // there are any problems 
        std::string heappath = vm["heappath"].as<std::string>();
        std::string zones = vm["zones"].as<std::string>();
        if (vm.count("instance")) {
            return format_heap_instance(heappath, vm["instance"].as<size_t>());
        } else {
            return format_heap(heappath, zones);
        }
    }
    catch(po::error& e) 
    { 
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
        return usage(progname, desc, 0);
    } 
    return -1;

}



/******************************************************************************
 * REPORT HEAP STATISTICS
 ******************************************************************************/

// Keep usage statistics per block size
class BlockStats {
public:
    BlockStats(size_t block_size, size_t nblocks, size_t nblocks_free)
        : block_size_(block_size),
          nblocks_(nblocks),
          nblocks_free_(nblocks_free)
    { }
    
    void add_stat(size_t nblocks, size_t nblocks_free)
    {
        nblocks_ += nblocks;
        nblocks_free_ += nblocks_free;
    }
    
    size_t block_size_;
    size_t nblocks_;
    size_t nblocks_free_;
};

std::string perc(size_t num, size_t den, bool format = true)
{
    std::stringstream ss;
    if (format) {
        ss << "(" << (100*num) / den << "%)";
    } else {
        ss << (100*num) / den;
    }
    return ss.str();
}

std::string with_perc(size_t num, size_t den)
{
    std::stringstream ss;
    ss << num << " " << perc(num, den);
    return ss.str();
}


// Summarize per-zone block statistics
class ZoneStats {
    typedef std::map<size_t, BlockStats> BlockStatsMap;
public:
    ZoneStats(size_t total_size, RRegion::TPtr<nvZone> nvzone = null_ptr)
        : total_size_(total_size),
          effective_size_(0),
          free_size_(0),
          nvzone_(nvzone)
    { }

    ZoneStats(RRegion::TPtr<nvHeap> nvheap, ZoneId zid)
        : effective_size_(0),
          free_size_(0)
    {
        total_size_ = nvheap->metazone_size();
        nvzone_ = nvheap->zone(zid);
        summarize_zone_stats();
    }

    void add_stat(size_t block_size, size_t nblocks, size_t nblocks_free)
    {
        BlockStatsMap::iterator it = blocks.find(block_size);
        if (it != blocks.end()) {
            BlockStats& blockstats = it->second;
            blockstats.add_stat(nblocks, nblocks_free);
        } else {
            blocks.insert(std::pair<size_t, BlockStats>(block_size, BlockStats(block_size, nblocks, nblocks_free)));
        }
        effective_size_ += nblocks*block_size;
        free_size_ += nblocks_free*block_size;
    }

    void add_stat(const BlockStats& blockstats)
    {
        add_stat(blockstats.block_size_, blockstats.nblocks_, blockstats.nblocks_free_);
    }

    void add_stat(const ZoneStats& other)
    {
        total_size_ += other.total_size_;
        for (BlockStatsMap::const_iterator it = other.blocks.begin(); it != other.blocks.end(); it++) {
            const BlockStats& other_blockstats = it->second;
            add_stat(other_blockstats);
        }
    }

    void summarize_zone_stats()
    {
        size_t nblocks = nvzone_->nblocks();
        Extent extent;
        bool extent_is_free;
        for (size_t next_start = 0;
             find_extent(nvzone_, next_start, nblocks, &extent, &extent_is_free) < nblocks; )
        {
            next_start = extent.start() + extent.len();
            RRegion::TPtr<nvExtentHeader> exhdr = static_cast<RRegion::TPtr<nvExtentHeader>>(nvzone_->block_header(extent.start()));
            if (!extent_is_free) {
                if (exhdr->secondary_type == nvExtentHeader::kExtentTypeSlab) {
                    RRegion::TPtr<nvSlab> nvslab = static_cast<RRegion::TPtr<nvSlab>>(nvzone_->block(extent.start()));
                    add_stat(nvslab->block_size(), nvslab->nblocks(), nvslab->nblocks_free());
                } else {
                    add_stat(exhdr->size * BLOCK_SIZE, 1, 0);
                }
            } else {
                add_stat(extent.len() * BLOCK_SIZE, 1, 1);
            }
        }
    }

    void stream_to(std::ostream& os) const {
        os << std::setw(20) << std::left << "Size: " << total_size_ << std::endl;
        os << std::setw(20) << std::left << "Effective size: " << effective_size_ << std::endl;
        os << std::setw(20) << std::left << "Used size: " << effective_size_ - free_size_ 
           << " " << perc(effective_size_ - free_size_, effective_size_) << std::endl;
        os << std::setw(20) << std::left << "Free size: " << free_size_ 
           << " " << perc(free_size_, effective_size_) << std::endl;
        if (nvzone_ != null_ptr) {
            os << std::setw(20) << std::left << "Owner: " << size_t(nvzone_->header.lease.lock_status()) << std::endl;
        }

        os << std::endl;
        os << "Per-block stats:" << std::endl;
        os << std::setw(20) << std::left << "Block Size";
        os << std::setw(20) << std::left << "#Free Blocks";
        os << std::setw(20) << std::left << "%Free Blocks";
        os << std::setw(20) << std::left << "#Total Blocks";
        os << std::endl;
        for (BlockStatsMap::const_iterator it = blocks.begin(); it != blocks.end(); it++) {
            const BlockStats& blockstats = it->second;
            os << std::setw(20) << std::left << blockstats.block_size_;
            os << std::setw(20) << std::left << blockstats.nblocks_free_;
            os << std::setw(20) << std::left << perc(blockstats.nblocks_free_, blockstats.nblocks_, false);
            os << std::setw(20) << std::left << blockstats.nblocks_;
            os << std::endl;
        }
    }

    BlockStatsMap blocks;
    size_t total_size_;
    size_t effective_size_;
    size_t free_size_;
    RRegion::TPtr<nvZone> nvzone_;
};

inline std::ostream& operator<<(std::ostream& os, const ZoneStats& zonestats)
{
    zonestats.stream_to(os);
    return os;
}


int report_heap_stats(std::string heap_path, std::string zones, bool per_zone)
{
    int rc;
    GlobalHeapInternal* heap;

    if (!heap_exists(heap_path)) {
        std::cout << "ERROR: Heap " << heap_path << " does not exist!" << std::endl;
        return -1;
    }
    std::vector<boost::filesystem::path> paths = find_existing_heap_paths(heap_path);

    if (paths.size() > 1) {
        if ((rc = GlobalHeapInternal::open(paths, &heap)) != 0) {
            return rc;
        }
    } else {
        if ((rc = GlobalHeapInternal::open(paths[0], &heap)) != 0) {
            return rc;
        }
    }
    RRegion::TPtr<nvHeap> nvheap = heap->nvheap();

    ZoneStats zs_all(0);
    std::list<ZoneId> zoneids = expand_zones(heap, zones);
    for (std::list<ZoneId>::iterator it = zoneids.begin(); it != zoneids.end(); it++) {
        ZoneId zid = *it;
        ZoneStats zs(nvheap, zid);
        zs_all.add_stat(zs);
        if (per_zone) {
            std::cout << "ZONE: " << zid << std::endl;
            std::cout << zs << std::endl;
        }
    }

    std::cout << "SUMMARY" << std::endl;
    std::cout << zs_all << std::endl;
    return heap->close();
}

int cmd_report(std::string progname, boost::program_options::parsed_options& parsed, boost::program_options::variables_map& vm)
{
    namespace po = boost::program_options; 
    po::options_description desc("report options");
    try {
        desc.add_options()
            ("zones", po::value<std::string>()->default_value("all")->required(), "Zones to report statistics for")
            ("heappath", po::value<std::string>()->required(), "Heap path")
            ("perzone", po::value<bool>()->default_value(false), "Report per zone statistics");

        if (vm.count("help")) {
            return usage(progname, desc, 0);
        }

        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());
        po::store(po::command_line_parser(opts).options(desc).run(), vm);
        po::notify(vm); // throws on error, so do after help in case 
                        // there are any problems 
        std::string heappath = vm["heappath"].as<std::string>();
        std::string zones = vm["zones"].as<std::string>();
        bool perzone = vm["perzone"].as<bool>();
        return report_heap_stats(heappath, zones, perzone);
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
            ("command", po::value<std::string>()->required(), "Command to execute: [create, format, report]")
            ("heappath", po::value<std::string>(), "Heap path")
            ("subargs", po::value<std::vector<std::string> >(), "Arguments for command");

        po::positional_options_description positionalOptions; 
        positionalOptions.add("command", 1); 
        positionalOptions.add("heappath", 1); 
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

            if (cmd == "create") {
                ret = cmd_create(progname, parsed, vm);
            } else if (cmd == "remove") {
                ret = cmd_remove(progname, parsed, vm);
            } else if (cmd == "format") {
                ret = cmd_format(progname, parsed, vm);
            } else if (cmd == "report") {
                ret = cmd_report(progname, parsed, vm);
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
