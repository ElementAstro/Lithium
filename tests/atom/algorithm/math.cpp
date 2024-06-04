#include "atom/algorithm/math.hpp"
#include <gtest/gtest.h>

// Tests for mulDiv64
TEST(AlgorithmTest, MulDiv64) {
    EXPECT_EQ(atom::algorithm::mulDiv64(10, 20, 2), 100);
}

// Tests for safeAdd
TEST(AlgorithmTest, SafeAdd) {
    EXPECT_EQ(atom::algorithm::safeAdd(10, 20), 30);
    EXPECT_EQ(atom::algorithm::safeAdd(UINT64_MAX, 1), 0);  // Overflow
}

// Tests for safeMul
TEST(AlgorithmTest, SafeMul) {
    EXPECT_EQ(atom::algorithm::safeMul(10, 20), 200);
    EXPECT_EQ(atom::algorithm::safeMul(UINT64_MAX, 2), 0);  // Overflow
}

// Tests for rotl64
TEST(AlgorithmTest, Rotl64) {
    EXPECT_EQ(atom::algorithm::rotl64(1, 1), 2);
    EXPECT_EQ(atom::algorithm::rotl64(1, 64), 1);  // Full rotation
}

// Tests for rotr64
TEST(AlgorithmTest, Rotr64) {
    EXPECT_EQ(atom::algorithm::rotr64(2, 1), 1);
    EXPECT_EQ(atom::algorithm::rotr64(1, 64), 1);  // Full rotation
}

// Tests for clz64
TEST(AlgorithmTest, Clz64) {
    EXPECT_EQ(atom::algorithm::clz64(0), 64);
    EXPECT_EQ(atom::algorithm::clz64(1), 63);
    EXPECT_EQ(atom::algorithm::clz64(0x8000000000000000), 0);
}

// Tests for normalize
TEST(AlgorithmTest, Normalize) {
    EXPECT_EQ(atom::algorithm::normalize(0), 0);
    EXPECT_EQ(atom::algorithm::normalize(1), UINT64_C(0x8000000000000000));
    EXPECT_EQ(atom::algorithm::normalize(0x8000000000000000),
              UINT64_C(0x8000000000000000));
}

// Tests for safeSub
TEST(AlgorithmTest, SafeSub) {
    EXPECT_EQ(atom::algorithm::safeSub(20, 10), 10);
    EXPECT_EQ(atom::algorithm::safeSub(10, 20), 0);  // Underflow
}

// Tests for safeDiv
TEST(AlgorithmTest, SafeDiv) {
    EXPECT_EQ(atom::algorithm::safeDiv(20, 10), 2);
    EXPECT_EQ(atom::algorithm::safeDiv(10, 0), 0);  // Division by zero
}
