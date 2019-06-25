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
#include "gtest/gtest.h"
#include "alps/common/error_stack.hh"
#include "alps/pegasus/address_space.hh"
#include "test_heap_fixture.hh"

using namespace alps;

class ExtentHeapTest: public HeapTest {};

TEST_F(ExtentHeapTest, test1)
{
    RRegion::TPtr<void> e1 = extentheap()->malloc(28*256*1024);
    RRegion::TPtr<void> e2 = extentheap()->malloc(2*256*1024);
    RRegion::TPtr<void> e3 = extentheap()->malloc(256*1024);

    EXPECT_NE(null_ptr, e1);
    EXPECT_NE(null_ptr, e2);
    EXPECT_NE(null_ptr, e3);

    extentheap()->free(e1);
    extentheap()->free(e2);
    extentheap()->free(e3);
}


int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
