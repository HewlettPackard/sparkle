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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <thread>

#include "gtest/gtest.h"
#include "alps/pegasus/pegasus.hh"
#include "alps/pegasus/relocatable_region.hh"
#include "test_common.hh"

using namespace alps;


void access_region(char* base, size_t len, bool read, bool write)
{
    char buf[1024];

    for (size_t i=0; i<len; i+=1024) {
        if (read) {
            memcpy(buf, &base[i], 1024);
        }
        if (write) {
            memcpy(&base[i], buf, 1024);
        }
    }
}

class MapTest: public RegionFileTest { 
public:
    void map(int flags = 0)
    {
        EXPECT_LE(0, fd = open(test_path("test_region").c_str(), O_CREAT|O_RDWR, S_IRUSR | S_IWUSR));
        EXPECT_EQ(0, ftruncate(fd, region_size));
        EXPECT_NE((void*) -1, base = mmap(0, region_size, PROT_READ|PROT_WRITE, MAP_SHARED|flags, fd, 0));
        ptr = (char*) base;
    }

    void concurrent_access(bool read, bool write) 
    {
        std::thread t0 = std::thread(access_region, ptr, region_size, read, write);
        std::thread t1 = std::thread(access_region, ptr, region_size, read, write);
        t0.join();
        t1.join();
    }


    int fd;
    void* base;
    char* ptr;
    size_t region_size;
};

TEST_F(MapTest, sequential_write_read)
{
    region_size = booksize();
    map();
    const char* buf = "BEEF";
    strcpy(ptr, buf);
    EXPECT_EQ(0, strcmp(ptr, buf));
}

TEST_F(MapTest, concurrent_access_read)
{
    region_size = booksize();
    map();
    concurrent_access(true, false);
}

TEST_F(MapTest, concurrent_access_write)
{
    region_size = booksize();
    map();
    concurrent_access(false, true);
}

TEST_F(MapTest, concurrent_access_read_write)
{
    region_size = booksize();
    map();
    concurrent_access(true, true);
}

TEST_F(MapTest, map_populate_concurrent_access_read)
{
    region_size = booksize();
    map(MAP_POPULATE);
    concurrent_access(true, false);
}

TEST_F(MapTest, map_populate_concurrent_access_write)
{
    region_size = booksize();
    map(MAP_POPULATE);
    concurrent_access(false, true);
}

TEST_F(MapTest, map_populate_concurrent_access_read_write)
{
    region_size = booksize();
    map(MAP_POPULATE);
    concurrent_access(true, true);
}

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
