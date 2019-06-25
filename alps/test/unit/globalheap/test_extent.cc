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
#include "alps/common/error_stack.hh"
#include "globalheap/extentheap.hh"

using namespace alps;

TEST(Extent, merge_right_into_left_1)
{
    Extent ex1(0, 10);
    Extent ex2(5, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(15LLU, ex1.len());   
}

TEST(Extent, merge_right_into_left_2)
{
    Extent ex1(0, 10);
    Extent ex2(5, 5);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(10LLU, ex1.len());   
}

TEST(Extent, merge_right_into_left_3)
{
    Extent ex1(0, 10);
    Extent ex2(5, 4);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(10LLU, ex1.len());   
}

TEST(Extent, merge_right_into_left_4)
{
    Extent ex1(0, 10);
    Extent ex2(5, 6);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(11LLU, ex1.len());
}

TEST(Extent, merge_right_into_left_5)
{
    Extent ex1(0, 10);
    Extent ex2(0, 11);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(11LLU, ex1.len());
}

TEST(Extent, merge_left_into_right_1)
{
    Extent ex1(5, 10);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(15LLU, ex1.len());   
}

TEST(Extent, merge_left_into_right_2)
{
    Extent ex1(5, 5);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(10LLU, ex1.len());   
}

TEST(Extent, merge_left_into_right_3)
{
    Extent ex1(5, 4);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(10LLU, ex1.len());   
}

TEST(Extent, merge_left_into_right_4)
{
    Extent ex1(5, 6);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(11LLU, ex1.len());
}

TEST(Extent, merge_left_into_right_5)
{
    Extent ex1(0, 11);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(11LLU, ex1.len());
}

TEST(Extent, merge_consecutive_1)
{
    Extent ex1(0, 10);
    Extent ex2(10, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(20LLU, ex1.len());
}

TEST(Extent, merge_consecutive_2)
{
    Extent ex1(10, 10);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(20LLU, ex1.len());
}

TEST(Extent, merge_no_1)
{
    Extent ex1(0, 10);
    Extent ex2(11, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(0LLU, ex1.start());   
    EXPECT_EQ(10LLU, ex1.len());
}

TEST(Extent, merge_no_2)
{
    Extent ex1(11, 10);
    Extent ex2(0, 10);
    ex1.merge(&ex2);
    EXPECT_EQ(11LLU, ex1.start());   
    EXPECT_EQ(10LLU, ex1.len());
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
