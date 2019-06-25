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

#define REGION_FILE_SIZE (8*booksize())


TEST_F(RegionFileTest, create)
{
    RegionFile* region_file;

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("test_region").c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}

TEST_F(RegionFileTest, create_truncate)
{
    RegionFile* region_file;
    size_t      region_size = REGION_FILE_SIZE;

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("test_region").c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->truncate(region_size));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
