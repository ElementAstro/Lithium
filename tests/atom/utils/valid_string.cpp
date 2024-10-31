#include <gtest/gtest.h>

#include "atom/utils/valid_string.hpp"

namespace atom::utils::test {

TEST(ValidStringTest, IsValidBracket_Valid) {
    std::string validStr = "({[]})";
    ValidationResult result = isValidBracket(validStr);
    ASSERT_TRUE(result.isValid);
    ASSERT_TRUE(result.invalidBrackets.empty());
    ASSERT_TRUE(result.errorMessages.empty());
}

TEST(ValidStringTest, IsValidBracket_Invalid) {
    std::string invalidStr = "({[})";
    ValidationResult result = isValidBracket(invalidStr);
    ASSERT_FALSE(result.isValid);
    ASSERT_FALSE(result.invalidBrackets.empty());
    ASSERT_FALSE(result.errorMessages.empty());
}

TEST(ValidStringTest, BracketValidator_Valid) {
    constexpr auto result = validateBrackets("({[]})");
    ASSERT_TRUE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 0);
}

TEST(ValidStringTest, BracketValidator_Invalid) {
    constexpr auto result = validateBrackets("({[})");
    ASSERT_FALSE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 1);
}

TEST(ValidStringTest, BracketValidator_Quotes) {
    constexpr auto result = validateBrackets("('')");
    ASSERT_TRUE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 0);
}

TEST(ValidStringTest, BracketValidator_EscapedQuotes) {
    constexpr auto result = validateBrackets(R"(\')");
    ASSERT_TRUE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 0);
}

TEST(ValidStringTest, BracketValidator_UnmatchedQuotes) {
    constexpr auto result = validateBrackets("('");
    ASSERT_FALSE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 1);
}

TEST(ValidStringTest, ToArray) {
    constexpr auto arr = toArray("test");
    ASSERT_EQ(arr.size(), 5);  // Including null terminator
    ASSERT_EQ(arr[0], 't');
    ASSERT_EQ(arr[1], 'e');
    ASSERT_EQ(arr[2], 's');
    ASSERT_EQ(arr[3], 't');
    ASSERT_EQ(arr[4], '\0');
}

TEST(ValidStringTest, ValidateBrackets_Valid) {
    constexpr auto result = validateBrackets("({[]})");
    ASSERT_TRUE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 0);
}

TEST(ValidStringTest, ValidateBrackets_Invalid) {
    constexpr auto result = validateBrackets("({[})");
    ASSERT_FALSE(result.isValid());
    ASSERT_EQ(result.getErrorCount(), 1);
}

}  // namespace atom::utils::test
