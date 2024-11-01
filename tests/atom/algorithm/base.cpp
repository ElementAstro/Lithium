#include "atom/algorithm/base.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(Base64Test, Encode) {
    std::string data = "Hello, World!";
    std::string encoded = atom::algorithm::base64Encode(data);
    EXPECT_EQ(encoded, "SGVsbG8sIFdvcmxkIQ==");
}

TEST(Base64Test, Decode) {
    std::string encoded = "SGVsbG8sIFdvcmxkIQ==";
    std::string decoded = atom::algorithm::base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello, World!");
}

TEST(Base64Test, EncodeDecode) {
    std::string data = "Hello, World!";
    std::string encoded = atom::algorithm::base64Encode(data);
    std::string decoded = atom::algorithm::base64Decode(encoded);
    EXPECT_EQ(decoded, data);
}

TEST(FastBase64Test, Encode) {
    std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                                       'W', 'o', 'r', 'l', 'd', '!'};
    std::string encoded = atom::algorithm::fbase64Encode(data);
    EXPECT_EQ(encoded, "SGVsbG8sIFdvcmxkIQ==");
}

TEST(FastBase64Test, Decode) {
    std::string encoded = "SGVsbG8sIFdvcmxkIQ==";
    std::vector<unsigned char> decoded = atom::algorithm::fbase64Decode(
        std::span<const char>(encoded.data(), encoded.size()));
    std::string decoded_str(decoded.begin(), decoded.end());
    EXPECT_EQ(decoded_str, "Hello, World!");
}

TEST(FastBase64Test, EncodeDecode) {
    std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                                       'W', 'o', 'r', 'l', 'd', '!'};
    std::string encoded = atom::algorithm::fbase64Encode(data);
    std::vector<unsigned char> decoded = atom::algorithm::fbase64Decode(
        std::span<const char>(encoded.data(), encoded.size()));
    std::string decoded_str(decoded.begin(), decoded.end());
    EXPECT_EQ(decoded_str, std::string(data.begin(), data.end()));
}

/*
TODO: Fix the following tests, they are not working as expected.

TEST(ConstBase64Test, Encode) {
    constexpr StaticString<13> DATA{"Hello, World!"};
    constexpr auto ENCODED = atom::algorithm::cbase64Encode(DATA);
    EXPECT_STREQ(ENCODED.cStr(), "SGVsbG8sIFdvcmxkIQ==");
}

TEST(ConstBase64Test, Decode) {
    constexpr StaticString<20> encoded("SGVsbG8sIFdvcmxkIQ==");
    constexpr auto decoded = atom::algorithm::cbase64Decode(encoded);
    EXPECT_STREQ(decoded.cStr(), "Hello, World!");
}

TEST(ConstBase64Test, EncodeDecode) {
    constexpr StaticString<13> data("Hello, World!");
    constexpr auto encoded = atom::algorithm::cbase64Encode(data);
    constexpr auto decoded = atom::algorithm::cbase64Decode(encoded);
    EXPECT_STREQ(decoded.cStr(), "Hello, World!");
}
*/

TEST(XORCipherTest, EncryptDecrypt) {
    std::string data = "Hello, World!";
    uint8_t key = 0xAA;
    std::string encrypted = atom::algorithm::xorEncrypt(data, key);
    std::string decrypted = atom::algorithm::xorDecrypt(encrypted, key);
    EXPECT_EQ(decrypted, data);
}
