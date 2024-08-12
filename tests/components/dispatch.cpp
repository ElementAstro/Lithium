#include "atom/components/dispatch.hpp"
#include <gtest/gtest.h>
#include "exception.hpp"
#include "function/type_caster.hpp"

// Test fixture for CommandDispatcher tests
class CommandDispatcherTest : public ::testing::Test {
protected:
    std::shared_ptr<atom::meta::TypeCaster> typeCaster =
        std::make_shared<atom::meta::TypeCaster>();
    CommandDispatcher dispatcher{typeCaster};

    void SetUp() override {
        // Set up code, if any, for each test
    }

    void TearDown() override {
        // Cleanup code, if any, for each test
    }
};

// Test the `def` method with a simple function
TEST_F(CommandDispatcherTest, DefineAndDispatchSimpleFunction) {
    dispatcher.def(
        "add", "math", "Adds two numbers",
        std::function<int(int, int)>([](int a, int b) { return a + b; }));

    std::any result = dispatcher.dispatch("add", 3, 4);
    ASSERT_EQ(std::any_cast<int>(result), 7);
}

// Test the `defT` method with a function that times out
TEST_F(CommandDispatcherTest, DefineAndDispatchTimeoutFunction) {
    dispatcher.defT("sleepy", "test", "Sleeps for a while",
                    std::function<void()>([]() {
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                    }));
    dispatcher.setTimeout("sleepy", std::chrono::milliseconds(500));

    ASSERT_THROW(dispatcher.dispatch("sleepy"), DispatchTimeout);
}

// Test dispatching with missing arguments and default values
TEST_F(CommandDispatcherTest, DispatchWithDefaultArguments) {
    dispatcher.def("increment", "math", "Increments a number",
                   std::function<int(int)>([](int a) { return a + 1; }),
                   std::nullopt, std::nullopt, {Arg("a", 42)});

    std::any result = dispatcher.dispatch("increment");
    ASSERT_EQ(std::any_cast<int>(result), 43);
}

// Test dispatching a command with precondition failure
TEST_F(CommandDispatcherTest, DispatchWithPreconditionFailure) {
    dispatcher.def(
        "alwaysFail", "test", "This should always fail",
        std::function<void()>([]() {}),
        std::optional<std::function<bool()>>([]() { return false; }));

    ASSERT_THROW(dispatcher.dispatch("alwaysFail"), DispatchException);
}

// Test handling of invalid command dispatches
TEST_F(CommandDispatcherTest, DispatchInvalidCommand) {
    ASSERT_THROW(dispatcher.dispatch("nonexistent"),
                 atom::error::InvalidArgument);
}

// Test alias creation and resolution
TEST_F(CommandDispatcherTest, AliasCreationAndResolution) {
    dispatcher.def(
        "hello", "greetings", "Returns a greeting",
        std::function<std::string()>([]() { return "Hello, world!"; }));
    dispatcher.addAlias("hello", "hi");

    std::any result = dispatcher.dispatch("hi");
    ASSERT_EQ(std::any_cast<std::string>(result), "Hello, world!");
}

// Test group management and command listing
TEST_F(CommandDispatcherTest, GroupManagementAndCommandListing) {
    dispatcher.def("cmd1", "group1", "Command 1",
                   std::function<void()>([]() {}));
    dispatcher.def("cmd2", "group1", "Command 2",
                   std::function<void()>([]() {}));
    dispatcher.def("cmd3", "group2", "Command 3",
                   std::function<void()>([]() {}));

    std::vector<std::string> group1Commands =
        dispatcher.getCommandsInGroup("group1");
    // TODO: FIX ME
    // Max: This is not working as expected.
    std::vector<std::string> expected{"cmd2", "cmd1"};
    ASSERT_EQ(group1Commands, expected);

    std::vector<std::string> allCommands = dispatcher.getAllCommands();

    std::vector<std::string> allExpected{"cmd1", "cmd2", "cmd3"};

    bool allFound = true;
    for (const auto& cmd : allExpected) {
        if (std::find(allCommands.begin(), allCommands.end(), cmd) ==
            allCommands.end()) {
            allFound = false;
            break;
        }
    }
    ASSERT_EQ(allFound, true);
}

// Test removing a command
TEST_F(CommandDispatcherTest, RemoveCommand) {
    dispatcher.def("toRemove", "misc", "A command to be removed",
                   std::function<void()>([]() {}));
    ASSERT_TRUE(dispatcher.has("toRemove"));

    dispatcher.removeCommand("toRemove");
    ASSERT_FALSE(dispatcher.has("toRemove"));
}

// Test dispatching a command with mismatched argument types
/*
TEST_F(CommandDispatcherTest, DispatchWithMismatchedArgumentTypes) {
    dispatcher.def(
        "addInts", "math", "Adds two integers",
        std::function<int(int, int)>([](int a, int b) { return a + b; }));

    ASSERT_THROW(
        dispatcher.dispatch("addInts", std::string("3"), std::string("4")),
        std::invalid_argument);
}
*/

// Test dispatching an overloaded function
TEST_F(CommandDispatcherTest, DispatchOverloadedFunction) {
    dispatcher.def("overloaded", "test", "Overloaded function",
                   std::function<int(int)>([](int a) { return a; }));
    dispatcher.def("overloaded", "test", "Overloaded function",
                   std::function<std::string(std::string)>(
                       [](std::string a) { return a; }));

    std::any intResult = dispatcher.dispatch("overloaded", 42);
    ASSERT_EQ(std::any_cast<int>(intResult), 42);

    std::any stringResult =
        dispatcher.dispatch("overloaded", std::string("test"));
    ASSERT_EQ(std::any_cast<std::string>(stringResult), "test");
}
