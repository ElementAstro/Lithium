#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "debug/check.hpp"

#include "atom/type/json.hpp"

using namespace lithium::debug;
using namespace testing;

class CommandCheckerTest : public Test {
protected:
    CommandChecker checker;

    void SetUp() override {
        // Set up any required initialization before each test
        checker.setMaxLineLength(80);
        checker.setDangerousCommands({"rm", "mkfs", "dd", "format"});
    }
};

TEST_F(CommandCheckerTest, DefaultRulesInitialization) {
    // Check if default rules are initialized properly
    auto errors = checker.check("rm -rf /");
    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].message, "Dangerous command detected: rm");
}

TEST_F(CommandCheckerTest, CustomRule) {
    // Add a custom rule and verify it works
    checker.addRule(
        "test_rule",
        [](const std::string& line,
           size_t lineNumber) -> std::optional<CommandChecker::Error> {
            if (line.find("test") != std::string::npos) {
                return CommandChecker::Error{
                    "Test rule triggered", lineNumber, 0,
                    CommandChecker::ErrorSeverity::WARNING};
            }
            return std::nullopt;
        });

    auto errors = checker.check("this is a test command");
    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].message, "Test rule triggered");
    EXPECT_EQ(errors[0].severity, CommandChecker::ErrorSeverity::WARNING);
}

TEST_F(CommandCheckerTest, LineLengthCheck) {
    // Check if line length rule works
    std::string longCommand(100, 'a');
    auto errors = checker.check(longCommand);
    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].message, "Line exceeds maximum length");
}

TEST_F(CommandCheckerTest, UnmatchedQuotesCheck) {
    // Check if unmatched quotes are detected
    auto errors = checker.check("echo \"unmatched quote");
    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].message, "Unmatched quotes detected");
}

TEST_F(CommandCheckerTest, BacktickUsageCheck) {
    // Check if backtick usage is detected
    auto errors = checker.check("echo `uname -a`");
    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].message,
              "Use of backticks detected, consider using $() instead");
}

TEST_F(CommandCheckerTest, ToJsonConversion) {
    // Check if errors are correctly converted to JSON
    auto errors = checker.check("rm -rf /");
    auto jsonErrors = checker.toJson(errors);

    ASSERT_EQ(jsonErrors.size(), 1);
    EXPECT_EQ(jsonErrors[0]["message"], "Dangerous command detected: rm");
    EXPECT_EQ(jsonErrors[0]["severity"], "error");
}

TEST_F(CommandCheckerTest, PrintErrorsWithColor) {
    // Check if errors are correctly printed with color
    auto errors = checker.check("rm -rf /");
    testing::internal::CaptureStdout();
    printErrors(errors, "rm -rf /", true);
    std::string output = testing::internal::GetCapturedStdout();

    // Check if color codes are present
    EXPECT_NE(output.find("\033[31m"), std::string::npos);
    EXPECT_NE(output.find("error: Dangerous command detected: rm"),
              std::string::npos);
}

TEST_F(CommandCheckerTest, PrintErrorsWithoutColor) {
    // Check if errors are correctly printed without color
    auto errors = checker.check("rm -rf /");
    testing::internal::CaptureStdout();
    printErrors(errors, "rm -rf /", false);
    std::string output = testing::internal::GetCapturedStdout();

    // Check if color codes are absent
    EXPECT_EQ(output.find("\033[31m"), std::string::npos);
    EXPECT_NE(output.find("error: Dangerous command detected: rm"),
              std::string::npos);
}
