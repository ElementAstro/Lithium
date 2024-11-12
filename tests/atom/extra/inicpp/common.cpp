#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "atom/extra/inicpp/common.hpp"

using namespace inicpp;

// Test whitespaces function
TEST(CommonTest, Whitespaces) {
    std::string_view ws = whitespaces();
    EXPECT_EQ(ws, " \t\n\r\f\v");
}

// Test indents function
TEST(CommonTest, Indents) {
    std::string_view ind = indents();
    EXPECT_EQ(ind, " \t");
}

// Test trim function
TEST(CommonTest, Trim) {
    std::string str = "   Hello, World!   ";
    trim(str);
    EXPECT_EQ(str, "Hello, World!");

    str = "NoLeadingOrTrailingSpaces";
    trim(str);
    EXPECT_EQ(str, "NoLeadingOrTrailingSpaces");

    str = "   ";
    trim(str);
    EXPECT_EQ(str, "");
}

// Test strToLong function
TEST(CommonTest, StrToLong) {
    std::optional<long> result = strToLong("12345");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 12345);

    result = strToLong("abc");
    EXPECT_FALSE(result.has_value());

    result = strToLong("");
    EXPECT_FALSE(result.has_value());
}

// Test strToULong function
TEST(CommonTest, StrToULong) {
    std::optional<unsigned long> result = strToULong("12345");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 12345);

    result = strToULong("abc");
    EXPECT_FALSE(result.has_value());

    result = strToULong("");
    EXPECT_FALSE(result.has_value());
}

// Test StringInsensitiveLess struct
TEST(CommonTest, StringInsensitiveLess) {
    StringInsensitiveLess cmp;

    EXPECT_TRUE(cmp("apple", "Banana"));
    EXPECT_FALSE(cmp("Banana", "apple"));
    EXPECT_FALSE(cmp("apple", "apple"));
    EXPECT_TRUE(cmp("apple", "APPLE"));
    EXPECT_FALSE(cmp("APPLE", "apple"));
}
