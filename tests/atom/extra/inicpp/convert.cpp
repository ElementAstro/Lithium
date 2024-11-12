#include <gtest/gtest.h>

#include "atom/extra/inicpp/convert.hpp"

#include <stdexcept>
#include <string>

using namespace inicpp;

// Test Convert<bool>
TEST(ConvertTest, BoolDecode) {
    Convert<bool> converter;
    bool result;

    converter.decode("TRUE", result);
    EXPECT_TRUE(result);

    converter.decode("FALSE", result);
    EXPECT_FALSE(result);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, BoolEncode) {
    Convert<bool> converter;
    std::string result;

    converter.encode(true, result);
    EXPECT_EQ(result, "true");

    converter.encode(false, result);
    EXPECT_EQ(result, "false");
}

// Test Convert<char>
TEST(ConvertTest, CharDecode) {
    Convert<char> converter;
    char result;

    converter.decode("A", result);
    EXPECT_EQ(result, 'A');

    EXPECT_THROW(converter.decode("", result), std::invalid_argument);
}

TEST(ConvertTest, CharEncode) {
    Convert<char> converter;
    std::string result;

    converter.encode('A', result);
    EXPECT_EQ(result, "A");
}

// Test Convert<unsigned char>
TEST(ConvertTest, UnsignedCharDecode) {
    Convert<unsigned char> converter;
    unsigned char result;

    converter.decode("A", result);
    EXPECT_EQ(result, 'A');

    EXPECT_THROW(converter.decode("", result), std::invalid_argument);
}

TEST(ConvertTest, UnsignedCharEncode) {
    Convert<unsigned char> converter;
    std::string result;

    converter.encode('A', result);
    EXPECT_EQ(result, "A");
}

// Test Convert<short>
TEST(ConvertTest, ShortDecode) {
    Convert<short> converter;
    short result;

    converter.decode("123", result);
    EXPECT_EQ(result, 123);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, ShortEncode) {
    Convert<short> converter;
    std::string result;

    converter.encode(123, result);
    EXPECT_EQ(result, "123");
}

// Test Convert<unsigned short>
TEST(ConvertTest, UnsignedShortDecode) {
    Convert<unsigned short> converter;
    unsigned short result;

    converter.decode("123", result);
    EXPECT_EQ(result, 123);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, UnsignedShortEncode) {
    Convert<unsigned short> converter;
    std::string result;

    converter.encode(123, result);
    EXPECT_EQ(result, "123");
}

// Test Convert<int>
TEST(ConvertTest, IntDecode) {
    Convert<int> converter;
    int result;

    converter.decode("123", result);
    EXPECT_EQ(result, 123);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, IntEncode) {
    Convert<int> converter;
    std::string result;

    converter.encode(123, result);
    EXPECT_EQ(result, "123");
}

// Test Convert<unsigned int>
TEST(ConvertTest, UnsignedIntDecode) {
    Convert<unsigned int> converter;
    unsigned int result;

    converter.decode("123", result);
    EXPECT_EQ(result, 123);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, UnsignedIntEncode) {
    Convert<unsigned int> converter;
    std::string result;

    converter.encode(123, result);
    EXPECT_EQ(result, "123");
}

// Test Convert<long>
TEST(ConvertTest, LongDecode) {
    Convert<long> converter;
    long result;

    converter.decode("123", result);
    EXPECT_EQ(result, 123);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, LongEncode) {
    Convert<long> converter;
    std::string result;

    converter.encode(123, result);
    EXPECT_EQ(result, "123");
}

// Test Convert<unsigned long>
TEST(ConvertTest, UnsignedLongDecode) {
    Convert<unsigned long> converter;
    unsigned long result;

    converter.decode("123", result);
    EXPECT_EQ(result, 123);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, UnsignedLongEncode) {
    Convert<unsigned long> converter;
    std::string result;

    converter.encode(123, result);
    EXPECT_EQ(result, "123");
}

// Test Convert<double>
TEST(ConvertTest, DoubleDecode) {
    Convert<double> converter;
    double result;

    converter.decode("123.45", result);
    EXPECT_EQ(result, 123.45);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, DoubleEncode) {
    Convert<double> converter;
    std::string result;

    converter.encode(123.45, result);
    EXPECT_EQ(result, "123.450000");
}

// Test Convert<float>
TEST(ConvertTest, FloatDecode) {
    Convert<float> converter;
    float result;

    converter.decode("123.45", result);
    EXPECT_EQ(result, 123.45f);

    EXPECT_THROW(converter.decode("INVALID", result), std::invalid_argument);
}

TEST(ConvertTest, FloatEncode) {
    Convert<float> converter;
    std::string result;

    converter.encode(123.45f, result);
    EXPECT_EQ(result, "123.450000");
}

// Test Convert<std::string>
TEST(ConvertTest, StringDecode) {
    Convert<std::string> converter;
    std::string result;

    converter.decode("Hello, World!", result);
    EXPECT_EQ(result, "Hello, World!");
}

TEST(ConvertTest, StringEncode) {
    Convert<std::string> converter;
    std::string result;

    converter.encode("Hello, World!", result);
    EXPECT_EQ(result, "Hello, World!");
}

#ifdef __cpp_lib_string_view
// Test Convert<std::string_view>
TEST(ConvertTest, StringViewDecode) {
    Convert<std::string_view> converter;
    std::string_view result;

    converter.decode("Hello, World!", result);
    EXPECT_EQ(result, "Hello, World!");
}

TEST(ConvertTest, StringViewEncode) {
    Convert<std::string_view> converter;
    std::string result;

    converter.encode("Hello, World!", result);
    EXPECT_EQ(result, "Hello, World!");
}
#endif

// Test Convert<const char*>
TEST(ConvertTest, ConstCharPtrDecode) {
    Convert<const char*> converter;
    const char* result;

    converter.decode("Hello, World!", result);
    EXPECT_STREQ(result, "Hello, World!");
}

TEST(ConvertTest, ConstCharPtrEncode) {
    Convert<const char*> converter;
    std::string result;

    const char* value = "Hello, World!";
    converter.encode(value, result);
    EXPECT_EQ(result, "Hello, World!");
}

// Test Convert<char[N]>
TEST(ConvertTest, CharArrayDecode) {
    Convert<char[20]> converter;
    char result[20];

    converter.decode("Hello, World!", result);
    EXPECT_STREQ(result, "Hello, World!");

    EXPECT_THROW(
        converter.decode("This string is too long for the array", result),
        std::invalid_argument);
}

TEST(ConvertTest, CharArrayEncode) {
    Convert<char[20]> converter;
    std::string result;

    char value[20] = "Hello, World!";
    converter.encode(value, result);
    EXPECT_EQ(result, "Hello, World!");
}
