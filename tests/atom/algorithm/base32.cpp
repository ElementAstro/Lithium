#include <gtest/gtest.h>
#include "atom/algorithm/base32.hpp"

TEST(Base32Test, EncodeDecodeTest)
{
    std::string input = "Hello World!";
    std::string encoded = Atom::Algorithm::encodeBase32(input);
    std::string decoded = Atom::Algorithm::decodeBase32(encoded);

    EXPECT_EQ(input, decoded);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
