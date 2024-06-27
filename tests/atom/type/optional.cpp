// optional_test.cpp
#include "atom/type/optional.hpp"
#include "gtest/gtest.h"

class OptionalTest : public ::testing::Test {};

TEST_F(OptionalTest, DefaultConstructor) {
    Optional<int> optional;
    EXPECT_FALSE(static_cast<bool>(optional));
}

TEST_F(OptionalTest, ValueConstructor) {
    Optional<int> optional(10);
    EXPECT_TRUE(static_cast<bool>(optional));
    EXPECT_EQ(optional.value(), 10);
}

TEST_F(OptionalTest, MoveConstructor) {
    Optional<int> optional1(10);
    Optional<int> optional2(std::move(optional1));
    EXPECT_TRUE(static_cast<bool>(optional2));
    EXPECT_EQ(optional2.value(), 10);
    EXPECT_FALSE(
        static_cast<bool>(optional1));  // NOLINT(bugprone-use-after-move)
}

TEST_F(OptionalTest, CopyConstructor) {
    Optional<int> optional1(10);
    Optional<int> optional2(optional1);
    EXPECT_TRUE(static_cast<bool>(optional2));
    EXPECT_EQ(optional2.value(), 10);
    EXPECT_TRUE(static_cast<bool>(optional1));
    EXPECT_EQ(optional1.value(), 10);
}

TEST_F(OptionalTest, Reset) {
    Optional<int> optional(10);
    EXPECT_TRUE(static_cast<bool>(optional));
    optional.reset();
    EXPECT_FALSE(static_cast<bool>(optional));
}

TEST_F(OptionalTest, Emplace) {
    Optional<int> optional;
    optional.emplace(10);
    EXPECT_TRUE(static_cast<bool>(optional));
    EXPECT_EQ(optional.value(), 10);
}

TEST_F(OptionalTest, PointerAccess) {
    Optional<int> optional(10);
    ASSERT_TRUE(static_cast<bool>(optional));
    EXPECT_EQ(*optional, 10);
    EXPECT_EQ(optional.value(), 10);  // Ensure operator-> doesn't throw
}

TEST_F(OptionalTest, DereferenceAccess) {
    Optional<int> optional(10);
    EXPECT_EQ(*optional, 10);
    EXPECT_EQ(optional.value(), 10);
}

TEST_F(OptionalTest, ValueOr) {
    Optional<int> optional;
    EXPECT_EQ(optional.valueOr(20), 20);

    Optional<int> optionalWithValue(10);
    EXPECT_EQ(optionalWithValue.valueOr(20), 10);
}

TEST_F(OptionalTest, Equality) {
    Optional<int> optional1(10);
    Optional<int> optional2(10);
    Optional<int> optional3;

    EXPECT_EQ(optional1, optional2);
    EXPECT_NE(optional1, optional3);
}