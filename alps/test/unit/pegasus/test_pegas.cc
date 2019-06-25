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

#include <stdlib.h>
#include <sys/fcntl.h>
#include <string>
#include <sstream>
#include <vector>
#include "gtest/gtest.h"
#include "alps/pegasus/address_space.hh"
#include "test_common.hh"

using namespace alps;

#define MAX_INTERLEAVE_REQUEST_SIZE 1024

class PegasTest : public RegionFileTest { };


TEST_F(PegasTest, create)
{ 
    RegionFile* region_file;

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("region_create").c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
    EXPECT_EQ(0, unlink(test_path("region_create").c_str()));
}

TEST_F(PegasTest, create_truncate)
{ 
    size_t      test_region_size = booksize() * 8;
    RegionFile* region_file;

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("region_create").c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->truncate(test_region_size));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
    EXPECT_EQ(0, unlink(test_path("region_create").c_str()));
}

TEST_F(PegasTest, interleave_create)
{ 
    //size_t   test_region_size = booksize * 8;
    //RRegion* region;
    //Pegas    pegas;

    //EXPECT_EQ(kErrorCodeOk, pegas.init(NULL));
    //std::vector<InterleaveGroup> vig {0x01, 0x02};
    //EXPECT_EQ(kErrorCodeOk, pegas.create(test_path("region_create").c_str(), test_region_size, &vig, &pregion));
    //EXPECT_EQ(kErrorCodeOk, pegas.close(pregion));
}

TEST_F(PegasTest, create_open)
{
    RegionFile* region_file;

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("region_create").c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(test_path("region_create").c_str(), O_RDWR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
    EXPECT_EQ(0, unlink(test_path("region_create").c_str()));
}

TEST_F(PegasTest, interleave_create_open)
{
    //size_t   test_region_size = booksize * 8;
    //RRegion* region;
    //Pegas    pegas;

    //EXPECT_EQ(kErrorCodeOk, pegas.init(NULL));
    //std::vector<InterleaveGroup> vig {0x01, 0x02};
    //EXPECT_EQ(kErrorCodeOk, pegas.create(test_path("region_create").c_str(), test_region_size, &vig, &pregion));
    //EXPECT_EQ(kErrorCodeOk, pegas.close(pregion));
    //EXPECT_EQ(kErrorCodeOk, pegas.open(test_path("region_create").c_str(), &pregion));
    //EXPECT_EQ(kErrorCodeOk, pegas.close(pregion));
}

TEST_F(PegasTest, create_open_map)
{
    RegionFile* region_file;
    RRegion*    region;
    size_t      test_region_size = booksize() * 8;

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("region_create").c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->truncate(test_region_size));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(test_path("region_create").c_str(), O_RDWR, &region_file));
    EXPECT_EQ(kErrorCodeOk, Pegasus::address_space()->map(region_file, &region));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
    EXPECT_EQ(0, unlink(test_path("region_create").c_str()));
}

TEST_F(PegasTest, create_open_map_multiple_files)
{
#if 0
    int                       num_region_files = 4;
    std::vector<std::string>  region_file_path;
    std::vector<RegionFile*>  region_file_handle;
    RRegion*                  region;
    size_t                    test_region_size = booksize;
    loff_t                    new_region_size;
    RegionFileList            rfl;

    for (int i=0; i < num_region_files; i++) {
        std::stringstream ss;
        ss << "region_create_" << i;
        std::string region_file_basename = ss.str();
        region_file_path.push_back(test_path(region_file_basename.c_str()));
    }

    Pegasus::init(NULL);
    region_file_handle.resize(num_region_files);
    for (int i=0; i < num_region_files; i++) {
        unlink(region_file_path[i].c_str());
        EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(region_file_path[i].c_str(), S_IRUSR | S_IWUSR, &region_file_handle[i]));
        EXPECT_EQ(kErrorCodeOk, region_file_handle[i]->truncate(test_region_size, &new_region_size));
        EXPECT_EQ(kErrorCodeOk, region_file_handle[i]->close());
    }
    for (int i=0; i < num_region_files; i++) {
        EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(region_file_path[i].c_str(), O_RDWR, &region_file_handle[i]));
        rfl.push_back(region_file_handle[i]);
    }
    EXPECT_EQ(kErrorCodeOk, Pegasus::address_space()->map(rfl, &region));
    sleep(3);
    EXPECT_EQ(kErrorCodeOk, Pegasus::address_space()->unmap(region));
    std::cout << "UNMAP: DONE" << std::endl;
    sleep(10);
    for (int i=0; i < num_region_files; i++) {
        EXPECT_EQ(kErrorCodeOk, region_file_handle[i]->close());
        unlink(region_file_path[i].c_str());
    }
#endif
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
