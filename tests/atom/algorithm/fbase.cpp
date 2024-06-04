#include "atom/algorithm/fbase.hpp"
#include <gtest/gtest.h>
#include <span>
#include <string>
#include <vector>

using namespace atom::algorithm;

// Test for Base64 encoding
TEST(FBase64Test, Encode) {
    std::vector<unsigned char> data = {'f', 'o', 'o'};
    std::span<const unsigned char> input(data.data(), data.size());
    std::string encoded = fbase64Encode(input);
    EXPECT_EQ(encoded, "Zm9v");  // Expected Base64 encoding of "foo"
}

TEST(FBase64Test, EncodeEmpty) {
    std::vector<unsigned char> data = {};
    std::span<const unsigned char> input(data.data(), data.size());
    std::string encoded = fbase64Encode(input);
    EXPECT_EQ(encoded,
              "");  // Encoding an empty input should result in an empty string
}

TEST(FBase64Test, EncodeLongString) {
    std::vector<unsigned char> data(1000, 'a');
    std::span<const unsigned char> input(data.data(), data.size());
    std::string encoded = fbase64Encode(input);
    // Just check the length of encoded string
    EXPECT_EQ(encoded.size(), 1336);  // 1000 bytes of input should produce 1336
                                      // characters of Base64 output
}

// Test for Base64 decoding
TEST(FBase64Test, Decode) {
    std::string data = "Zm9v";
    std::span<const char> input(data.data(), data.size());
    std::vector<unsigned char> decoded = fbase64Decode(input);
    std::vector<unsigned char> expected = {'f', 'o', 'o'};
    EXPECT_EQ(decoded, expected);  // Expected decoded output of "Zm9v" is "foo"
}

TEST(FBase64Test, DecodeEmpty) {
    std::string data = "";
    std::span<const char> input(data.data(), data.size());
    std::vector<unsigned char> decoded = fbase64Decode(input);
    EXPECT_EQ(decoded.size(),
              0);  // Decoding an empty input should result in an empty vector
}

TEST(FBase64Test, DecodeInvalidInput) {
    std::string data = "InvalidBase64";
    std::span<const char> input(data.data(), data.size());

    // Test should handle invalid input gracefully, depending on the
    // implementation Here we assume it returns an empty vector for invalid
    // input
    std::vector<unsigned char> decoded = fbase64Decode(input);
    EXPECT_EQ(decoded.size(), 0);  // Check for expected behavior
}

TEST(FBase64Test, EncodeDecode) {
    std::vector<unsigned char> data = {'f', 'o', 'o'};
    std::span<const unsigned char> input(data.data(), data.size());
    std::string encoded = fbase64Encode(input);

    std::span<const char> encodedInput(encoded.data(), encoded.size());
    std::vector<unsigned char> decoded = fbase64Decode(encodedInput);

    EXPECT_EQ(decoded, data);  // Ensure that decoding the encoded string
                               // returns the original data
}
