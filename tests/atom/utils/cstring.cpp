#include "atom/utils/cstring.hpp"
#include <gtest/gtest.h>

using namespace atom::utils;

TEST(CStringTest, Deduplicate) {
    const char input[] = "aabbcc";
    auto result = deduplicate(input);
    constexpr std::array<char, 7> expected = {'a',  'b',  'c', '\0',
                                              '\0', '\0', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, Split) {
    constexpr auto result = split("a,b,c,d", ',');
    constexpr std::array<std::string_view, 10> expected = {
        "a", "b", "c", "d", "", "", "", "", "", ""};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, Replace) {
    constexpr auto result = replace("aabbcc", 'b', 'd');
    constexpr std::array<char, 7> expected = {'a', 'a', 'd', 'd',
                                              'c', 'c', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, ToLower) {
    constexpr auto result = toLower("ABC");
    constexpr std::array<char, 4> expected = {'a', 'b', 'c', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, ToUpper) {
    constexpr auto result = toUpper("abc");
    constexpr std::array<char, 4> expected = {'A', 'B', 'C', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, Concat) {
    constexpr auto result = concat("Hello", "World");
    constexpr std::array<char, 11> expected = {'H', 'e', 'l', 'l', 'o', 'W',
                                               'o', 'r', 'l', 'd', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, Trim) {
    constexpr auto result = trim(" a b c ");
    constexpr std::array<char, 7> expected = {'a',  'b',  'c', '\0',
                                              '\0', '\0', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, Substring) {
    constexpr auto result = substring("Hello, World", 7, 5);
    constexpr std::array<char, 13> expected = {'W',  'o',  'r',  'l',  'd',
                                               '\0', '\0', '\0', '\0', '\0',
                                               '\0', '\0', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(CStringTest, Equal) {
    constexpr bool result = equal("Hello", "Hello");
    EXPECT_TRUE(result);

    constexpr bool not_equal = equal("Hello", "World");
    EXPECT_FALSE(not_equal);
}

TEST(CStringTest, Find) {
    constexpr std::size_t result = find("Hello", 'e');
    EXPECT_EQ(result, 1);

    constexpr std::size_t not_found = find("Hello", 'x');
    EXPECT_EQ(not_found, 5);  // N - 1
}

TEST(CStringTest, Length) {
    constexpr std::size_t result = length("Hello");
    EXPECT_EQ(result, 5);
}

TEST(CStringTest, Reverse) {
    constexpr auto result = reverse("Hello");
    constexpr std::array<char, 6> expected = {'o', 'l', 'l', 'e', 'H', '\0'};
    EXPECT_EQ(result, expected);
}

TEST(DeduplicateTest, HandlesEmptyString) {
    const char input[] = "";
    auto result = deduplicate(input);
    EXPECT_EQ(result[0], '\0');
}

TEST(DeduplicateTest, HandlesNoDuplicates) {
    const char input[] = "abc";
    auto result = deduplicate(input);
    EXPECT_EQ(result[0], 'a');
    EXPECT_EQ(result[1], 'b');
    EXPECT_EQ(result[2], 'c');
    EXPECT_EQ(result[3], '\0');
}

TEST(DeduplicateTest, RemovesDuplicates) {
    const char input[] = "banana";
    auto result = deduplicate(input);
    EXPECT_EQ(result[0], 'b');
    EXPECT_EQ(result[1], 'a');
    EXPECT_EQ(result[2], 'n');
    EXPECT_EQ(result[3], '\0');
}

TEST(DeduplicateTest, HandlesAllDuplicates) {
    const char input[] = "aaaa";
    auto result = deduplicate(input);
    EXPECT_EQ(result[0], 'a');
    EXPECT_EQ(result[1], '\0');
}

TEST(DeduplicateTest, HandlesCaseSensitivity) {
    const char input[] = "AaAa";
    auto result = deduplicate(input);
    EXPECT_EQ(result[0], 'A');
    EXPECT_EQ(result[1], 'a');
    EXPECT_EQ(result[2], '\0');
}

TEST(SplitTest, NormalCase) {
    const char input[] = "hello,world,this,is,a,test";
    auto [result, count] = split(input, ',');
    ASSERT_EQ(count, 6);
    EXPECT_STREQ(result[0].data(), "hello");
    EXPECT_STREQ(result[1].data(), "world");
    EXPECT_STREQ(result[2].data(), "this");
    EXPECT_STREQ(result[3].data(), "is");
    EXPECT_STREQ(result[4].data(), "a");
    EXPECT_STREQ(result[5].data(), "test");
}

TEST(SplitTest, NoDelimiter) {
    const char input[] = "hello";
    auto [result, count] = split(input, ',');
    ASSERT_EQ(count, 1);
    EXPECT_STREQ(result[0].data(), "hello");
}

TEST(SplitTest, EmptyString) {
    const char input[] = "";
    auto [result, count] = split(input, ',');
    ASSERT_EQ(count, 1);
    EXPECT_STREQ(result[0].data(), "");
}

TEST(SplitTest, DelimiterAtEnd) {
    const char input[] = "test,";
    auto [result, count] = split(input, ',');
    ASSERT_EQ(count, 2);
    EXPECT_STREQ(result[0].data(), "test");
    EXPECT_STREQ(result[1].data(), "");
}

class TrimTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TrimTest, HandlesEmptyString) { EXPECT_EQ(trim(""), ""); }

TEST_F(TrimTest, HandlesAllWhitespace) {
    EXPECT_EQ(trim("    \t\n\r\f\v"), "");
}

TEST_F(TrimTest, TrimsLeadingWhitespace) {
    EXPECT_EQ(trim("   hello"), "hello");
}

TEST_F(TrimTest, TrimsTrailingWhitespace) {
    EXPECT_EQ(trim("hello   "), "hello");
}

TEST_F(TrimTest, TrimsBothEnds) { EXPECT_EQ(trim("   hello   "), "hello"); }

TEST_F(TrimTest, PreservesInnerWhitespace) {
    EXPECT_EQ(trim("   hello   world   "), "hello   world");
}

TEST_F(TrimTest, HandlesAllTypesOfWhitespace) {
    EXPECT_EQ(trim(" \t\n\r\f\vhello \t\n\r\f\v"), "hello");
}

TEST_F(TrimTest, HandlesStringWithOnlyOneChar) { EXPECT_EQ(trim(" a "), "a"); }

TEST_F(TrimTest, PreservesStringWithNoWhitespace) {
    EXPECT_EQ(trim("hello"), "hello");
}

TEST_F(TrimTest, HandlesStringWithWhitespaceInMiddle) {
    EXPECT_EQ(trim("hello \t\n world"), "hello \t\n world");
}