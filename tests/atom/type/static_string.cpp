#include <gtest/gtest.h>

#include "atom/type/static_string.hpp"

// Testing default constructor
TEST(StaticStringTest, DefaultConstructor) {
    StaticString<10> str;
    EXPECT_EQ(str.size(), 0);
    EXPECT_STREQ(str.cStr(), "");
}

// Testing string initialization
TEST(StaticStringTest, StringInitialization) {
    StaticString<5> str("Hello");
    EXPECT_EQ(str.size(), 5);
    EXPECT_STREQ(str.cStr(), "Hello");
}

// Testing edge case of empty string
TEST(StaticStringTest, EmptyStringInitialization) {
    StaticString<0> str("");
    EXPECT_EQ(str.size(), 0);
    EXPECT_STREQ(str.cStr(), "");
}

// Testing string comparison
TEST(StaticStringTest, StringComparison) {
    StaticString<5> str1("Hello");
    StaticString<5> str2("Hello");
    StaticString<5> str3("World");

    EXPECT_TRUE(str1 == str2);
    EXPECT_FALSE(str1 == str3);
    EXPECT_TRUE(str1 != str3);
}

// Testing string addition with character
TEST(StaticStringTest, AdditionWithCharacter) {
    StaticString<4> str("Hell");
    str += 'o';
    EXPECT_STREQ(str.cStr(), "Hello");
}

// Testing string addition to produce new StaticString
TEST(StaticStringTest, AdditionWithCharacterProducesNewString) {
    StaticString<4> str("Hell");
    auto new_str = str + 'o';
    EXPECT_STREQ(new_str.cStr(), "Hello");
    EXPECT_EQ(new_str.size(), 5);
}

// Testing string concatenation between two StaticString objects
TEST(StaticStringTest, ConcatenationOfTwoStaticStrings) {
    StaticString<5> str1("Hello");
    StaticString<5> str2("World");
    auto result = str1 + str2;

    EXPECT_EQ(result.size(), 10);
    EXPECT_STREQ(result.cStr(), "HelloWorld");
}

// Testing string comparison operators
TEST(StaticStringTest, StringComparisonOperators) {
    StaticString<5> str1("Apple");
    StaticString<6> str2("Banana");
    StaticString<5> str3("Apple");

    EXPECT_LT(str1, str2);
    EXPECT_LE(str1, str2);
    EXPECT_LE(str1, str3);
    EXPECT_GT(str2, str1);
    EXPECT_GE(str2, str1);
    EXPECT_GE(str3, str1);
}

// Testing edge case of adding a character to a full StaticString
TEST(StaticStringTest, AddCharacterToFullStaticString) {
    StaticString<5> str("Hello");
    str += '!';
    EXPECT_STREQ(str.cStr(), "Hello");
    EXPECT_EQ(str.size(), 5);
}

// Testing concatenation that would overflow the buffer
TEST(StaticStringTest, ConcatenationWithOverflow) {
    StaticString<5> str1("Hello");
    StaticString<3> str2("!!!");
    auto result = str1 + str2;

    EXPECT_EQ(result.size(), 8);
    EXPECT_STREQ(result.cStr(), "Hello!!!");
}

// Testing comparison with std::string_view
TEST(StaticStringTest, ComparisonWithStringView) {
    StaticString<5> str("Hello");
    std::string_view sv("Hello");
    EXPECT_TRUE(str == sv);
    EXPECT_FALSE(str != sv);
}
