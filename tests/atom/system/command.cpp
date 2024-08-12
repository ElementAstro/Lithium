#include <gtest/gtest.h>

#include "atom/system/command.hpp"

#include <future>
#include <thread>

namespace atom::system {
// Mocking helper functions for testing if required
bool _CreateProcessAsUser(const std::string &, const std::string &,
                          const std::string &, const std::string &) {
    return true;
}
}  // namespace atom::system

using namespace atom::system;

// Helper function to create a dummy process for testing
void createDummyProcess() {
#ifdef _WIN32
    system("start /B ping -n 10 127.0.0.1 > NUL");
#else
    system("sleep 10 &");
#endif
}

// Test fixture for setting up common test environment
class CommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test executeCommandWithStatus function
TEST_F(CommandTest, ExecuteCommandWithStatus) {
    auto [output, status] = executeCommandWithStatus("echo Hello World");
    EXPECT_EQ(status, 0);
    EXPECT_EQ(output, "Hello World\n");
}

// Test executeCommands function
TEST_F(CommandTest, ExecuteCommands) {
    std::vector<std::string> commands = {"echo Hello", "echo World"};
    EXPECT_NO_THROW(executeCommands(commands));
}

// Test executeCommandWithEnv function
TEST_F(CommandTest, ExecuteCommandWithEnv) {
    std::unordered_map<std::string, std::string> envVars = {
        {"TEST_ENV_VAR", "12345"}};
    auto result = executeCommandWithEnv("echo $TEST_ENV_VAR", envVars);
    EXPECT_EQ(result, "12345\n");
}

// Test killProcessByName function
TEST_F(CommandTest, KillProcessByName) {
    createDummyProcess();
    EXPECT_NO_THROW(killProcessByName("sleep", SIGTERM));
}

// Test executeCommandStream function with termination condition
TEST_F(CommandTest, ExecuteCommandStream) {
    int status = 0;
    std::function<bool()> terminateCondition = []() { return true; };
    auto result = executeCommandStream("sleep 5", false, nullptr, status,
                                       terminateCondition);
    EXPECT_EQ(result, "");
}
