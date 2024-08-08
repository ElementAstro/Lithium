#include <gtest/gtest.h>

#include "atom/algorithm/bignumber.hpp"

using namespace atom::algorithm;

// Helper function to compare BigNumber with expected string
void compareBigNumber(const BigNumber& bn, const std::string& expected) {
    EXPECT_EQ(bn.getString(), expected);
}

// Test constructor from string
TEST(BigNumberTest, ConstructorFromString) {
    BigNumber bn1("12345");
    compareBigNumber(bn1, "12345");

    BigNumber bn2("-67890");
    compareBigNumber(bn2, "-67890");

    BigNumber bn3("0000123");
    compareBigNumber(bn3, "123");
}

// Test constructor from long long
TEST(BigNumberTest, ConstructorFromLongLong) {
    BigNumber bn1(12345);
    compareBigNumber(bn1, "12345");

    BigNumber bn2(-67890);
    compareBigNumber(bn2, "-67890");

    BigNumber bn3(0);
    compareBigNumber(bn3, "0");
}

// Test addition
TEST(BigNumberTest, Addition) {
    BigNumber bn1("12345");
    BigNumber bn2("67890");
    BigNumber result = bn1 + bn2;
    compareBigNumber(result, "80235");

    BigNumber bn3("-12345");
    result = bn1 + bn3;
    compareBigNumber(result, "0");

    BigNumber bn4("9999999999999999999999999999");
    BigNumber bn5("1");
    result = bn4 + bn5;
    compareBigNumber(result, "10000000000000000000000000000");
}

// Test subtraction
TEST(BigNumberTest, Subtraction) {
    BigNumber bn1("12345");
    BigNumber bn2("67890");
    BigNumber result = bn1 - bn2;
    compareBigNumber(result, "-55545");

    BigNumber bn3("-12345");
    result = bn1 - bn3;
    compareBigNumber(result, "24690");

    BigNumber bn4("10000000000000000000000000000");
    BigNumber bn5("1");
    result = bn4 - bn5;
    compareBigNumber(result, "9999999999999999999999999999");
}

// Test multiplication
TEST(BigNumberTest, Multiplication) {
    BigNumber bn1("12345");
    BigNumber bn2("67890");
    BigNumber result = bn1 * bn2;
    compareBigNumber(result, "838102050");

    BigNumber bn3("-12345");
    result = bn1 * bn3;
    compareBigNumber(result, "-152399025");

    BigNumber bn4("0");
    result = bn1 * bn4;
    compareBigNumber(result, "0");

    BigNumber bn5("9999999999999999999999999999");
    BigNumber bn6("1");
    result = bn5 * bn6;
    compareBigNumber(result, "9999999999999999999999999999");
}

// Test division
TEST(BigNumberTest, Division) {
    BigNumber bn1("12345");
    BigNumber bn2("5");
    BigNumber result = bn1 / bn2;
    compareBigNumber(result, "2469");

    BigNumber bn3("-12345");
    result = bn1 / bn3;
    compareBigNumber(result, "-1");

    BigNumber bn4("10000000000000000000000000000");
    BigNumber bn5("1");
    result = bn4 / bn5;
    compareBigNumber(result, "10000000000000000000000000000");

    BigNumber bn6("12345");
    result = bn6 / bn1;
    compareBigNumber(result, "1");
}

// Test power
TEST(BigNumberTest, Power) {
    BigNumber bn1("2");
    BigNumber result = bn1.pow(10);
    compareBigNumber(result, "1024");

    BigNumber bn2("10");
    result = bn2.pow(0);
    compareBigNumber(result, "1");

    BigNumber bn3("-2");
    result = bn3.pow(3);
    compareBigNumber(result, "-8");

    BigNumber bn4("123456789");
    result = bn4.pow(1);
    compareBigNumber(result, "123456789");
}

// Test equality
TEST(BigNumberTest, Equality) {
    BigNumber bn1("12345");
    BigNumber bn2("12345");
    EXPECT_TRUE(bn1 == bn2);

    BigNumber bn3("-12345");
    EXPECT_FALSE(bn1 == bn3);

    BigNumber bn4("123450");
    EXPECT_FALSE(bn1 == bn4);

    BigNumber bn5("0");
    BigNumber bn6("0");
    EXPECT_TRUE(bn5 == bn6);
}

// Test comparison operators
TEST(BigNumberTest, ComparisonOperators) {
    BigNumber bn1("12345");
    BigNumber bn2("67890");
    EXPECT_TRUE(bn2 > bn1);
    EXPECT_TRUE(bn1 < bn2);
    EXPECT_TRUE(bn1 <= bn2);
    EXPECT_TRUE(bn2 >= bn1);

    BigNumber bn3("12345");
    EXPECT_TRUE(bn1 >= bn3);
    EXPECT_TRUE(bn1 <= bn3);
}

// Test negation
TEST(BigNumberTest, Negation) {
    BigNumber bn1("12345");
    BigNumber result = bn1.negate();
    compareBigNumber(result, "-12345");

    BigNumber bn2("-67890");
    result = bn2.negate();
    compareBigNumber(result, "67890");
}

// Test increment and decrement
TEST(BigNumberTest, IncrementDecrement) {
    BigNumber bn1("12345");
    ++bn1;
    compareBigNumber(bn1, "12346");

    --bn1;
    compareBigNumber(bn1, "12345");

    bn1++;
    compareBigNumber(bn1, "12346");

    bn1--;
    compareBigNumber(bn1, "12345");
}

// Test isEven and isOdd
TEST(BigNumberTest, IsEvenIsOdd) {
    BigNumber bn1("12345");
    EXPECT_TRUE(bn1.isOdd());
    EXPECT_FALSE(bn1.isEven());

    BigNumber bn2("67890");
    EXPECT_TRUE(bn2.isEven());
    EXPECT_FALSE(bn2.isOdd());

    BigNumber bn3("0");
    EXPECT_TRUE(bn3.isEven());
    EXPECT_FALSE(bn3.isOdd());
}

// Test digits count
TEST(BigNumberTest, DigitsCount) {
    BigNumber bn1("12345");
    EXPECT_EQ(bn1.digits(), 5);

    BigNumber bn2("-67890");
    EXPECT_EQ(bn2.digits(), 5);

    BigNumber bn3("0");
    EXPECT_EQ(bn3.digits(), 1);

    BigNumber bn4("10000000000000000000000000000");
    EXPECT_EQ(bn4.digits(), 29);
}