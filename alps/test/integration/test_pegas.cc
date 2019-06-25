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

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

#include "mint/mint.h"
#include "gtest/gtest.h"

#include "alps/pegasus/pegasus.hh"
#include "pegasus/topology.hh"
#include "pegasus/topology_factory.hh"

#include "test_common.hh"

using namespace alps;


class PegasTest : public RegionFileTest { 
public:
    void SetUp() {
        RegionFileTest::__SetUp(false); // no cleanup
        EXPECT_EQ(kErrorCodeOk, Pegasus::topology_factory()->construct("/lfs/dummy", &topo));
    }

    Topology* topo;
};


TEST_F(PegasTest, create_truncate)
{ 
    size_t      test_region_size = booksize() * 8;
    RegionFile* region_file;

    std::string region_name = "region_create-" + std::to_string(topo->nearest_ig());

    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path(region_name).c_str(), S_IRUSR | S_IWUSR, &region_file));
    EXPECT_EQ(kErrorCodeOk, region_file->truncate(test_region_size));
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}


int main(int argc, char **argv) {
    ::alps::init_integration_test_env<::alps::TestEnvironment>(argc, argv);
    mint_init(argc, argv);
    return RUN_ALL_TESTS();
}
