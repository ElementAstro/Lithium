#include "atom/algorithm/math.hpp"
#include <gtest/gtest.h>
#include "exception.hpp"

using namespace atom::algorithm;

class MathFunctionsTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

// Tests for mulDiv64
TEST_F(MathFunctionsTest, MulDiv64) {
    EXPECT_EQ(mulDiv64(10, 20, 2), 100);
}

// Tests for safeAdd
TEST_F(MathFunctionsTest, SafeAdd) {
    EXPECT_EQ(safeAdd(10, 20), 30);
    EXPECT_THROW(safeAdd(UINT64_MAX, 1),
                 atom::error::OverflowException);
}

// Tests for safeMul
TEST_F(MathFunctionsTest, SafeMul) {
    EXPECT_EQ(safeMul(10, 20), 200);
    EXPECT_THROW(safeMul(UINT64_MAX, 2),
                 atom::error::OverflowException);
}

// Tests for rotl64
TEST_F(MathFunctionsTest, Rotl64) {
    EXPECT_EQ(rotl64(1, 1), 2);
    EXPECT_EQ(rotl64(1, 64), 1);  // Full rotation
}

// Tests for rotr64
TEST_F(MathFunctionsTest, Rotr64) {
    EXPECT_EQ(rotr64(2, 1), 1);
    EXPECT_EQ(rotr64(1, 64), 1);  // Full rotation
}

// Tests for clz64
TEST_F(MathFunctionsTest, Clz64) {
    EXPECT_EQ(clz64(0), 64);
    EXPECT_EQ(clz64(1), 63);
    EXPECT_EQ(clz64(0x8000000000000000), 0);
}

// Tests for normalize
TEST_F(MathFunctionsTest, Normalize) {
    EXPECT_EQ(normalize(0), 0);
    EXPECT_EQ(normalize(1), UINT64_C(0x8000000000000000));
    EXPECT_EQ(normalize(0x8000000000000000),
              UINT64_C(0x8000000000000000));
}

// Tests for safeSub
TEST_F(MathFunctionsTest, SafeSub) {
    EXPECT_EQ(safeSub(20, 10), 10);
    EXPECT_THROW(safeSub(10, 20),
                 atom::error::UnderflowException);
}

// Tests for safeDiv
TEST_F(MathFunctionsTest, SafeDiv) {
    EXPECT_EQ(safeDiv(20, 10), 2);
    EXPECT_THROW(safeDiv(10, 0), atom::error::InvalidArgument);
}

TEST_F(MathFunctionsTest, BitReverse64Test) {
    EXPECT_EQ(bitReverse64(0x0123456789ABCDEF), 0xF7B3D591E6A2C480);
    EXPECT_EQ(bitReverse64(0xFFFFFFFFFFFFFFFF), 0xFFFFFFFFFFFFFFFF);
    EXPECT_EQ(bitReverse64(0x0000000000000000), 0x0000000000000000);
    EXPECT_EQ(bitReverse64(0x8000000000000000), 0x0000000000000001);
    EXPECT_EQ(bitReverse64(0x0000000000000001), 0x8000000000000000);
    EXPECT_EQ(bitReverse64(0x123456789ABCDEF0), 0x0F7B3D591E6A2C48);
}

TEST_F(MathFunctionsTest, ApproximateSqrtTest) {
    // Test case 1
    uint64_t input1 = 16;
    uint64_t expected1 = 4;
    EXPECT_EQ(approximateSqrt(input1), expected1);

    // Test case 2
    uint64_t input2 = 25;
    uint64_t expected2 = 5;
    EXPECT_EQ(approximateSqrt(input2), expected2);
}

TEST_F(MathFunctionsTest, Gcd64Test) {
    // Test case 1
    uint64_t a1 = 12;
    uint64_t b1 = 18;
    uint64_t expected1 = 6;
    EXPECT_EQ(gcd64(a1, b1), expected1);

    // Test case 2
    uint64_t a2 = 48;
    uint64_t b2 = 60;
    uint64_t expected2 = 12;
    EXPECT_EQ(gcd64(a2, b2), expected2);
}

TEST_F(MathFunctionsTest, Lcm64Test) {
    // Test case 1
    uint64_t a1 = 12;
    uint64_t b1 = 18;
    uint64_t expected1 = 36;
    EXPECT_EQ(lcm64(a1, b1), expected1);

    // Test case 2
    uint64_t a2 = 48;
    uint64_t b2 = 60;
    uint64_t expected2 = 240;
    EXPECT_EQ(lcm64(a2, b2), expected2);
}

TEST_F(MathFunctionsTest, IsPowerOfTwoTest) {
    // Test case 1
    uint64_t input1 = 8;
    bool expected1 = true;
    EXPECT_EQ(isPowerOfTwo(input1), expected1);

    // Test case 2
    uint64_t input2 = 15;
    bool expected2 = false;
    EXPECT_EQ(isPowerOfTwo(input2), expected2);
}

TEST_F(MathFunctionsTest, NextPowerOfTwoTest) {
    // Test case 1
    uint64_t input1 = 7;
    uint64_t expected1 = 8;
    EXPECT_EQ(nextPowerOfTwo(input1), expected1);

    // Test case 2
    uint64_t input2 = 16;
    uint64_t expected2 = 16;
    EXPECT_EQ(nextPowerOfTwo(input2), expected2);
}
