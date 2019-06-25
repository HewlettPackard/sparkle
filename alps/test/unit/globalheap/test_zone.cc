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
#include "alps/pegasus/relocatable_region.hh"
#include "globalheap/freespacemap.hh"
#include "globalheap/layout.hh"
#include "globalheap/zone.hh"
#include "test_heap_fixture.hh"

using namespace alps;

class ZoneTest: public HeapTest {};


TEST_F(ZoneTest, alloc_extent)
{
    Zone* zone;
    EXPECT_NE((Zone*) NULL, zone = zoneheap()->acquire_new_zone());
    size_t MAX_BLOCK = zone->nvzone()->nblocks();

    RRegion::TPtr<void> e1;
    RRegion::TPtr<nvExtentHeader> eh1;
    zone->alloc_extent(1, &eh1, &e1);

    RRegion::TPtr<void> e2;
    RRegion::TPtr<nvExtentHeader> eh2;
    zone->alloc_extent(2, &eh2, &e2);

    RRegion::TPtr<void> e3;
    RRegion::TPtr<nvExtentHeader> eh3;
    zone->alloc_extent(3, &eh3, &e3);

    // check allocations/frees are properly propagated to the persistent structure
    {
        FreeSpaceMap fsmap_shadow(zone->nvzone());
        fsmap_shadow.init();
        EXPECT_EQ_VERBOSE(*zone->fsmap(), fsmap_shadow);
    }
    
    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(Extent(6, MAX_BLOCK-6));
        EXPECT_EQ_VERBOSE(fsmap_expect, *zone->fsmap());
    }

    zone->free_extent(e1);

    // check allocations/frees are properly propagated to the persistent structure
    {
        FreeSpaceMap fsmap_shadow(zone->nvzone());
        fsmap_shadow.init();
        EXPECT_EQ_VERBOSE(*zone->fsmap(), fsmap_shadow);
    }
    
    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(Extent(0, 1));
        fsmap_expect.insert(Extent(6, MAX_BLOCK-6));
        EXPECT_EQ_VERBOSE(fsmap_expect, *zone->fsmap());
    }

    RRegion::TPtr<void> e4;
    RRegion::TPtr<nvExtentHeader> eh4;
    zone->alloc_extent(4, &eh4, &e4);

    // check allocations/frees are properly propagated to the persistent structure
    {
        FreeSpaceMap fsmap_shadow(zone->nvzone());
        fsmap_shadow.init();
        EXPECT_EQ_VERBOSE(*zone->fsmap(), fsmap_shadow);
    }
    
    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(Extent(0, 1));
        fsmap_expect.insert(Extent(10, MAX_BLOCK-10));
        EXPECT_EQ_VERBOSE(fsmap_expect, *zone->fsmap());
    }

    RRegion::TPtr<void> e5;
    RRegion::TPtr<nvExtentHeader> eh5;
    zone->alloc_extent(1, &eh4, &e5);

    // check allocations/frees are properly propagated to the persistent structure
    {
        FreeSpaceMap fsmap_shadow(zone->nvzone());
        fsmap_shadow.init();
        EXPECT_EQ_VERBOSE(*zone->fsmap(), fsmap_shadow);
    }
    
    // check that the freespace map contains the extents we expect
    {
        ExtentMap fsmap_expect;
        fsmap_expect.insert(Extent(10, MAX_BLOCK-10));
        EXPECT_EQ_VERBOSE(fsmap_expect, *zone->fsmap());
    }

    zoneheap()->release_zone(zone);

    UNUSED_ND(e2);
    UNUSED_ND(e3);
    UNUSED_ND(e4);
    UNUSED_ND(e5);
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
