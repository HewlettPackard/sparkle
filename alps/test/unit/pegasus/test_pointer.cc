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
#include "gtest/gtest.h"
#include "alps/pegasus/pegasus.hh"
#include "alps/pegasus/pointer.hh"
#include "test_common.hh"

using namespace alps;


class PointerTest: public RegionTest { };

TEST_F(PointerTest, base)
{ 
    RRegion::TPtr<int> tptr1 = region->base<int>(0x0);
    UNUSED_ND(tptr1);
}

struct struct_bar {
    
};

struct struct_foo {
    RRegion::PPtr<int>               a;
    RRegion::PPtr<struct struct_foo> array[20];
    RRegion::PPtr<struct struct_foo> next;
    RRegion::PPtr<struct struct_bar> pbar;
    struct struct_bar                bar;
};

#ifdef BASE_RELATIVE_POINTERS

TEST_F(PointerTest, construct)
{
    RRegion::TPtr<struct struct_foo> tptr1 = alloc(sizeof(struct_foo));

    RegionId region_id = tptr1.addr_.region_id_;
    LinearAddr linear_addr = tptr1.addr_.linear_addr_;

    RRegion::TPtr<struct struct_foo> tptr2(region_id, linear_addr);

    EXPECT_EQ(tptr1, tptr2);
}

TEST_F(PointerTest, deref_and_assign1)
{ 
    RRegion::TPtr<struct struct_foo> tptr1 = alloc(sizeof(struct_foo));
    tptr1->a = alloc(sizeof(int));
    *(tptr1->a) = 0xBEEF;
    EXPECT_EQ(0xBEEF, *(tptr1->a));
}

TEST_F(PointerTest, deref_and_assign2)
{ 
    RRegion::TPtr<struct struct_foo> head = alloc(sizeof(struct struct_foo));
    int array_nelements = 32;
    int array_size = sizeof(int) * array_nelements;
    head->a = alloc(array_size);
    head->next = alloc(sizeof(struct struct_foo));
    *(head->a) = 0xBEEF;
    *(head->next->a) = 0xC0FFE;
}

TEST_F(PointerTest, simple_assignment)
{
    RRegion::TPtr<struct struct_foo> node1 = alloc(sizeof(struct struct_foo));
    RRegion::TPtr<struct struct_foo> node2 = alloc(sizeof(struct struct_foo));

    node1->next = node2;
    node2 = node1;

    node1->pbar = &node2->bar;
}

TEST_F(PointerTest, assign_virtual)
{
    RRegion::TPtr<struct struct_foo> head = alloc(sizeof(struct struct_foo));
    int array_nelements = 32;
    int array_size = sizeof(int) * array_nelements;
    head->a = alloc(array_size);
    head->next = alloc(sizeof(struct struct_foo));

    // assignment via copy constructor
    RRegion::TPtr<struct struct_foo> node = (RRegion::TPtr<struct struct_foo>) head->next;
    EXPECT_EQ(head->next.addr_.linear_addr_, node.addr_.linear_addr_);

    // assignment via assign operator
    RRegion::TPtr<struct struct_foo> node2;
    node2 = head->next;
    EXPECT_EQ(head->next.addr_.linear_addr_, node2.addr_.linear_addr_);
}

TEST_F(PointerTest, assign_array_pptr)
{
    RRegion::TPtr<struct struct_foo> head = alloc(sizeof(struct struct_foo));
    head->array[0] = alloc(sizeof(struct struct_foo));
}

TEST_F(PointerTest, assign_array_tptr)
{
    RRegion::TPtr<struct struct_foo> array[10];
    array[0] = alloc(sizeof(struct struct_foo));
}

TEST_F(PointerTest, cast_tptr_to_zptr)
{
    RRegion::TPtr<struct struct_foo> thead1 = alloc(sizeof(struct struct_foo));

    RRegion::ZPtr<struct struct_foo> zhead1 = thead1;
    EXPECT_EQ(thead1.addr_, zhead1.addr_);

    RRegion::ZPtr<struct struct_foo> zhead2;
    zhead2 = thead1;
    EXPECT_EQ(thead1.addr_, zhead2.addr_);
}

TEST_F(PointerTest, cast_zptr_to_tptr)
{
    RRegion::TPtr<struct struct_foo> thead1 = alloc(sizeof(struct struct_foo));

    RRegion::ZPtr<struct struct_foo> zhead1 = thead1;
    EXPECT_EQ(thead1.addr_, zhead1.addr_);

    RRegion::TPtr<struct struct_foo> thead2 = zhead1;
    EXPECT_EQ(zhead1.addr_, thead2.addr_);
    EXPECT_EQ(region, thead2.region_);

    RRegion::TPtr<struct struct_foo> thead3;
    thead2 = zhead1;
    EXPECT_EQ(zhead1.addr_, thead2.addr_);
    EXPECT_EQ(region, thead2.region_);
}

TEST_F(PointerTest, pointer_to_pointer)
{
    RRegion::TPtr<struct struct_foo> node1 = alloc(sizeof(struct struct_foo));
    RRegion::TPtr<struct struct_foo> node2 = alloc(sizeof(struct struct_foo));
    RRegion::TPtr<struct struct_foo> node3 = alloc(sizeof(struct struct_foo));

    node1->next = node2;

    EXPECT_EQ(node1->next, node2);

    RRegion::TPtr<RRegion::PPtr<struct struct_foo>> pnext = &node1->next;
    *pnext = node3;

    EXPECT_EQ(node1->next, node3);
}

TEST_F(PointerTest, pptr_get)
{
    RRegion::TPtr<struct struct_foo> node1 = alloc(sizeof(struct struct_foo));
    RRegion::TPtr<struct struct_foo> node2 = alloc(sizeof(struct struct_foo));
    RRegion::TPtr<struct struct_foo> node3 = alloc(sizeof(struct struct_foo));

    RRegion::TPtr<void> tptr1 = base<void>();
    EXPECT_EQ(((uintptr_t) tptr1.get()) + 0 * sizeof(struct struct_foo), (uintptr_t) node1.get());
    EXPECT_EQ(((uintptr_t) tptr1.get()) + 1 * sizeof(struct struct_foo), (uintptr_t) node2.get());
    EXPECT_EQ(((uintptr_t) tptr1.get()) + 2 * sizeof(struct struct_foo), (uintptr_t) node3.get());
}

TEST_F(PointerTest, operators_pointer_arithmetic)
{
    int nr_elems = 16;
    int i;
    int j;

    RRegion::TPtr<int> array = alloc(nr_elems*sizeof(int));
    RRegion::TPtr<int> ptr;

    for (int i=0; i<nr_elems; i++) {
        array[i] = i;
    }

    // operator++
    for (i=0, ptr=array; i<nr_elems; i++, ptr++) {
        EXPECT_EQ(*ptr, i);
    }

    // operator+=
    for (i=0, ptr=array; i<nr_elems; i++, ptr+=1) {
        EXPECT_EQ(*ptr, i);
    }

    // operator+
    for (i=0, ptr=array; i<nr_elems; i++) {
        EXPECT_EQ(*(ptr+i), i);
    }

    // operator--
    for (i=nr_elems-1, ptr=&array[nr_elems-1]; i>=0; i--, ptr--) {
        EXPECT_EQ(*ptr, i);
    }

    // operator-=
    for (i=nr_elems-1, ptr=&array[nr_elems-1]; i>=0; i--, ptr-=1) {
        EXPECT_EQ(*ptr, i);
    }

    // operator-
    for (i=nr_elems-1, j=0, ptr=&array[nr_elems-1]; i>=0; i--, j++) {
        EXPECT_EQ(*(ptr-j), i);
    }
}

#endif


#ifdef SELF_RELATIVE_POINTERS

TEST_F(PointerTest, construct)
{
    RRegion::TPtr<struct struct_foo> tptr1 = alloc(sizeof(struct_foo));

    std::cout << (void*) &tptr1 << std::endl;
    std::cout << (void*) tptr1.offset() << std::endl;
    std::cout << tptr1.get() << std::endl;

    RRegion::TPtr<struct struct_foo> tptr2 = tptr1+1;

    std::cout << (void*) &tptr2 << std::endl;
    std::cout << (void*) tptr2.offset() << std::endl;
    std::cout << tptr2.get() << std::endl;


}



#endif

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
