#include "atom/algorithm/math.hpp"
#include <gtest/gtest.h>

class MathFunctionsTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

// Tests for mulDiv64
TEST_F(MathFunctionsTest, MulDiv64) {
    EXPECT_EQ(atom::algorithm::mulDiv64(10, 20, 2), 100);
}

// Tests for safeAdd
TEST_F(MathFunctionsTest, SafeAdd) {
    EXPECT_EQ(atom::algorithm::safeAdd(10, 20), 30);
    EXPECT_EQ(atom::algorithm::safeAdd(UINT64_MAX, 1), 0);  // Overflow
}

// Tests for safeMul
TEST_F(MathFunctionsTest, SafeMul) {
    EXPECT_EQ(atom::algorithm::safeMul(10, 20), 200);
    EXPECT_EQ(atom::algorithm::safeMul(UINT64_MAX, 2), 0);  // Overflow
}

// Tests for rotl64
TEST_F(MathFunctionsTest, Rotl64) {
    EXPECT_EQ(atom::algorithm::rotl64(1, 1), 2);
    EXPECT_EQ(atom::algorithm::rotl64(1, 64), 1);  // Full rotation
}

// Tests for rotr64
TEST_F(MathFunctionsTest, Rotr64) {
    EXPECT_EQ(atom::algorithm::rotr64(2, 1), 1);
    EXPECT_EQ(atom::algorithm::rotr64(1, 64), 1);  // Full rotation
}

// Tests for clz64
TEST_F(MathFunctionsTest, Clz64) {
    EXPECT_EQ(atom::algorithm::clz64(0), 64);
    EXPECT_EQ(atom::algorithm::clz64(1), 63);
    EXPECT_EQ(atom::algorithm::clz64(0x8000000000000000), 0);
}

// Tests for normalize
TEST_F(MathFunctionsTest, Normalize) {
    EXPECT_EQ(atom::algorithm::normalize(0), 0);
    EXPECT_EQ(atom::algorithm::normalize(1), UINT64_C(0x8000000000000000));
    EXPECT_EQ(atom::algorithm::normalize(0x8000000000000000),
              UINT64_C(0x8000000000000000));
}

// Tests for safeSub
TEST_F(MathFunctionsTest, SafeSub) {
    EXPECT_EQ(atom::algorithm::safeSub(20, 10), 10);
    EXPECT_EQ(atom::algorithm::safeSub(10, 20), 0);  // Underflow
}

// Tests for safeDiv
TEST_F(MathFunctionsTest, SafeDiv) {
    EXPECT_EQ(atom::algorithm::safeDiv(20, 10), 2);
    EXPECT_EQ(atom::algorithm::safeDiv(10, 0), 0);  // Division by zero
}

TEST_F(MathFunctionsTest, BitReverse64Test) {
    // Test case 1
    uint64_t input1 = 12345;
    uint64_t expected1 = 0x543210;
    EXPECT_EQ(atom::algorithm::bitReverse64(input1), expected1);

    // Test case 2
    uint64_t input2 = 987654321;
    uint64_t expected2 = 0x123456789;
    EXPECT_EQ(atom::algorithm::bitReverse64(input2), expected2);
}

TEST_F(MathFunctionsTest, ApproximateSqrtTest) {
    // Test case 1
    uint64_t input1 = 16;
    uint64_t expected1 = 4;
    EXPECT_EQ(atom::algorithm::approximateSqrt(input1), expected1);

    // Test case 2
    uint64_t input2 = 25;
    uint64_t expected2 = 5;
    EXPECT_EQ(atom::algorithm::approximateSqrt(input2), expected2);
}

TEST_F(MathFunctionsTest, Gcd64Test) {
    // Test case 1
    uint64_t a1 = 12;
    uint64_t b1 = 18;
    uint64_t expected1 = 6;
    EXPECT_EQ(atom::algorithm::gcd64(a1, b1), expected1);

    // Test case 2
    uint64_t a2 = 48;
    uint64_t b2 = 60;
    uint64_t expected2 = 12;
    EXPECT_EQ(atom::algorithm::gcd64(a2, b2), expected2);
}

TEST_F(MathFunctionsTest, Lcm64Test) {
    // Test case 1
    uint64_t a1 = 12;
    uint64_t b1 = 18;
    uint64_t expected1 = 36;
    EXPECT_EQ(atom::algorithm::lcm64(a1, b1), expected1);

    // Test case 2
    uint64_t a2 = 48;
    uint64_t b2 = 60;
    uint64_t expected2 = 240;
    EXPECT_EQ(atom::algorithm::lcm64(a2, b2), expected2);
}

TEST_F(MathFunctionsTest, IsPowerOfTwoTest) {
    // Test case 1
    uint64_t input1 = 8;
    bool expected1 = true;
    EXPECT_EQ(atom::algorithm::isPowerOfTwo(input1), expected1);

    // Test case 2
    uint64_t input2 = 15;
    bool expected2 = false;
    EXPECT_EQ(atom::algorithm::isPowerOfTwo(input2), expected2);
}

TEST_F(MathFunctionsTest, NextPowerOfTwoTest) {
    // Test case 1
    uint64_t input1 = 7;
    uint64_t expected1 = 8;
    EXPECT_EQ(atom::algorithm::nextPowerOfTwo(input1), expected1);

    // Test case 2
    uint64_t input2 = 16;
    uint64_t expected2 = 16;
    EXPECT_EQ(atom::algorithm::nextPowerOfTwo(input2), expected2);
}