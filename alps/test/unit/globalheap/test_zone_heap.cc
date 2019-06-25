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
#include "test_heap_fixture.hh"

using namespace alps;

class ZoneHeapTest: public HeapTest {};

TEST_F(ZoneHeapTest, acquire)
{
    EXPECT_NE((Zone*) 0, zoneheap()->acquire_new_zone());
    EXPECT_NE((Zone*) 0, zoneheap()->acquire_new_zone());
    EXPECT_EQ((Zone*) 0, zoneheap()->acquire_new_zone());
}

TEST_F(ZoneHeapTest, acquire_release)
{
    Zone* zone[16];
    EXPECT_NE((Zone*) 0, zone[0] = zoneheap()->acquire_new_zone());
    EXPECT_NE((Zone*) 0, zone[1] = zoneheap()->acquire_new_zone());
    EXPECT_EQ((Zone*) 0, zoneheap()->acquire_new_zone());
    zoneheap()->release_zone(zone[0]);
    EXPECT_NE((Zone*) 0, zoneheap()->acquire_new_zone());
    EXPECT_EQ((Zone*) 0, zoneheap()->acquire_new_zone());
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
