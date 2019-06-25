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
#include <vector>
#include <sstream>
#include "gtest/gtest.h"

#include "alps/common/externalizable.hh"

using namespace alps;

struct ChildOptions: public Externalizable {
    ChildOptions() {
        foo_bool = false;
    }

    bool             foo_bool = false;
    std::string      foo_string = "init_string: child";
    std::vector<int> foo_vector;

    enum FooEnum {
        kEnumVal0 = 0,
        kEnumVal1 = 1,
        kEnumVal2 = 2
    };

    FooEnum foo_enum;

    EXTERNALIZABLE(ChildOptions);
};

ErrorStack ChildOptions::load(YAML::Node* node, bool ignore_missing) {
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, foo_bool);
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, foo_string);
    EXTERNALIZE_LOAD_ENUM_ELEMENT_OPTIONAL(node, foo_enum, kEnumVal2)
    EXTERNALIZE_LOAD_ELEMENT(node, ignore_missing, foo_vector);
    return kRetOk;
}


ErrorStack ChildOptions::save(YAML::Emitter* out) const {
    EXTERNALIZE_SAVE_ELEMENT(out, foo_bool, "foo_bool description");
    EXTERNALIZE_SAVE_ELEMENT(out, foo_string, "foo_string description");
    EXTERNALIZE_SAVE_ENUM_ELEMENT(out, foo_enum, "foo_enum description")
    EXTERNALIZE_SAVE_ELEMENT(out, foo_vector, "foo_vector description");
    return kRetOk;
}

ErrorStack ChildOptions::add_command_options(CommandOptionList* cmdopt) {
    return kRetOk;
};

struct ParentOptions: public Externalizable {
    ParentOptions() {
        goo_bool = false;
    }

    bool             goo_bool = false;
    std::string      goo_string = "init_string: parent";
    ChildOptions     child;

    EXTERNALIZABLE(ParentOptions);
};


// template-ing just for const/non-const
template <typename PARENT_OPTION_PTR, typename CHILD_PTR>
std::vector< CHILD_PTR > get_children_impl(PARENT_OPTION_PTR option) {
    std::vector< CHILD_PTR > children;
    children.push_back(&option->child);
    return children;
}

std::vector<Externalizable* > get_children(ParentOptions* option) {
    return get_children_impl<ParentOptions*, Externalizable*>(option);
}

std::vector< const Externalizable* > get_children(const ParentOptions* option) {
    return get_children_impl<const ParentOptions*, const Externalizable*>(option);
}

ErrorStack ParentOptions::load(YAML::Node* node, bool ignore_missing) {
    *this = ParentOptions();  // This guarantees default values for optional XML elements.
    for (Externalizable* child : get_children(this)) {
        CHECK_ERROR(get_child_element(node, child->get_tag_name(), child, ignore_missing));
    }
    return kRetOk;
}

ErrorStack ParentOptions::save(YAML::Emitter* out) const {
//    EXTERNALIZE_SAVE_ELEMENT(element, foo_, "foo comment");
    return kRetOk;
}

ErrorStack ParentOptions::add_command_options(CommandOptionList* cmdopt) {
    return kRetOk;
};



TEST(TestOptions, load_default)
{
    ChildOptions options;

    options.load_from_string(
        "foo_bool: true\n"
        "foo_string: test_string\n"
        "foo_vector: [1,2,3]");

    EXPECT_EQ(options.foo_bool , true);
    EXPECT_EQ(options.foo_string , "test_string");
    EXPECT_EQ(options.foo_enum , ChildOptions:: kEnumVal2);
    EXPECT_EQ(options.foo_vector[0], 1);
    EXPECT_EQ(options.foo_vector[1], 2);
    EXPECT_EQ(options.foo_vector[2], 3);
}

TEST(TestOptions, load)
{
    ChildOptions options;

    options.load_from_string(
        "foo_bool: true\n"
        "foo_string: test_string\n"
        "foo_enum: 1\n"
        "foo_vector: [1,2,3]");

    EXPECT_EQ(options.foo_bool, true);
    EXPECT_EQ(options.foo_string, "test_string");
    EXPECT_EQ(options.foo_enum, ChildOptions:: kEnumVal1);
    EXPECT_EQ(options.foo_vector[0], 1);
    EXPECT_EQ(options.foo_vector[1], 2);
    EXPECT_EQ(options.foo_vector[2], 3);
}

TEST(TestOptions, load_allow_missing)
{
    ChildOptions options;

    options.load_from_string(
        "foo_bool: true\n"
        "foo_vector: [1,2,3]", true);

    EXPECT_EQ(options.foo_bool , true);
    EXPECT_EQ(options.foo_string , "init_string: child");
    EXPECT_EQ(options.foo_enum , ChildOptions:: kEnumVal2);
    EXPECT_EQ(options.foo_vector[0], 1);
    EXPECT_EQ(options.foo_vector[1], 2);
    EXPECT_EQ(options.foo_vector[2], 3);
}

TEST(TestOptions, load_multiple)
{
    ChildOptions options;

    options.load_from_string(
        "foo_bool: true\n"
        "foo_vector: [1,2,3]", true);

    options.load_from_string(
        "foo_string: test_string\n", true);


    EXPECT_EQ(options.foo_bool , true);
    EXPECT_EQ(options.foo_string, "test_string");
    EXPECT_EQ(options.foo_enum , ChildOptions:: kEnumVal2);
    EXPECT_EQ(options.foo_vector[0], 1);
    EXPECT_EQ(options.foo_vector[1], 2);
    EXPECT_EQ(options.foo_vector[2], 3);
}


TEST(TestOptions, load_child)
{
    ParentOptions options;

    options.load_from_string(
        "ChildOptions:\n"
        "  foo_bool: true\n"
        "  foo_enum: 1\n"
        "  foo_vector: [1,2,3]");

    EXPECT_EQ(options.child.foo_bool, true);
}

TEST(TestOptions, save)
{
    ChildOptions options;

    options.load_from_string(
        "#this is a comment\n"
        "foo_bool: true\n"
        "foo_string: test_string\n"
        "foo_enum: 1\n"
        "foo_vector: [1,2,3]");

    EXPECT_EQ(options.foo_bool, true);
    EXPECT_EQ(options.foo_vector[0], 1);
    EXPECT_EQ(options.foo_vector[1], 2);
    EXPECT_EQ(options.foo_vector[2], 3);
    
    options.foo_bool = false;

    std::stringstream ss;
    options.save_to_stream(&ss);
}



int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
