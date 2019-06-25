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

#include <iostream>
#include <stdlib.h>

#include <map>
#include <string>
#include <sstream>
#include <type_traits>

#include "mint/mint.h"
#include "gtest/gtest.h"

#include "alps/pegasus/pegasus.hh"
#include "pegasus/topology.hh"
#include "pegasus/topology_factory.hh"

#ifdef __ARCH_FAM__
#include <fam_atomic.h>
#endif

#include "test_common.hh"

using namespace alps;

class AtomicTestBase : public RegionFileTest { 
public:
    void SetUp() {
        __SetUp(false, false); // no cleanup, no create
    }

    void __SetUp(bool cleanup, bool create) {
        RegionFileTest::__SetUp(cleanup);
        region_file_size = booksize();
        if (create) {
            EXPECT_LE(0, fd = open(test_path("test_region_atomic").c_str(), O_CREAT|O_RDWR, S_IRUSR | S_IWUSR));
            EXPECT_EQ(0, ftruncate(fd, region_file_size));
            EXPECT_EQ(0, close(fd));
        }
        EXPECT_LE(0, fd = open(test_path("test_region_atomic").c_str(), O_RDWR, S_IRUSR | S_IWUSR));
        EXPECT_NE((void*) -1, base = mmap(0, region_file_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
#ifdef __ARCH_FAM__
        EXPECT_EQ(0, fam_atomic_register_region(base, region_file_size, fd, 0));
#endif
        ptr = (char*) base;

    }

    void TearDown() {
#ifdef __ARCH_FAM__
        fam_atomic_unregister_region(base, region_file_size);
#endif
        EXPECT_EQ(0, close(fd));
    }

    size_t region_file_size;
    void* base;
    char* ptr;
    int   fd;
};

class AtomicTestMultiThread: public AtomicTestBase, public MultiThreadTest {

};

class AtomicTestPrepare : public AtomicTestBase { 
public:
    void SetUp() {
        AtomicTestBase::__SetUp(true, true); // cleanup, create
    }
};

TEST_F(AtomicTestPrepare, prepare)
{
    // null test that simply invokes the fixture to do process-wide 
    // initialization before running the actual test
}


MINT_F_TEST_PARENT(AtomicTest, AtomicTestMultiThread);

MINT_F_TEST(AtomicTest, producer)
{
#ifdef __ARCH_FAM__
    int64_t* counter = (int64_t*) ptr;
    int64_t  old_c;
    int64_t  new_c;

    for (int64_t i=0; i<1024; i++) {
        old_c = fam_atomic_64_fetch_add(counter, 1);
        while ((new_c = fam_atomic_64_read(counter)) == old_c+1);
    }
#endif
}

MINT_F_TEST(AtomicTest, consumer)
{
#ifdef __ARCH_FAM__
    int64_t* counter = (int64_t*) ptr;
    int64_t  old_c;

    for (int64_t i=0; i<1024; i++) {
        while ((old_c = fam_atomic_64_read(counter)) == i*2);
        fam_atomic_64_fetch_add(counter, 1);
    }
#endif
}


int main(int argc, char **argv) {
    ::alps::init_integration_test_env<::alps::TestEnvironment>(argc, argv);
    mint_init(argc, argv);
    return RUN_ALL_TESTS();
}
