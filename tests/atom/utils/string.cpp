#include "atom/utils/string.hpp"
#include <gtest/gtest.h>
#include <string_view>

using namespace atom::utils;

TEST(StringUtilsTest, HasUppercase) {
    EXPECT_TRUE(hasUppercase("Hello"));
    EXPECT_FALSE(hasUppercase("hello"));
}

TEST(StringUtilsTest, ToUnderscore) {
    EXPECT_EQ(toUnderscore("HelloWorld"), "hello_world");
    EXPECT_EQ(toUnderscore("helloWorld"), "hello_world");
    EXPECT_EQ(toUnderscore("Hello World"), "hello_world");
}

TEST(StringUtilsTest, ToCamelCase) {
    EXPECT_EQ(toCamelCase("hello_world"), "helloWorld");
    EXPECT_EQ(toCamelCase("Hello_world"), "helloWorld");
    EXPECT_EQ(toCamelCase("hello world"), "helloWorld");
}

TEST(StringUtilsTest, URLEncode) {
    EXPECT_EQ(urlEncode("hello world"), "hello%20world");
    EXPECT_EQ(urlEncode("a+b=c"), "a%2Bb%3Dc");
}

TEST(StringUtilsTest, URLDecode) {
    EXPECT_EQ(urlDecode("hello%20world"), "hello world");
    EXPECT_EQ(urlDecode("a%2Bb%3Dc"), "a+b=c");
}

TEST(StringUtilsTest, StartsWith) {
    EXPECT_TRUE(startsWith("hello world", "hello"));
    EXPECT_FALSE(startsWith("hello world", "world"));
}

TEST(StringUtilsTest, EndsWith) {
    EXPECT_TRUE(endsWith("hello world", "world"));
    EXPECT_FALSE(endsWith("hello world", "hello"));
}

TEST(StringUtilsTest, SplitString) {
    std::vector<std::string> result = splitString("a,b,c", ',');
    std::vector<std::string> expected = {"a", "b", "c"};
    EXPECT_EQ(result, expected);
}

TEST(StringUtilsTest, JoinStrings) {
    std::vector<std::string_view> input = {"a", "b", "c"};
    EXPECT_EQ(joinStrings(input, ","), "a,b,c");
}

TEST(StringUtilsTest, ReplaceString) {
    EXPECT_EQ(replaceString("hello world", "world", "universe"),
              "hello universe");
    EXPECT_EQ(replaceString("hello world world", "world", "universe"),
              "hello universe universe");
}

TEST(StringUtilsTest, ReplaceStrings) {
    std::vector<std::pair<std::string_view, std::string_view>> replacements = {
        {"world", "universe"}, {"hello", "hi"}};
    EXPECT_EQ(replaceStrings("hello world", replacements), "hi universe");
}

TEST(StringUtilsTest, SVVtoSV) {
    std::vector<std::string_view> svv = {"a", "b", "c"};
    std::vector<std::string> result = SVVtoSV(svv);
    std::vector<std::string> expected = {"a", "b", "c"};
    EXPECT_EQ(result, expected);
}

TEST(StringUtilsTest, Explode) {
    std::vector<std::string> result = explode("a,b,c", ',');
    std::vector<std::string> expected = {"a", "b", "c"};
    EXPECT_EQ(result, expected);
}

TEST(StringUtilsTest, Trim) {
    EXPECT_EQ(trim("  hello  "), "hello");
    EXPECT_EQ(trim("\nhello\n", "\n"), "hello");
    EXPECT_EQ(trim("\thello\t"), "hello");
}

TEST(StringUtilsTest, StringToWString) {
    EXPECT_EQ(stringToWString("hello"), L"hello");
}

TEST(StringUtilsTest, WStringToString) {
    EXPECT_EQ(wstringToString(L"hello"), "hello");
}