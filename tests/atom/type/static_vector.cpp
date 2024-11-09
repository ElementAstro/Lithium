/*
 * test_static_vector.hpp
 *
 * Unit tests for StaticVector class
 */

#include <gtest/gtest.h>

#include "atom/type/static_vector.hpp"

// Test default constructor
TEST(StaticVectorTest, DefaultConstructor) {
    StaticVector<int, 5> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.capacity(), 5);
}

// Test initializer list constructor
TEST(StaticVectorTest, InitializerListConstructor) {
    StaticVector<int, 5> vec = {1, 2, 3};
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

// Test copy constructor
TEST(StaticVectorTest, CopyConstructor) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2 = vec1;
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
}

// Test move constructor
TEST(StaticVectorTest, MoveConstructor) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2 = std::move(vec1);
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec1.size(), 0);
}

// Test copy assignment operator
TEST(StaticVectorTest, CopyAssignment) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2;
    vec2 = vec1;
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
}

// Test move assignment operator
TEST(StaticVectorTest, MoveAssignment) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2;
    vec2 = std::move(vec1);
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec1.size(), 0);
}

// Test pushBack with copy
TEST(StaticVectorTest, PushBackCopy) {
    StaticVector<int, 5> vec;
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

// Test pushBack with move
TEST(StaticVectorTest, PushBackMove) {
    StaticVector<std::string, 5> vec;
    std::string str = "test";
    vec.pushBack(std::move(str));
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], "test");
    EXPECT_TRUE(str.empty());
}

// Test emplaceBack
TEST(StaticVectorTest, EmplaceBack) {
    StaticVector<std::pair<int, int>, 5> vec;
    vec.emplaceBack(1, 2);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].first, 1);
    EXPECT_EQ(vec[0].second, 2);
}

// Test popBack
TEST(StaticVectorTest, PopBack) {
    StaticVector<int, 5> vec = {1, 2, 3};
    vec.popBack();
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

// Test clear
TEST(StaticVectorTest, Clear) {
    StaticVector<int, 5> vec = {1, 2, 3};
    vec.clear();
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
}

// Test access operators
TEST(StaticVectorTest, AccessOperators) {
    StaticVector<int, 5> vec = {1, 2, 3};
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    vec[1] = 5;
    EXPECT_EQ(vec[1], 5);
}

// Test at with bounds checking
TEST(StaticVectorTest, At) {
    StaticVector<int, 5> vec = {1, 2, 3};
    EXPECT_EQ(vec.at(0), 1);
    EXPECT_EQ(vec.at(1), 2);
    EXPECT_EQ(vec.at(2), 3);
    EXPECT_THROW(vec.at(3), std::out_of_range);
}

// Test front and back
TEST(StaticVectorTest, FrontBack) {
    StaticVector<int, 5> vec = {1, 2, 3};
    EXPECT_EQ(vec.front(), 1);
    EXPECT_EQ(vec.back(), 3);
    vec.front() = 5;
    vec.back() = 7;
    EXPECT_EQ(vec.front(), 5);
    EXPECT_EQ(vec.back(), 7);
}

// Test iterators
TEST(StaticVectorTest, Iterators) {
    StaticVector<int, 5> vec = {1, 2, 3};
    auto it = vec.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(it, vec.end());
}

// Test reverse iterators
TEST(StaticVectorTest, ReverseIterators) {
    StaticVector<int, 5> vec = {1, 2, 3};
    auto rit = vec.rbegin();
    EXPECT_EQ(*rit, 3);
    ++rit;
    EXPECT_EQ(*rit, 2);
    ++rit;
    EXPECT_EQ(*rit, 1);
    ++rit;
    EXPECT_EQ(rit, vec.rend());
}

// Test swap
TEST(StaticVectorTest, Swap) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2 = {4, 5, 6};
    vec1.swap(vec2);
    EXPECT_EQ(vec1.size(), 3);
    EXPECT_EQ(vec1[0], 4);
    EXPECT_EQ(vec1[1], 5);
    EXPECT_EQ(vec1[2], 6);
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
}

// Test equality operator
TEST(StaticVectorTest, EqualityOperator) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2 = {1, 2, 3};
    StaticVector<int, 5> vec3 = {4, 5, 6};
    EXPECT_TRUE(vec1 == vec2);
    EXPECT_FALSE(vec1 == vec3);
}

// Test three-way comparison operator
TEST(StaticVectorTest, ThreeWayComparisonOperator) {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2 = {1, 2, 3};
    StaticVector<int, 5> vec3 = {4, 5, 6};
    EXPECT_TRUE((vec1 <=> vec2) == 0);
    EXPECT_TRUE((vec1 <=> vec3) < 0);
    EXPECT_TRUE((vec3 <=> vec1) > 0);
}
