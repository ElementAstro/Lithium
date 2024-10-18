#include "atom/type/string.hpp"
#include <gtest/gtest.h>

// Test default constructor
TEST(StringTest, DefaultConstructor) {
    String s;
    EXPECT_TRUE(s.empty());
}

// Test C-string constructor
TEST(StringTest, CStringConstructor) {
    String s("Hello");
    EXPECT_EQ(s.cStr(), std::string("Hello"));
}

// Test std::string constructor
TEST(StringTest, StdStringConstructor) {
    std::string stdStr = "Hello";
    String s(stdStr);
    EXPECT_EQ(s.cStr(), stdStr);
}

// Test std::string_view constructor
TEST(StringTest, StringViewConstructor) {
    std::string_view stdView = "Hello";
    String s(stdView);
    EXPECT_EQ(s.cStr(), std::string(stdView));
}

// Test copy constructor
TEST(StringTest, CopyConstructor) {
    String s1("Hello");
    String s2(s1);
    EXPECT_EQ(s2.cStr(), std::string("Hello"));
}

// Test move constructor
TEST(StringTest, MoveConstructor) {
    String s1("Hello");
    String s2(std::move(s1));
    EXPECT_EQ(s2.cStr(), std::string("Hello"));
    EXPECT_TRUE(s1.empty());
}

// Test copy assignment
TEST(StringTest, CopyAssignment) {
    String s1("Hello");
    String s2;
    s2 = s1;
    EXPECT_EQ(s2.cStr(), std::string("Hello"));
}

// Test move assignment
TEST(StringTest, MoveAssignment) {
    String s1("Hello");
    String s2;
    s2 = std::move(s1);
    EXPECT_EQ(s2.cStr(), std::string("Hello"));
    EXPECT_TRUE(s1.empty());
}

// Test equality operator
TEST(StringTest, EqualityOperator) {
    String s1("Hello");
    String s2("Hello");
    String s3("World");
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);
}

// Test inequality operator
TEST(StringTest, InequalityOperator) {
    String s1("Hello");
    String s2("Hello");
    String s3("World");
    EXPECT_FALSE(s1 != s2);
    EXPECT_TRUE(s1 != s3);
}

// Test empty function
TEST(StringTest, Empty) {
    String s1;
    String s2("Hello");
    EXPECT_TRUE(s1.empty());
    EXPECT_FALSE(s2.empty());
}

// Test comparison operators
TEST(StringTest, ComparisonOperators) {
    String s1("Apple");
    String s2("Banana");
    EXPECT_TRUE(s1 < s2);
    EXPECT_FALSE(s1 > s2);
    EXPECT_TRUE(s1 <= s2);
    EXPECT_FALSE(s1 >= s2);
}

// Test concatenation operators
TEST(StringTest, ConcatenationOperators) {
    String s1("Hello");
    String s2("World");
    s1 += s2;
    EXPECT_EQ(s1.cStr(), std::string("HelloWorld"));

    s1 += "!";
    EXPECT_EQ(s1.cStr(), std::string("HelloWorld!"));

    s1 += '!';
    EXPECT_EQ(s1.cStr(), std::string("HelloWorld!!"));
}

// Test cStr function
TEST(StringTest, CStr) {
    String s("Hello");
    EXPECT_EQ(std::string(s.cStr()), "Hello");
}

// Test length function
TEST(StringTest, Length) {
    String s("Hello");
    EXPECT_EQ(s.length(), 5);
}

// Test substr function
TEST(StringTest, Substr) {
    String s("HelloWorld");
    String sub = s.substr(5, 5);
    EXPECT_EQ(sub.cStr(), "World");
}

// Test find function
TEST(StringTest, Find) {
    String s("HelloWorld");
    EXPECT_EQ(s.find("World"), 5);
    EXPECT_EQ(s.find("Hello"), 0);
    EXPECT_EQ(s.find("NotFound"), String::NPOS);
}

// Test replace function
TEST(StringTest, Replace) {
    String s("HelloWorld");
    EXPECT_TRUE(s.replace("World", "Everyone"));
    EXPECT_EQ(s.cStr(), "HelloEveryone");
    EXPECT_FALSE(s.replace("NotFound", "Everyone"));
}

// Test replaceAll function
TEST(StringTest, ReplaceAll) {
    String s("HelloHelloHello");
    EXPECT_EQ(s.replaceAll("Hello", "Hi"), 3);
    EXPECT_EQ(s.cStr(), "HiHiHi");
}

// Test toUpper function
TEST(StringTest, ToUpper) {
    String s("HelloWorld");
    String upper = s.toUpper();
    EXPECT_EQ(upper.cStr(), "HELLOWORLD");
}

// Test toLower function
TEST(StringTest, ToLower) {
    String s("HelloWorld");
    String lower = s.toLower();
    EXPECT_EQ(lower.cStr(), "helloworld");
}

// Test split function
TEST(StringTest, Split) {
    String s("one,two,three");
    auto parts = s.split(",");
    ASSERT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0].cStr(), "one");
    EXPECT_EQ(parts[1].cStr(), "two");
    EXPECT_EQ(parts[2].cStr(), "three");
}

// Test join function
TEST(StringTest, Join) {
    std::vector<String> parts = {"one", "two", "three"};
    String joined = String::join(parts, ",");
    EXPECT_EQ(joined.cStr(), "one,two,three");
}

// Test insert function
TEST(StringTest, Insert) {
    String s("HelloWorld");
    s.insert(5, ' ');
    EXPECT_EQ(s.cStr(), "Hello World");
}

// Test erase function
TEST(StringTest, Erase) {
    String s("HelloWorld");
    s.erase(5, 5);
    EXPECT_EQ(s.cStr(), "Hello");
}

// Test reverse function
TEST(StringTest, Reverse) {
    String s("Hello");
    String reversed = s.reverse();
    EXPECT_EQ(reversed.cStr(), "olleH");
}

// Test equalsIgnoreCase function
TEST(StringTest, EqualsIgnoreCase) {
    String s1("Hello");
    String s2("hello");
    EXPECT_TRUE(s1.equalsIgnoreCase(s2));
}

// Test startsWith function
TEST(StringTest, StartsWith) {
    String s("HelloWorld");
    EXPECT_TRUE(s.startsWith("Hello"));
    EXPECT_FALSE(s.startsWith("World"));
}

// Test endsWith function
TEST(StringTest, EndsWith) {
    String s("HelloWorld");
    EXPECT_TRUE(s.endsWith("World"));
    EXPECT_FALSE(s.endsWith("Hello"));
}

// Test trim functions
TEST(StringTest, Trim) {
    String s("   HelloWorld   ");
    s.trim();
    EXPECT_EQ(s.cStr(), "HelloWorld");

    s = "   HelloWorld   ";
    s.ltrim();
    EXPECT_EQ(s.cStr(), "HelloWorld   ");

    s = "   HelloWorld   ";
    s.rtrim();
    EXPECT_EQ(s.cStr(), "   HelloWorld");
}

// Test format function
TEST(StringTest, Format) {
    String s = String::format("Hello {}, {}", "World", 2024);
    EXPECT_EQ(s.cStr(), "Hello World, 2024");
}
