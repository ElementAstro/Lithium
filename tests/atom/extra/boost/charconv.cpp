#include "atom/extra/boost/charconv.hpp"

#include <gtest/gtest.h>

using namespace atom::extra::boost;

class BoostCharConvTest : public ::testing::Test {
protected:
    FormatOptions defaultOptions;
};

// Test intToString with default options
TEST_F(BoostCharConvTest, IntToStringDefault) {
    EXPECT_EQ(BoostCharConv::intToString(123), "123");
    EXPECT_EQ(BoostCharConv::intToString(-123), "-123");
}

// Test intToString with different bases
TEST_F(BoostCharConvTest, IntToStringBase) {
    EXPECT_EQ(BoostCharConv::intToString(255, 16), "ff");
    EXPECT_EQ(BoostCharConv::intToString(255, 2), "11111111");
}

// Test intToString with thousands separator
TEST_F(BoostCharConvTest, IntToStringThousandsSeparator) {
    FormatOptions options;
    options.thousandsSeparator = ',';
    EXPECT_EQ(BoostCharConv::intToString(1234567, 10, options), "1,234,567");
}

// Test floatToString with default options
TEST_F(BoostCharConvTest, FloatToStringDefault) {
    EXPECT_EQ(BoostCharConv::floatToString(123.456), "123.456");
    EXPECT_EQ(BoostCharConv::floatToString(-123.456), "-123.456");
}

// Test floatToString with precision
TEST_F(BoostCharConvTest, FloatToStringPrecision) {
    FormatOptions options;
    options.precision = 2;
    EXPECT_EQ(BoostCharConv::floatToString(123.456, options), "123.46");
}

// Test floatToString with scientific format
TEST_F(BoostCharConvTest, FloatToStringScientific) {
    FormatOptions options;
    options.format = NumberFormat::SCIENTIFIC;
    EXPECT_EQ(BoostCharConv::floatToString(123.456, options), "1.23456e+02");
}

// Test floatToString with thousands separator
TEST_F(BoostCharConvTest, FloatToStringThousandsSeparator) {
    FormatOptions options;
    options.thousandsSeparator = ',';
    EXPECT_EQ(BoostCharConv::floatToString(1234567.89, options),
              "1,234,567.89");
}

// Test stringToInt with valid input
TEST_F(BoostCharConvTest, StringToIntValid) {
    EXPECT_EQ(BoostCharConv::stringToInt<int>("123"), 123);
    EXPECT_EQ(BoostCharConv::stringToInt<int>("-123"), -123);
}

// Test stringToInt with invalid input
TEST_F(BoostCharConvTest, StringToIntInvalid) {
    EXPECT_THROW(BoostCharConv::stringToInt<int>("abc"), std::runtime_error);
}

// Test stringToFloat with valid input
TEST_F(BoostCharConvTest, StringToFloatValid) {
    EXPECT_EQ(BoostCharConv::stringToFloat<double>("123.456"), 123.456);
    EXPECT_EQ(BoostCharConv::stringToFloat<double>("-123.456"), -123.456);
}

// Test stringToFloat with invalid input
TEST_F(BoostCharConvTest, StringToFloatInvalid) {
    EXPECT_THROW(BoostCharConv::stringToFloat<double>("abc"),
                 std::runtime_error);
}

// Test toString with integer
TEST_F(BoostCharConvTest, ToStringInt) {
    EXPECT_EQ(BoostCharConv::toString(123), "123");
}

// Test toString with floating point
TEST_F(BoostCharConvTest, ToStringFloat) {
    EXPECT_EQ(BoostCharConv::toString(123.456), "123.456");
}

// Test fromString with integer
TEST_F(BoostCharConvTest, FromStringInt) {
    EXPECT_EQ(BoostCharConv::fromString<int>("123"), 123);
}

// Test fromString with floating point
TEST_F(BoostCharConvTest, FromStringFloat) {
    EXPECT_EQ(BoostCharConv::fromString<double>("123.456"), 123.456);
}

// Test specialValueToString with NaN
TEST_F(BoostCharConvTest, SpecialValueToStringNaN) {
    EXPECT_EQ(BoostCharConv::specialValueToString(std::nan("")), "NaN");
}

// Test specialValueToString with positive infinity
TEST_F(BoostCharConvTest, SpecialValueToStringPosInf) {
    EXPECT_EQ(BoostCharConv::specialValueToString(
                  std::numeric_limits<double>::infinity()),
              "Inf");
}

// Test specialValueToString with negative infinity
TEST_F(BoostCharConvTest, SpecialValueToStringNegInf) {
    EXPECT_EQ(BoostCharConv::specialValueToString(
                  -std::numeric_limits<double>::infinity()),
              "-Inf");
}
