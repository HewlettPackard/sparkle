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

#include "alps/layers/pointer.hh"
#include "test_common.hh"

using namespace alps;


class PointerTest: public AnonymousRegionTest {};

struct struct_bar {
    
};

struct struct_foo {
    PPtr<int>               a;
    PPtr<struct struct_foo> array[20];
    PPtr<struct struct_foo> next;
    PPtr<struct struct_bar> pbar;
    struct struct_bar       bar;
};


TEST_F(PointerTest, construct)
{
    TPtr<struct struct_foo> tptr1 = alloc(sizeof(struct_foo));

    TPtr<struct struct_foo> tptr2(tptr1);

    EXPECT_EQ(tptr1, tptr2);
}

TEST_F(PointerTest, deref_and_assign1)
{ 
    TPtr<struct struct_foo> tptr1 = alloc(sizeof(struct_foo));
    tptr1->a = alloc(sizeof(int));
    *(tptr1->a) = 0xBEEF;
    EXPECT_EQ(0xBEEF, *(tptr1->a));
}

TEST_F(PointerTest, deref_and_assign2)
{ 
    TPtr<struct struct_foo> head = alloc(sizeof(struct struct_foo));
    int array_nelements = 32;
    int array_size = sizeof(int) * array_nelements;
    head->a = alloc(array_size);
    head->next = alloc(sizeof(struct struct_foo));
    head->next->a = alloc(sizeof(int));
    *(head->a) = 0xBEEF;
    *(head->next->a) = 0xC0FFE;
}

TEST_F(PointerTest, simple_assignment)
{
    TPtr<struct struct_foo> node1 = alloc(sizeof(struct struct_foo));
    TPtr<struct struct_foo> node2 = alloc(sizeof(struct struct_foo));

    node1->next = node2;
    node2 = node1;

    node1->pbar = &node2->bar;
}

TEST_F(PointerTest, assign_virtual)
{
    TPtr<struct struct_foo> head = alloc(sizeof(struct struct_foo));
    int array_nelements = 32;
    int array_size = sizeof(int) * array_nelements;
    head->a = alloc(array_size);
    head->next = alloc(sizeof(struct struct_foo));

    // assignment via copy constructor
    TPtr<struct struct_foo> node = head->next;
    EXPECT_EQ(head->next.get(), node.get());

    // assignment via assign operator
    TPtr<struct struct_foo> node2;
    node2 = head->next;
    EXPECT_EQ(head->next.get(), node2.get());
}

TEST_F(PointerTest, assign_array_pptr)
{
    TPtr<struct struct_foo> head = alloc(sizeof(struct struct_foo));
    head->array[0] = alloc(sizeof(struct struct_foo));
}

TEST_F(PointerTest, assign_array_tptr)
{
    TPtr<struct struct_foo> array[10];
    array[0] = alloc(sizeof(struct struct_foo));
}

TEST_F(PointerTest, pointer_to_pointer)
{
    TPtr<struct struct_foo> node1 = alloc(sizeof(struct struct_foo));
    TPtr<struct struct_foo> node2 = alloc(sizeof(struct struct_foo));
    TPtr<struct struct_foo> node3 = alloc(sizeof(struct struct_foo));

    node1->next = node2;

    EXPECT_EQ(node1->next, node2);

    TPtr<TPtr<struct struct_foo>> next = &node1->next;
    *next = node3;

    EXPECT_EQ(node1->next, node3);
}

TEST_F(PointerTest, operators_pointer_arithmetic)
{
    int nr_elems = 16;
    int i;
    int j;

    TPtr<int> array = alloc(nr_elems*sizeof(int));
    TPtr<int> ptr;

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


int main(int argc, char** argv)
{
    //::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
