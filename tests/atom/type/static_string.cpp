#include <gtest/gtest.h>

#include "atom/type/static_string.hpp"

TEST(StaticStringTest, DefaultConstructor) {
    StaticString<10> str;
    EXPECT_EQ(str.size(), 0);
    EXPECT_STREQ(str.cStr(), "");
}

// Test C-style string constructor
TEST(StaticStringTest, CStringConstructor) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    EXPECT_EQ(str.size(), 5);
    EXPECT_STREQ(str.cStr(), "Hello");
}

// Test std::string constructor
TEST(StaticStringTest, StdStringConstructor) {
    std::string s = "Hello";
    StaticString<5> str(s);
    EXPECT_EQ(str.size(), 5);
    EXPECT_STREQ(str.cStr(), "Hello");
}

// Test size method
TEST(StaticStringTest, SizeMethod) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    EXPECT_EQ(str.size(), 5);
}

// Test cStr method
TEST(StaticStringTest, CStrMethod) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    EXPECT_STREQ(str.cStr(), "Hello");
}

// Test begin and end methods
TEST(StaticStringTest, BeginEndMethods) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    EXPECT_EQ(*str.begin(), 'H');
    EXPECT_EQ(*(str.end() - 1), 'o');
}

// Test equality operators
TEST(StaticStringTest, EqualityOperators) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    EXPECT_TRUE(str == "Hello");
    EXPECT_FALSE(str == "World");
}

// Test inequality operators
TEST(StaticStringTest, InequalityOperators) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    EXPECT_TRUE(str != "World");
    EXPECT_FALSE(str != "Hello");
}

// Test append operator
TEST(StaticStringTest, AppendOperator) {
    StaticString<5> str;
    str += 'H';
    str += 'i';
    EXPECT_EQ(str.size(), 2);
    EXPECT_STREQ(str.cStr(), "Hi");
}

// Test concatenation operator
TEST(StaticStringTest, ConcatenationOperator) {
    std::array<char, 6> arr1 = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str1(arr1);
    StaticString<1> str2;
    str2 += '!';
    auto result = str1 + str2;
    EXPECT_EQ(result.size(), 6);
    EXPECT_STREQ(result.cStr(), "Hello!");
}

// Test concatenation with string literal
TEST(StaticStringTest, ConcatenationWithStringLiteral) {
    std::array<char, 6> arr = {'H', 'e', 'l', 'l', 'o', '\0'};
    StaticString<5> str(arr);
    std::array<char, 2> lit = {'!', '\0'};
    auto result = str + lit;
    EXPECT_EQ(result.size(), 6);
    EXPECT_STREQ(result.cStr(), "Hello!");
}
