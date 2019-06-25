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
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <attr/xattr.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>

#include "gtest/gtest.h"
#include "alps/pegasus/pegasus.hh"
#include "alps/pegasus/relocatable_region.hh"
#include "test_common.hh"

//using namespace alps;

class InterleaveGroup {
public:
    InterleaveGroup();
    InterleaveGroup(const char* ig_str);
    InterleaveGroup(uint8_t ig);

    operator std::string() const {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2) << int(ig_) << std::dec;
        return ss.str();
    }
    
    bool operator==(const InterleaveGroup& other) const
    {
        return ig_ == other.ig_;
    }

private:
    uint8_t ig_;    
};

inline InterleaveGroup::InterleaveGroup()
    : ig_(0)
{

}

inline InterleaveGroup::InterleaveGroup(const char* ig_str)
{
    char str[3] = {ig_str[0], ig_str[1], 0};
    ig_ = atoi(str);
}

inline InterleaveGroup::InterleaveGroup(uint8_t ig)
    : ig_(ig)
{ }



class InterleaveGroupVector {
public:
    InterleaveGroupVector();
    InterleaveGroupVector(std::string desc);

    void* c_array() const;
    size_t size() const;
    void reserve(size_t n);
    void resize(size_t n);

    operator std::string() const {
        std::string str;
        for (std::vector<InterleaveGroup>::const_iterator it = vig_.begin();
             it != vig_.end();
             it++) 
        {
            str += *it;
        }
        return str;
    }

    bool operator==(const InterleaveGroupVector& other) const {
        if (vig_.size() != other.vig_.size()) {
            return false;
        }
        return std::equal(vig_.begin(), vig_.begin() + vig_.size(), other.vig_.begin());
    }

private:
    std::vector<InterleaveGroup> vig_;
};


InterleaveGroupVector::InterleaveGroupVector()
{ 

}


InterleaveGroupVector::InterleaveGroupVector(std::string desc)
{
    for (size_t i=0; i < desc.size(); i+=2) {
        InterleaveGroup ig(&desc[i]);
        vig_.push_back(ig);            
    }
}




inline size_t InterleaveGroupVector::size() const {
    return vig_.size();
}

inline void* InterleaveGroupVector::c_array() const {
    return (void*) &vig_[0];
}

inline void InterleaveGroupVector::reserve(size_t n) {
    vig_.reserve(n);
}

inline void InterleaveGroupVector::resize(size_t n) {
    vig_.resize(n);
}


std::ostream& operator<<(std::ostream& out, const InterleaveGroupVector& vig)
{
   return out << std::string(vig);
}


class PolicyTest: public alps::RegionFileTest {
public:

    void create_region() {
        int fd;
        EXPECT_LE(0, fd = open(test_path("test_region").c_str(), O_CREAT|O_RDWR, S_IRUSR | S_IWUSR));
        EXPECT_LE(0, close(fd));
    }
 
    void set_policy(const char* policy)
    {
        EXPECT_EQ(0, setxattr(test_path("test_region").c_str(), "user.LFS.AllocationPolicy", policy, strlen(policy), 0)) << strerror(errno);
    }

    void set_interleave_request(const InterleaveGroupVector& vig)
    {
        EXPECT_EQ(0, setxattr(test_path("test_region").c_str(), "user.LFS.InterleaveRequest", vig.c_array(), vig.size(), 0)) << strerror(errno);
    }

    InterleaveGroupVector get_interleave(size_t interleave_size)
    {
        InterleaveGroupVector vig;
        vig.resize(interleave_size);
        
        EXPECT_EQ((int) interleave_size, getxattr(test_path("test_region").c_str(), "user.LFS.Interleave", vig.c_array(), interleave_size)) << strerror(errno);
        return vig;
    }

    void truncate(size_t region_size) {
        EXPECT_EQ(0, ::truncate(test_path("test_region").c_str(), region_size)) << strerror(errno);
    }
};


TEST_F(PolicyTest, test_precise)
{
    size_t region_size = 4*booksize();

    create_region();
    set_policy("RequestIG");

    InterleaveGroupVector requested_vig("00010203");
    InterleaveGroupVector assigned_vig;
    set_interleave_request(requested_vig);
    truncate(region_size);
    assigned_vig = get_interleave(region_size / booksize());
    EXPECT_EQ(requested_vig, assigned_vig);
}


int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
