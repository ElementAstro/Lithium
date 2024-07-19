#include "atom/utils/cstring.hpp"
#include <gtest/gtest.h>

using namespace atom::utils;

TEST(CStringTest, Deduplicate) {
    constexpr auto RESULT = deduplicate("aabbcc");
    constexpr std::array<char, 7> EXPECTED = {'a',  'b',  'c', '\0',
                                              '\0', '\0', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(CStringTest, Replace) {
    constexpr auto RESULT = replace("aabbcc", 'b', 'd');
    constexpr std::array<char, 7> EXPECTED = {'a', 'a', 'd', 'd',
                                              'c', 'c', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(CStringTest, ToLower) {
    constexpr auto RESULT = toLower("ABC");
    constexpr std::array<char, 4> EXPECTED = {'a', 'b', 'c', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(CStringTest, ToUpper) {
    constexpr auto RESULT = toUpper("abc");
    constexpr std::array<char, 4> EXPECTED = {'A', 'B', 'C', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(CStringTest, Concat) {
    constexpr auto RESULT = concat("Hello", "World");
    constexpr std::array<char, 11> EXPECTED = {'H', 'e', 'l', 'l', 'o', 'W',
                                               'o', 'r', 'l', 'd', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(CStringTest, Substring) {
    constexpr auto RESULT = substring("Hello, World", 7, 5);
    constexpr std::array<char, 13> EXPECTED = {'W',  'o',  'r',  'l',  'd',
                                               '\0', '\0', '\0', '\0', '\0',
                                               '\0', '\0', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(CStringTest, Equal) {
    constexpr bool RESULT = equal("Hello", "Hello");
    EXPECT_TRUE(RESULT);

    constexpr bool NOT_EQUAL = equal("Hello", "World");
    EXPECT_FALSE(NOT_EQUAL);
}

TEST(CStringTest, Find) {
    constexpr std::size_t RESULT = find("Hello", 'e');
    EXPECT_EQ(RESULT, 1);

    constexpr std::size_t NOT_FOUND = find("Hello", 'x');
    EXPECT_EQ(NOT_FOUND, 5);  // N - 1
}

TEST(CStringTest, Length) {
    constexpr std::size_t RESULT = length("Hello");
    EXPECT_EQ(RESULT, 5);
}

TEST(CStringTest, Reverse) {
    constexpr auto RESULT = reverse("Hello");
    constexpr std::array<char, 6> EXPECTED = {'o', 'l', 'l', 'e', 'H', '\0'};
    EXPECT_EQ(RESULT, EXPECTED);
}

TEST(DeduplicateTest, HandlesEmptyString) {
    const char INPUT[] = "";
    auto result = deduplicate(INPUT);
    EXPECT_EQ(result[0], '\0');
}

TEST(DeduplicateTest, HandlesNoDuplicates) {
    const char INPUT[] = "abc";
    auto result = deduplicate(INPUT);
    EXPECT_EQ(result[0], 'a');
    EXPECT_EQ(result[1], 'b');
    EXPECT_EQ(result[2], 'c');
    EXPECT_EQ(result[3], '\0');
}

TEST(DeduplicateTest, RemovesDuplicates) {
    const char INPUT[] = "banana";
    auto result = deduplicate(INPUT);
    EXPECT_EQ(result[0], 'b');
    EXPECT_EQ(result[1], 'a');
    EXPECT_EQ(result[2], 'n');
    EXPECT_EQ(result[3], '\0');
}

TEST(DeduplicateTest, HandlesAllDuplicates) {
    const char INPUT[] = "aaaa";
    auto result = deduplicate(INPUT);
    EXPECT_EQ(result[0], 'a');
    EXPECT_EQ(result[1], '\0');
}

TEST(DeduplicateTest, HandlesCaseSensitivity) {
    const char INPUT[] = "AaAa";
    auto result = deduplicate(INPUT);
    EXPECT_EQ(result[0], 'A');
    EXPECT_EQ(result[1], 'a');
    EXPECT_EQ(result[2], '\0');
}

TEST(SplitTest, BasicFunctionality) {
    constexpr auto result = split("apple,banana,cherry", ',');

    ASSERT_EQ(result.size(), 20);  // 期望有4个元素
    EXPECT_EQ(result[0], "apple");
    EXPECT_EQ(result[1], "banana");
    EXPECT_EQ(result[2], "cherry");
    EXPECT_EQ(result[3], "");  // 空字符串在最后
}

TEST(SplitTest, CustomDelimiter) {
    constexpr auto result = split("apple;banana;cherry", ';');

    ASSERT_EQ(result.size(), 4);  // 期望有4个元素
    EXPECT_EQ(result[0], "apple");
    EXPECT_EQ(result[1], "banana");
    EXPECT_EQ(result[2], "cherry");
    EXPECT_EQ(result[3], "");  // 空字符串在最后
}

TEST(SplitTest, EmptyString) {
    constexpr auto result = split("", ',');

    ASSERT_EQ(result.size(), 1);  // 期望有1个元素
    EXPECT_EQ(result[0], "");     // 空字符串
}

TEST(SplitTest, OnlyDelimiters) {
    constexpr auto result = split(",,,", ',');

    ASSERT_EQ(result.size(), 4);  // 期望有4个元素
    EXPECT_EQ(result[0], "");     // 第一个子串是空字符串
    EXPECT_EQ(result[1], "");     // 第二个子串是空字符串
    EXPECT_EQ(result[2], "");     // 第三个子串是空字符串
    EXPECT_EQ(result[3], "");     // 最后一个子串是空字符串
}

TEST(SplitTest, NoDelimiters) {
    constexpr auto result = split("applebanana", ',');

    ASSERT_EQ(result.size(), 2);          // 期望有2个元素
    EXPECT_EQ(result[0], "applebanana");  // 整个字符串为一个子串
    EXPECT_EQ(result[1], "");             // 空字符串在最后
}

TEST(SplitTest, MultipleDelimiters) {
    constexpr auto result = split("apple,banana,,cherry", ',');

    ASSERT_EQ(result.size(), 5);  // 期望有5个元素
    EXPECT_EQ(result[0], "apple");
    EXPECT_EQ(result[1], "banana");
    EXPECT_EQ(result[2], "");  // 空字符串
    EXPECT_EQ(result[3], "cherry");
    EXPECT_EQ(result[4], "");  // 空字符串在最后
}

TEST(SplitTest, LeadingAndTrailingDelimiters) {
    constexpr auto result = split(",apple,banana,", ',');

    ASSERT_EQ(result.size(), 5);  // 期望有5个元素
    EXPECT_EQ(result[0], "");     // 空字符串在开始
    EXPECT_EQ(result[1], "apple");
    EXPECT_EQ(result[2], "banana");
    EXPECT_EQ(result[3], "");  // 空字符串在结尾
    EXPECT_EQ(result[4], "");  // 空字符串在结尾
}

TEST(TrimTest, BasicTrimming) {
    constexpr auto result = trim("   Hello, World!   ");
    EXPECT_STREQ(result.data(), "Hello, World!");
}

TEST(TrimTest, NoSpaces) {
    constexpr auto result = trim("NoSpaces");
    EXPECT_STREQ(result.data(), "NoSpaces");
}

TEST(TrimTest, OnlySpaces) {
    constexpr auto result = trim("     ");
    EXPECT_STREQ(result.data(), "");  // 只包含空格，返回空字符串
}

TEST(TrimTest, LeadingSpaces) {
    constexpr auto result = trim("   Leading");
    EXPECT_STREQ(result.data(), "Leading");
}

TEST(TrimTest, TrailingSpaces) {
    constexpr auto result = trim("Trailing   ");
    EXPECT_STREQ(result.data(), "Trailing");
}

TEST(TrimTest, LeadingAndTrailingSpaces) {
    constexpr auto result = trim("   Both   ");
    EXPECT_STREQ(result.data(), "Both");
}