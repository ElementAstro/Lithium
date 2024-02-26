#include <gtest/gtest.h>
#include "atom/algorithm/base64.hpp"

TEST(Base64Test, EncodeDecodeTest)
{
    std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'};

    // Test base64Encode and base64Decode
    std::string encoded = base64Encode(data);
    std::vector<unsigned char> decoded = base64Decode(encoded);

    EXPECT_EQ(data, decoded);

    // Test base64EncodeEnhance and base64DecodeEnhance
    std::vector<uint8_t> dataEnhance = {'T', 'e', 's', 't', ' ', 'S', 't', 'r', 'i', 'n', 'g'};
    std::string encodedEnhance = base64EncodeEnhance(dataEnhance);
    std::vector<uint8_t> decodedEnhance = base64DecodeEnhance(encodedEnhance);

    EXPECT_EQ(dataEnhance, decodedEnhance);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
