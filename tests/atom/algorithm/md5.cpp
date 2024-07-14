#include "atom/algorithm/md5.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

TEST(MD5Test, BasicTest) {
    EXPECT_EQ(MD5::encrypt(""), "d41d8cd98f00b204e9800998ecf8427e");
    EXPECT_EQ(MD5::encrypt("a"), "0cc175b9c0f1b6a831c399e269772661");
    EXPECT_EQ(MD5::encrypt("abc"), "900150983cd24fb0d6963f7d28e17f72");
    EXPECT_EQ(MD5::encrypt("message digest"),
              "f96b697d7cb7938d525a2f31aaf161d0");
    EXPECT_EQ(MD5::encrypt("abcdefghijklmnopqrstuvwxyz"),
              "c3fcd3d76192e4007dfb496cca67e13b");
    EXPECT_EQ(MD5::encrypt("123456789012345678901234567890123456789012345678901"
                           "23456789012345678901234567890"),
              "57edf4a22be3c955ac49da2e2107b67a");
}