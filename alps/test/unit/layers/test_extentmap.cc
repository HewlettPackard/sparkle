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
#include <sstream>
#include "gtest/gtest.h"
#include "alps/layers/bits/extentmap.hh"
#include "test_common.hh"

using namespace alps;


// Tests with a single extent

TEST(ExtentMap, insert_single)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));

    EXPECT_OUTPUT(map, ExtentInterval(10, 10) << std::endl);
}

// Merge tests with two extents

TEST(ExtentMap, no_merge_lowest_with_next)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(0, 9));

    EXPECT_OUTPUT(map, 
        ExtentInterval(0, 9) << std::endl <<
        ExtentInterval(10, 10) << std::endl);
}

TEST(ExtentMap, merge_lowest_with_next)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(0, 10));

    EXPECT_OUTPUT(map, ExtentInterval(0, 20) << std::endl);
}

TEST(ExtentMap, no_merge_highest_with_prev)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(21, 10));

    EXPECT_OUTPUT(map, 
        ExtentInterval(10, 10) << std::endl <<
        ExtentInterval(21, 10) << std::endl);
}

TEST(ExtentMap, merge_highest_with_prev)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(20, 10));

    EXPECT_OUTPUT(map, ExtentInterval(10, 20) << std::endl);
}

// Merge tests with three extents

TEST(ExtentMap, merge_middle_with_prev)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(40, 10));
    map.insert(ExtentInterval(20, 10));

    EXPECT_OUTPUT(map, 
        ExtentInterval(10, 20) << std::endl <<
        ExtentInterval(40, 10) << std::endl);
}

TEST(ExtentMap, no_merge_middle_with_prev)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(40, 10));
    map.insert(ExtentInterval(21, 10));

    EXPECT_OUTPUT(map, 
        ExtentInterval(10, 10) << std::endl <<
        ExtentInterval(21, 10) << std::endl <<
        ExtentInterval(40, 10) << std::endl);
}

TEST(ExtentMap, merge_lowest_with_next_2)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(40, 10));
    map.insert(ExtentInterval(0, 10));

    EXPECT_OUTPUT(map, 
        ExtentInterval(0, 20) << std::endl <<
        ExtentInterval(40, 10) << std::endl);
}

TEST(ExtentMap, merge_highest_with_prev_2)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(40, 10));
    map.insert(ExtentInterval(50, 10));

    EXPECT_OUTPUT(map, 
        ExtentInterval(10, 10) << std::endl <<
        ExtentInterval(40, 20) << std::endl);
}

TEST(ExtentMap, merge_middle_with_prev_and_next)
{
    ExtentMap map;
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(40, 10));
    map.insert(ExtentInterval(20, 20));

    EXPECT_OUTPUT(map, 
        ExtentInterval(10, 40) << std::endl);
}

// Remove tests 

TEST(ExtentMap, remove_ge1)
{
    ExtentMap map;
    map.insert(ExtentInterval(0, 10));
    map.insert(ExtentInterval(30, 10));
    map.insert(ExtentInterval(10, 10));
    map.insert(ExtentInterval(80, 10));
    map.insert(ExtentInterval(40, 10));

    EXPECT_OUTPUT(map, 
        ExtentInterval(0, 20) << std::endl <<
        ExtentInterval(30, 20) << std::endl << 
        ExtentInterval(80, 10) << std::endl);

    ExtentInterval e;
    EXPECT_EQ(0, map.remove_ge(10, &e));
    EXPECT_EQ(ExtentInterval(80, 10), e);
    EXPECT_EQ(0, map.remove_ge(15, &e));
    EXPECT_EQ(ExtentInterval(0, 20), e);
}

TEST(ExtentMap, remove_ge2)
{
    ExtentMap map;
    map.insert(ExtentInterval(0, 10));
    map.insert(ExtentInterval(30, 10));
    map.insert(ExtentInterval(50, 10));
    map.insert(ExtentInterval(40, 10));

    ExtentInterval e;
    EXPECT_EQ(0, map.remove_ge(30, &e));
    EXPECT_EQ(ExtentInterval(30, 30), e);
    EXPECT_NE(0, map.remove_ge(30, &e));
    EXPECT_NE(0, map.remove_ge(11, &e));
    EXPECT_EQ(0, map.remove_ge(9, &e));
    EXPECT_EQ(ExtentInterval(0, 10), e);
}

TEST(ExtentMap, remove_ge3)
{
    ExtentMap map;
    map.insert(ExtentInterval(30, 2));
    map.insert(ExtentInterval(25, 3));
    map.insert(ExtentInterval(0, 1));

    ExtentInterval e;

    EXPECT_EQ(0, map.find_ge(1, &e));
    EXPECT_EQ(e, ExtentInterval(0, 1));

    // returns (30,2) because internally extentmap look ups in the 
    // order by length index and (30, 2) comes before (25, 3) in
    // that index
    EXPECT_EQ(0, map.find_ge(2, &e));
    EXPECT_EQ(e, ExtentInterval(30, 2));

    EXPECT_EQ(0, map.find_ge(3, &e));
    EXPECT_EQ(e, ExtentInterval(25, 3));
}

int main(int argc, char** argv)
{
    //::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
