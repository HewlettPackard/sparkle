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
#include "alps/common/error_stack.hh"
#include "pegasus/lfs_topology.hh"

using namespace alps;

TEST(TopologyTest, zhop_distance)
{
    FRDNode n1(1,1,4);   
    FRDNode n2(1,1,5);   
    FRDNode n3(1,4,5);   

    EXPECT_EQ(3U, n1-n2);
    EXPECT_EQ(5U, n1-n3);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
