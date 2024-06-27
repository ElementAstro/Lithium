#include "atom/type/short_string.hpp"
#include <gtest/gtest.h>

#include <cstring>

TEST(ShortStringTest, DefaultConstructor) {
    atom::type::ShortString s;
    EXPECT_EQ(s.length(), 0);
}

TEST(ShortStringTest, ConstructorFromString) {
    std::string input = "Hello";
    atom::type::ShortString s(input);
    EXPECT_EQ(s.length(), input.length());
    EXPECT_EQ(s, input);
}

TEST(ShortStringTest, ConstructorFromStringView) {
    std::string_view input = "World";
    atom::type::ShortString s(input);
    EXPECT_EQ(s.length(), input.length());
    EXPECT_EQ(s, input);
}

TEST(ShortStringTest, ConstructorFromCString) {
    const char* input = "Short";
    atom::type::ShortString s(input);
    EXPECT_EQ(s.length(), std::strlen(input));
    EXPECT_EQ(s, input);
}

TEST(ShortStringTest, CopyConstructor) {
    std::string input = "String";
    atom::type::ShortString s1(input);
    atom::type::ShortString s2(s1);
    EXPECT_EQ(s1.length(), s2.length());
    EXPECT_EQ(s1, s2);
}

TEST(ShortStringTest, MoveConstructor) {
    std::string input = "Copy";
    atom::type::ShortString s1(input);
    atom::type::ShortString s2(std::move(s1));
    EXPECT_EQ(s1.length(), 0);
    EXPECT_EQ(s2.length(), input.length());
    EXPECT_EQ(s2, input);
}

TEST(ShortStringTest, CopyAssignmentOperator) {
    std::string input = "Assignment";
    atom::type::ShortString s1;
    atom::type::ShortString s2(input);
    s1 = s2;
    EXPECT_EQ(s1.length(), s2.length());
    EXPECT_EQ(s1, s2);
}

TEST(ShortStringTest, MoveAssignmentOperator) {
    std::string input = "Operator";
    atom::type::ShortString s1;
    atom::type::ShortString s2(input);
    s1 = std::move(s2);
    EXPECT_EQ(s1.length(), input.length());
    EXPECT_EQ(s2.length(), 0);
    EXPECT_EQ(s1, input);
}

TEST(ShortStringTest, AssignmentFromString) {
    std::string input = "Test";
    atom::type::ShortString s;
    s = input;
    EXPECT_EQ(s.length(), input.length());
    EXPECT_EQ(s, input);
}

TEST(ShortStringTest, AssignmentFromCString) {
    const char* input = "Unit";
    atom::type::ShortString s;
    s = input;
    EXPECT_EQ(s.length(), std::strlen(input));
    EXPECT_EQ(s, input);
}

TEST(ShortStringTest, AssignmentFromStringView) {
    std::string_view input = "String";
    atom::type::ShortString s;
    s = input;
    EXPECT_EQ(s.length(), input.length());
    EXPECT_EQ(s, input);
}

TEST(ShortStringTest, StreamInsertionOperator) {
    std::string input = "Insertion";
    atom::type::ShortString s(input);
    std::stringstream ss;
    ss << s;
    EXPECT_EQ(ss.str(), input);
}

TEST(ShortStringTest, Concatenation) {
    std::string input1 = "Hello";
    std::string input2 = "World";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    atom::type::ShortString result = s1 + s2;
    EXPECT_EQ(result.length(), input1.length() + input2.length());
    EXPECT_EQ(result, input1 + input2);
}

TEST(ShortStringTest, Append) {
    std::string input1 = "Append";
    std::string input2 = "Test";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    s1 += s2;
    EXPECT_EQ(s1.length(), input1.length() + input2.length());
    EXPECT_EQ(s1, input1 + input2);
}

TEST(ShortStringTest, AppendFromStringView) {
    std::string input1 = "String";
    std::string_view input2 = "View";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    s1 += input2;
    EXPECT_EQ(s1.length(), input1.length() + input2.length());
}

TEST(ShortStringTest, EqualityComparison) {
    std::string input = "Equality";
    atom::type::ShortString s1(input);
    atom::type::ShortString s2(input);
    EXPECT_TRUE(s1 == s2);
}

TEST(ShortStringTest, InequalityComparison) {
    std::string input1 = "Inequality";
    std::string input2 = "Test";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    EXPECT_TRUE(s1 != s2);
}

TEST(ShortStringTest, LessThanComparison) {
    std::string input1 = "Less";
    std::string input2 = "Than";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    EXPECT_TRUE(s1 < s2);
}

TEST(ShortStringTest, GreaterThanComparison) {
    std::string input1 = "Greater";
    std::string input2 = "Than";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    EXPECT_TRUE(s1 > s2);
}

TEST(ShortStringTest, LessThanOrEqualToComparison) {
    std::string input1 = "Less";
    std::string input2 = "Equal";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    EXPECT_TRUE(s1 <= s2);
}

TEST(ShortStringTest, GreaterThanOrEqualToComparison) {
    std::string input1 = "Greater";
    std::string input2 = "Equal";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    EXPECT_TRUE(s1 >= s2);
}

TEST(ShortStringTest, AccessCharacter) {
    std::string input = "Access";
    atom::type::ShortString s(input);
    for (size_t i = 0; i < input.length(); i++) {
        EXPECT_EQ(s[i], input[i]);
    }
}

TEST(ShortStringTest, AccessCharacterConst) {
    std::string input = "Character";
    const atom::type::ShortString s(input);
    for (size_t i = 0; i < input.length(); i++) {
        EXPECT_EQ(s[i], input[i]);
    }
}

TEST(ShortStringTest, Length) {
    std::string input = "Length";
    atom::type::ShortString s(input);
    EXPECT_EQ(s.length(), input.length());
}

TEST(ShortStringTest, Substr) {
    std::string input = "Substring";
    atom::type::ShortString s(input);
    size_t pos = 2;
    size_t count = 4;
    atom::type::ShortString result = s.substr(pos, count);
    EXPECT_EQ(result.length(), count);
    EXPECT_EQ(result, input.substr(pos, count));
}

TEST(ShortStringTest, Clear) {
    std::string input = "Clear";
    atom::type::ShortString s(input);
    s.clear();
    EXPECT_EQ(s.length(), 0);
}

TEST(ShortStringTest, Swap) {
    std::string input1 = "Swap1";
    std::string input2 = "Swap2";
    atom::type::ShortString s1(input1);
    atom::type::ShortString s2(input2);
    s1.swap(s2);
    EXPECT_EQ(s1.length(), input2.length());
    EXPECT_EQ(s2.length(), input1.length());
    EXPECT_EQ(s1, input2);
    EXPECT_EQ(s2, input1);
}