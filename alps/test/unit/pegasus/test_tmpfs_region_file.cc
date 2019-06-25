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
#include "alps/pegasus/pegasus_options.hh"
#include "test_common.hh"

using namespace alps;

TEST_F(RegionFileTest, create)
{
    RegionFile* f;
   
    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("test_create").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
}

TEST_F(RegionFileTest, create_resize)
{
    RegionFile* f;
    loff_t file_size = 8*booksize();

    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("test_create").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
    EXPECT_EQ(kErrorCodeOk, f->truncate(file_size));
}

TEST_F(RegionFileTest, setxattr)
{
    RegionFile* f;
    char interleave_request[] = {0x2, 0x3, 0x2, 0x3, 0x2, 0x3};

    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("fsetxattr").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
    EXPECT_EQ(kErrorCodeOk, f->setxattr("user.LFS.InterleaveRequest", interleave_request, sizeof(interleave_request), 0));
}

TEST_F(RegionFileTest, getxattr)
{
    RegionFile* f;
    char buf[1024];
    char interleave_request[] = {0x2, 0x3, 0x2, 0x3, 0x2, 0x3};

    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("fsetxattr").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
    EXPECT_EQ(kErrorCodeOk, f->setxattr("user.LFS.InterleaveRequest", interleave_request, sizeof(interleave_request), 0));
    EXPECT_EQ(kErrorCodeOk, f->getxattr("user.LFS.InterleaveRequest", buf, sizeof(interleave_request)));
    for (size_t i=0; i < sizeof(interleave_request); i++) {
        EXPECT_EQ(interleave_request[i], buf[i]);
    }
}

TEST_F(RegionFileTest, getxattr_noattr)
{
    RegionFile* f;
    char buf[1024];

    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("fsetxattr_noattr").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
    EXPECT_NE(kErrorCodeOk, f->getxattr("nonexistent_attribute", buf, sizeof(buf)));
}

TEST_F(RegionFileTest, creat_truncate1)
{
    RegionFile* f;
    char     buf[1024];
    size_t   test_file_size = 8*booksize();
    char     expected_interleave[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("fsetxattr_noattr").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
    EXPECT_EQ(kErrorCodeOk, f->truncate(test_file_size));

    EXPECT_EQ(kErrorCodeOk, f->getxattr("user.LFS.Interleave", buf, sizeof(expected_interleave)));
    for (size_t i=0; i < sizeof(expected_interleave); i++) {
        EXPECT_EQ(expected_interleave[i], buf[i]);
    }
}

TEST_F(RegionFileTest, creat_truncate2)
{
    RegionFile* f;
    char     buf[1024];
    size_t   test_file_size = 8*booksize();
    char     interleave_request[] =  {0x2, 0x3, 0x2, 0x3, 0x2, 0x3};
    char     expected_interleave[] = {0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x0, 0x0};

    EXPECT_NE((RegionFile*) 0, f = TmpfsRegionFile::construct(test_path("fsetxattr_noattr").c_str(), pegasus_options));
    EXPECT_EQ(kErrorCodeOk, f->create(S_IRUSR | S_IWUSR));
    EXPECT_EQ(kErrorCodeOk, f->setxattr("user.LFS.InterleaveRequest", interleave_request, sizeof(interleave_request), 0));
    EXPECT_EQ(kErrorCodeOk, f->truncate(test_file_size));

    EXPECT_EQ(kErrorCodeOk, f->getxattr("user.LFS.Interleave", buf, sizeof(expected_interleave)));
    for (size_t i=0; i < sizeof(expected_interleave); i++) {
        EXPECT_EQ(expected_interleave[i], buf[i]);
    }
}

#if 0
TEST(pfile, creat_truncate3)
{
  int fd;
  char buf[MAX_INTERLEAVE_REQUEST_SIZE];
  size_t test_file_size = 64*1024LLU*1024LLU;
  char interleave_request[] =  {0x2, 0x3, 0x2, 0x3, 0x2, 0x3};
  char expected_interleave[MAX_INTERLEAVE_REQUEST_SIZE];
  int expected_interleave_size = test_file_size/LfsDevshm::kBookSize;
  ASSERT_LE(expected_interleave_size, MAX_INTERLEAVE_REQUEST_SIZE);
  // construct a round-robin expected-interleave request
  int max_n = numa_max_node();
  for (int i=0; i<expected_interleave_size; i++) {
    expected_interleave[i] = i % max_n;
  } 

  LfsDevshm::set_interleave_policy(LfsDevshm::kRoundRobin);

  EXPECT_LE(0, fd = LfsDevshm::creat("/lfs/test/creat_truncate3", S_IRUSR | S_IWUSR));
  EXPECT_EQ(0, LfsDevshm::fsetxattr(fd, "user.interleave_request", interleave_request, sizeof(interleave_request), 0));
  EXPECT_EQ(0, LfsDevshm::ftruncate(fd, test_file_size));
  EXPECT_EQ(test_file_size, (size_t) lseek(fd, 0, SEEK_END));

  EXPECT_EQ(0, LfsDevshm::fgetxattr(fd, "user.interleave", buf, expected_interleave_size));
  for (int i=0; i < expected_interleave_size; i++) {
    EXPECT_EQ(expected_interleave[i], buf[i]);
  }
 
  LfsDevshm::close(fd);
  EXPECT_EQ(0, LfsDevshm::unlink("/lfs/test/creat_truncate3"));
}

#endif

int main(int argc, char** argv)
{
    ::alps::init_test_env<::alps::TestEnvironment>(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
