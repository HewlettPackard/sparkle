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

#include <fcntl.h>
#include "alps/pegasus/address_space.hh"
#include "test_common.hh"
#include "helper_functions.hh"

using namespace alps;

#define NUM_REGION_FILES 4
#define REGION_FILE_SIZE (2*booksize())


class MapEnvironment: public TestEnvironment {
public:
    MapEnvironment(TestOptions& test_options)
        : TestEnvironment(test_options)
    { }
 
    void SetUp() {
        TestEnvironment::SetUp();
        size_t  region_file_size = REGION_FILE_SIZE;
        create_region_file_paths(test_path("test_region"), NUM_REGION_FILES, &region_file_paths, &region_file_paths_c);
        create_region_files(region_file_paths, region_file_size, S_IRUSR | S_IWUSR, true, true, booksize());
    }

    void TearDown() { }

protected:
    std::vector<boost::filesystem::path> region_file_paths;
    const char**                         region_file_paths_c;

};

class MapMultiRegionFileTest: public RegionFileTest {
public:

    void SetUp() {
        RegionFileTest::__SetUp(false);
        create_region_file_paths(test_path("test_region"), NUM_REGION_FILES, &region_file_paths, &region_file_paths_c);
    }

protected:
    std::vector<boost::filesystem::path> region_file_paths;
    const char**                         region_file_paths_c;
};

TEST_F(MapMultiRegionFileTest, map_whole)
{
    RegionFile* region_file;
    size_t      region_size = REGION_FILE_SIZE * NUM_REGION_FILES;
    void*       mapped_addr;

    size_t length = region_size;
    loff_t offset = 0;

    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(region_file_paths_c, NUM_REGION_FILES, O_RDWR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->map(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, offset, &mapped_addr));
    EXPECT_EQ(kErrorCodeOk, region_file->unmap(mapped_addr, length));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}

TEST_F(MapMultiRegionFileTest, map_partial_case1)
{
    RegionFile* region_file;
    size_t      region_size = REGION_FILE_SIZE * NUM_REGION_FILES;
    void*       mapped_addr;

    size_t length = region_size - booksize()/2;
    loff_t offset = 0;

    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(region_file_paths_c, NUM_REGION_FILES, O_RDWR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->map(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, offset, &mapped_addr));
    EXPECT_EQ(kErrorCodeOk, region_file->unmap(mapped_addr, length));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}

TEST_F(MapMultiRegionFileTest, map_partial_case2)
{
    RegionFile* region_file;
    size_t      region_size = REGION_FILE_SIZE * NUM_REGION_FILES;
    void*       mapped_addr;

    size_t length = region_size - booksize()/2;
    loff_t offset = booksize()/2;

    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(region_file_paths_c, NUM_REGION_FILES, O_RDWR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->map(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, offset, &mapped_addr));
    EXPECT_EQ(kErrorCodeOk, region_file->unmap(mapped_addr, length));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}

TEST_F(MapMultiRegionFileTest, map_partial_case3)
{
    RegionFile* region_file;
    size_t      region_size = REGION_FILE_SIZE * NUM_REGION_FILES;
    void*       mapped_addr;

    size_t length = region_size - booksize();
    loff_t offset = booksize()/2;

    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(region_file_paths_c, NUM_REGION_FILES, O_RDWR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->map(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, offset, &mapped_addr));
    EXPECT_EQ(kErrorCodeOk, region_file->unmap(mapped_addr, length));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<MapEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
