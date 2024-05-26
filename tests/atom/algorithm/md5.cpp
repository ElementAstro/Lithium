#include "atom/algorithm/md5.hpp"
#include <gtest/gtest.h>

TEST(MD5Test, EncryptTest)
{
    atom::utils::MD5 md5;
    std::string input = "Hello, World!";
    std::string expectedOutput = "5eb63bbbe01eeed093cb22bb8f5acdc3";

    std::string actualOutput = atom::utils::MD5::encrypt(input);

    EXPECT_EQ(actualOutput, expectedOutput);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
