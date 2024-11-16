#include "base64.hpp"

#include <gtest/gtest.h>

// Test case for decoding an empty string
TEST(Base64DecodeTest, EmptyString) {
    std::string encoded;
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "");
}

// Test case for decoding a simple string
TEST(Base64DecodeTest, SimpleString) {
    std::string encoded = "SGVsbG8=";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello");
}

// Test case for decoding a string with padding
TEST(Base64DecodeTest, StringWithPadding) {
    std::string encoded = "SGVsbG8gd29ybGQ=";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello world");
}

// Test case for decoding a string without padding
TEST(Base64DecodeTest, StringWithoutPadding) {
    std::string encoded = "SGVsbG8gd29ybGQ";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello world");
}

// Test case for decoding a string with special characters
TEST(Base64DecodeTest, StringWithSpecialCharacters) {
    std::string encoded = "U3BlY2lhbCBjaGFyYWN0ZXJzOiAhQCMkJV4mKigpXys9";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Special characters: !@#$%^&*()_+=");
}

// Test case for decoding a string with numbers
TEST(Base64DecodeTest, StringWithNumbers) {
    std::string encoded = "MTIzNDU2Nzg5MA==";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "1234567890");
}

// Test case for decoding a string with mixed case
TEST(Base64DecodeTest, StringWithMixedCase) {
    std::string encoded = "SGVsbG8gV29ybGQ=";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello World");
}

// Test case for decoding a string with non-alphanumeric characters
TEST(Base64DecodeTest, StringWithNonAlphanumericCharacters) {
    std::string encoded = "LyoqKiov";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "/****/");
}

// Test case for decoding a string with newline characters
TEST(Base64DecodeTest, StringWithNewlineCharacters) {
    std::string encoded = "SGVsbG8K";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello\n");
}

// Test case for decoding a string with whitespace characters
TEST(Base64DecodeTest, StringWithWhitespaceCharacters) {
    std::string encoded = "SGVsbG8gV29ybGQgd2l0aCB3aGl0ZXNwYWNl";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello World with whitespace");
}

// Test case for decoding a malformed base64 string
TEST(Base64DecodeTest, MalformedBase64String) {
    std::string encoded = "SGVsbG8gd29ybGQ@";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "");
}

// Test case for decoding a string with invalid characters
TEST(Base64DecodeTest, StringWithInvalidCharacters) {
    std::string encoded = "SGVsbG8gd29ybGQ$";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "");
}

// Test case for decoding a string with URL-safe base64 encoding
TEST(Base64DecodeTest, URLSafeBase64String) {
    std::string encoded = "SGVsbG8gd29ybGQ-";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello world");
}

// Test case for decoding a string with base64 encoding without padding
TEST(Base64DecodeTest, Base64WithoutPadding) {
    std::string encoded = "SGVsbG8gd29ybGQ";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello world");
}

// Test case for decoding a string with base64 encoding with padding
TEST(Base64DecodeTest, Base64WithPadding) {
    std::string encoded = "SGVsbG8gd29ybGQ=";
    std::string decoded = base64Decode(encoded);
    EXPECT_EQ(decoded, "Hello world");
}
