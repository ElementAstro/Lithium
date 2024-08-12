#include "debug/terminal.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "debug/check.hpp"
#include "debug/suggestion.hpp"

using namespace lithium::debug;
using ::testing::_;
using ::testing::Return;

class MockSuggestionEngine : public SuggestionEngine {
public:
    MOCK_METHOD(std::vector<std::string>, suggest, (const std::string&));
};

class MockCommandChecker : public CommandChecker {
public:
    MOCK_METHOD(bool, checkCommand, (const std::string&));
};

class ConsoleTerminalTest : public ::testing::Test {
protected:
    void SetUp() override {
        suggestionEngine_ = std::make_shared<MockSuggestionEngine>();
        commandChecker_ = std::make_shared<MockCommandChecker>();
        terminal_ = std::make_unique<ConsoleTerminal>();
    }

    std::unique_ptr<ConsoleTerminal> terminal_;
    std::shared_ptr<MockSuggestionEngine> suggestionEngine_;
    std::shared_ptr<MockCommandChecker> commandChecker_;
};

TEST_F(ConsoleTerminalTest, TestGetRegisteredCommands) {
    std::vector<std::string> expectedCommands = {"help", "list_component",
                                                 "show_component_info"};
    EXPECT_CALL(*commandChecker_, checkCommand(_)).WillRepeatedly(Return(true));

    auto commands = terminal_->getRegisteredCommands();
    EXPECT_EQ(commands, expectedCommands);
}

TEST_F(ConsoleTerminalTest, TestCallCommand_ValidCommand) {
    std::vector<std::any> args = {"arg1", 42};
    EXPECT_CALL(*commandChecker_, checkCommand("help")).WillOnce(Return(true));
    EXPECT_CALL(*suggestionEngine_, suggest("help")).Times(0);

    terminal_->callCommand("help", args);
    // 验证命令执行的副作用或输出
}

TEST_F(ConsoleTerminalTest, TestCallCommand_InvalidCommand) {
    std::vector<std::any> args;
    EXPECT_CALL(*commandChecker_, checkCommand("invalid"))
        .WillOnce(Return(false));
    EXPECT_CALL(*suggestionEngine_, suggest("invalid"))
        .WillOnce(Return(std::vector<std::string>{"help"}));

    testing::internal::CaptureStdout();
    terminal_->callCommand("invalid", args);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, ::testing::HasSubstr("Command 'invalid' not found."));
    EXPECT_THAT(output, ::testing::HasSubstr("Did you mean:"));
    EXPECT_THAT(output, ::testing::HasSubstr("- help"));
}
