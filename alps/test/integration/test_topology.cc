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

#include "mint/mint.h"
#include "gtest/gtest.h"
#include "alps/pegasus/pegasus.hh"

#include "pegasus/topology.hh"
#include "pegasus/topology_factory.hh"

#include "test_common.hh"

using namespace alps;

class TopologyTest : public RegionFileTest { 
public:
    void SetUp() {
        RegionFileTest::__SetUp(false); // no cleanup
        EXPECT_EQ(kErrorCodeOk, Pegasus::topology_factory()->construct("/lfs/dummy", &topo));
    }

    Topology* topo;
};


TEST_F(TopologyTest, nearest_ig)
{
    EXPECT_LE(topo->nearest_ig(), int(topo->max_interleave_group()));
}


TEST_F(TopologyTest, interleave_group)
{ 
    size_t      test_region_size = booksize() * 8;
    RegionFile* region_file;

    std::string region_name = "interleave_group-" + std::to_string(topo->nearest_ig());

    // set interleave group
    EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path(region_name).c_str(), S_IRUSR | S_IWUSR, &region_file));
    InterleaveGroup ig = topo->nearest_ig();
    std::vector<InterleaveGroup> vig{ig};
    EXPECT_EQ(kErrorCodeOk, region_file->set_interleave_group(0, test_region_size, vig));
    EXPECT_EQ(kErrorCodeOk, region_file->truncate(test_region_size));
    EXPECT_EQ(kErrorCodeOk, region_file->close());

    // verify interleave group assignment
    EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(test_path(region_name).c_str(), O_RDWR, &region_file));
    std::vector<InterleaveGroup> assigned_vig;
    EXPECT_EQ(kErrorCodeOk, region_file->interleave_group(0, test_region_size, &assigned_vig));
    EXPECT_EQ(test_region_size/booksize(), assigned_vig.size());
    for (size_t i=0; i<assigned_vig.size(); i++) {
        EXPECT_EQ(ig, assigned_vig[i]);
    }
    EXPECT_EQ(kErrorCodeOk, region_file->close());
}


int main(int argc, char **argv) {
    ::alps::init_integration_test_env<::alps::TestEnvironment>(argc, argv);
    mint_init(argc, argv);
    return RUN_ALL_TESTS();
}
