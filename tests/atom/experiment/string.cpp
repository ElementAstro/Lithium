#include <gtest/gtest.h>
#include "atom/experiment/string.hpp"

TEST(StringTest, DefaultConstructor)
{
    String str;
    EXPECT_EQ(str.length(), 0);
    EXPECT_STREQ(str.toCharArray(), "");
}

TEST(StringTest, CStyleStringConstructor)
{
    const char *cstr = "Hello";
    String str(cstr);
    EXPECT_EQ(str.length(), 5);
    EXPECT_STREQ(str.toCharArray(), cstr);
}

TEST(StringTest, StdStringConstructor)
{
    std::string stdstr = "World";
    String str(stdstr);
    EXPECT_EQ(str.length(), 5);
    EXPECT_STREQ(str.toCharArray(), stdstr.c_str());
}

TEST(StringTest, CopyConstructor)
{
    String str1("Hello");
    String str2(str1);
    EXPECT_EQ(str2.length(), 5);
    EXPECT_STREQ(str2.toCharArray(), "Hello");
}

TEST(StringTest, AssignmentOperator)
{
    String str1("Hello");
    String str2;
    str2 = str1;
    EXPECT_EQ(str2.length(), 5);
    EXPECT_STREQ(str2.toCharArray(), "Hello");
}

TEST(StringTest, EqualityOperator)
{
    String str1("Hello");
    String str2("Hello");
    String str3("World");
    EXPECT_TRUE(str1 == str2);
    EXPECT_FALSE(str1 == str3);
}

TEST(StringTest, InequalityOperator)
{
    String str1("Hello");
    String str2("Hello");
    String str3("World");
    EXPECT_FALSE(str1 != str2);
    EXPECT_TRUE(str1 != str3);
}

TEST(StringTest, LessThanOperator)
{
    String str1("Hello");
    String str2("World");
    EXPECT_TRUE(str1 < str2);
    EXPECT_FALSE(str2 < str1);
}

TEST(StringTest, GreaterThanOperator)
{
    String str1("Hello");
    String str2("World");
    EXPECT_TRUE(str2 > str1);
    EXPECT_FALSE(str1 > str2);
}

TEST(StringTest, LessThanOrEqualOperator)
{
    String str1("Hello");
    String str2("World");
    String str3("Hello");
    EXPECT_TRUE(str1 <= str2);
    EXPECT_TRUE(str1 <= str3);
    EXPECT_FALSE(str2 <= str1);
}

TEST(StringTest, GreaterThanOrEqualOperator)
{
    String str1("Hello");
    String str2("World");
    String str3("Hello");
    EXPECT_TRUE(str2 >= str1);
    EXPECT_TRUE(str1 >= str3);
    EXPECT_FALSE(str1 >= str2);
}

TEST(StringTest, ConcatenationOperator)
{
    String str1("Hello");
    String str2("World");
    String result = str1 + str2;
    EXPECT_EQ(result.length(), 10);
    EXPECT_STREQ(result.toCharArray(), "HelloWorld");
}

TEST(StringTest, ConcatenationAssignmentOperator)
{
    String str1("Hello");
    String str2("World");
    str1 += str2;
    EXPECT_EQ(str1.length(), 10);
    EXPECT_STREQ(str1.toCharArray(), "HelloWorld");
}

TEST(StringTest, CharConcatenationAssignmentOperator)
{
    String str("Hello");
    str += '!';
    EXPECT_EQ(str.length(), 6);
    EXPECT_STREQ(str.toCharArray(), "Hello!");
}

TEST(StringTest, ToCharArray)
{
    String str("Hello");
    const char *cstr = str.toCharArray();
    EXPECT_STREQ(cstr, "Hello");
}

TEST(StringTest, Length)
{
    String str("Hello");
    EXPECT_EQ(str.length(), 5);
}

TEST(StringTest, Substring)
{
    String str("Hello World");
    String sub1 = str.substring(6);
    String sub2 = str.substring(0, 5);
    EXPECT_EQ(sub1.length(), 5);
    EXPECT_EQ(sub2.length(), 5);
    EXPECT_STREQ(sub1.toCharArray(), "World");
    EXPECT_STREQ(sub2.toCharArray(), "Hello");
}

TEST(StringTest, Find)
{
    String str("Hello World");
    size_t pos1 = str.find("World");
    size_t pos2 = str.find("Universe");
    EXPECT_EQ(pos1, 6);
    EXPECT_EQ(pos2, String::npos);
}

TEST(StringTest, Replace)
{
    String str("Hello World");
    size_t count = str.replace("World", "Universe");
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(str.toCharArray(), "Hello Universe");
}

TEST(StringTest, ToUpperCase)
{
    String str("hello world");
    String upper = str.toUpperCase();
    EXPECT_STREQ(upper.toCharArray(), "HELLO WORLD");
}

TEST(StringTest, ToLowerCase)
{
    String str("HELLO WORLD");
    String lower = str.toLowerCase();
    EXPECT_STREQ(lower.toCharArray(), "hello world");
}

TEST(StringTest, Split)
{
    String str("Hello,World,Universe");
    std::vector<String> tokens = str.split(",");
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_STREQ(tokens[0].toCharArray(), "Hello");
    EXPECT_STREQ(tokens[1].toCharArray(), "World");
    EXPECT_STREQ(tokens[2].toCharArray(), "Universe");
}

TEST(StringTest, Join)
{
    std::vector<String> strings = {"Hello", "World", "Universe"};
    String joined = String::join(strings, ",");
    EXPECT_STREQ(joined.toCharArray(), "Hello,World,Universe");
}

TEST(StringTest, ReplaceAll)
{
    String str("Hello World World");
    size_t count = str.replaceAll("World", "Universe");
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(str.toCharArray(), "Hello Universe Universe");
}

TEST(StringTest, InsertChar)
{
    String str("Hello");
    str.insertChar(5, '!');
    EXPECT_STREQ(str.toCharArray(), "Hello!");
}

TEST(StringTest, DeleteChar)
{
    String str("Hello!");
    str.deleteChar(5);
    EXPECT_STREQ(str.toCharArray(), "Hello");
}

TEST(StringTest, Reverse)
{
    String str("Hello");
    String reversed = str.reverse();
    EXPECT_STREQ(reversed.toCharArray(), "olleH");
}

TEST(StringTest, EqualsIgnoreCase)
{
    String str1("Hello");
    String str2("hello");
    String str3("World");
    EXPECT_TRUE(str1.equalsIgnoreCase(str2));
    EXPECT_FALSE(str1.equalsIgnoreCase(str3));
}

TEST(StringTest, IndexOf)
{
    String str("Hello World");
    size_t pos1 = str.indexOf("World");
    size_t pos2 = str.indexOf("Universe");
    EXPECT_EQ(pos1, 6);
    EXPECT_EQ(pos2, String::npos);
}

TEST(StringTest, Trim)
{
    String str1("  Hello World  ");
    String str2("Hello World");
    str1.trim();
    EXPECT_STREQ(str1.toCharArray(), str2.toCharArray());
}

TEST(StringTest, StartsWith)
{
    String str("Hello World");
    EXPECT_TRUE(str.startsWith("Hello"));
    EXPECT_FALSE(str.startsWith("World"));
}

TEST(StringTest, EndsWith)
{
    String str("Hello World");
    EXPECT_TRUE(str.endsWith("World"));
    EXPECT_FALSE(str.endsWith("Hello"));
}

TEST(StringTest, Escape)
{
    String str("Hello \"World\"");
    String escaped = str.escape();
    EXPECT_STREQ(escaped.toCharArray(), "Hello \\\"World\\\"");
}

TEST(StringTest, Unescape)
{
    String str("Hello \\\"World\\\"");
    String unescaped = str.unescape();
    EXPECT_STREQ(unescaped.toCharArray(), "Hello \"World\"");
}

TEST(StringTest, ToInt)
{
    String str("123");
    int value = str.toInt();
    EXPECT_EQ(value, 123);
}

TEST(StringTest, ToFloat)
{
    String str("3.14");
    float value = str.toFloat();
    EXPECT_FLOAT_EQ(value, 3.14);
}

TEST(StringTest, Format)
{
    String str = String::format("%s %d", "Hello", 2022);
    EXPECT_STREQ(str.toCharArray(), "Hello 2022");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
