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

#include "test_common.hh"

using namespace alps;

class PointerTestBase : public RegionTest { 
public:
    void SetUp() {
        __SetUp(false, false); // no cleanup, no create
    }

    void __SetUp(bool cleanup, bool create) {
        RegionFileTest::__SetUp(cleanup);
        region_file_size = booksize();
        if (create) {
            EXPECT_EQ(kErrorCodeOk, Pegasus::create_region_file(test_path("region_pointer").c_str(), S_IRUSR | S_IWUSR, &region_file));
            EXPECT_EQ(kErrorCodeOk, region_file->truncate(region_file_size));
            EXPECT_EQ(kErrorCodeOk, region_file->close());
        }
        EXPECT_EQ(kErrorCodeOk, Pegasus::open_region_file(test_path("region_pointer").c_str(), O_RDWR, &region_file));
        EXPECT_EQ(kErrorCodeOk, Pegasus::address_space()->map(region_file, &region));
        last_alloc = 0;
 
    }
};

class PointerTestMultiThread: public PointerTestBase, public MultiThreadTest {

};

class PointerTestPrepare : public PointerTestBase { 
public:
    void SetUp() {
        PointerTestBase::__SetUp(true, true); // cleanup, create
    }
};

TEST_F(PointerTestPrepare, prepare)
{
    // null test that simply invokes the fixture to do process-wide 
    // initialization before running the actual test
}


template<typename T>
std::string serialize_pointer(T& ptr)
{
  std::stringstream ss;
  
  ss << ptr.region_id();
  ss << ",";
  ss << ptr.offset();
  std::cout << ss.str() << std::endl;
  return ss.str();
}

template<typename T>
T deserialize_pointer(std::string str)
{
  std::istringstream ss(str);
  std::string a, b;
  std::getline(ss, a, ',');
  std::getline(ss, b, ',');
  return T(stoi(a), stoi(b));
}



struct struct_foo {
    int val;    
};

struct struct_root {
    RRegion::PPtr<struct struct_foo> foo_ptr;    
};


MINT_F_TEST_PARENT(PointerTest, PointerTestMultiThread);

MINT_F_TEST(PointerTest, alloc)
{
    TEST_BLOCK(alloc) 
    {
        RRegion::TPtr<struct struct_root> root = region->base<struct_root>(0x0);
        root->foo_ptr = region->base<struct_foo>(0x100);
        root->foo_ptr->val = 0xBEEF;

        //RRegion::TPtr<struct struct_foo> ptr = (RRegion::TPtr<struct struct_foo>) root->foo_ptr;
        //mint_set_kv("ptr", serialize_pointer(ptr));
    }

    RRegion::TPtr<struct struct_root> my_root = region->base<struct_root>(0x0);
    EXPECT_EQ(0xBEEF, my_root->foo_ptr->val);
}

MINT_F_TEST(PointerTest, use)
{
    TEST_BLOCK(use) 
    {
        RRegion::TPtr<struct struct_root> root = region->base<struct_root>(0x0);
        EXPECT_EQ(0xBEEF, root->foo_ptr->val);

        //mint_get_kv("ptr", serialize_pointer(ptr));
    }
}


int main(int argc, char **argv) {
    ::alps::init_integration_test_env<::alps::TestEnvironment>(argc, argv);
    mint_init(argc, argv);
    return RUN_ALL_TESTS();
}
