#include <gtest/gtest.h>
#include "../../src/atom/server/commander.hpp"

class CommandDispatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // set up test environment
    }

    void TearDown() override {
        // clean up test environment
    }

    // define helper functions for testing
};

// Test case for RegisterHandler function
TEST_F(CommandDispatcherTest, RegisterHandlerTest)
{
    std::string name = "test";
    int data = 5;
    // Register a handler
    dispatcher1.RegisterHandler(name, [](const int &data)
                                { return data * 2; });

    // Check if the handler is registered
    auto it = dispatcher1.handlers_.find(dispatcher1.Djb2Hash(name.c_str()));
    ASSERT_NE(it, dispatcher1.handlers_.end());
    ASSERT_EQ(it->second(data), 10);
}

// Test case for RegisterMemberHandler function
TEST_F(CommandDispatcherTest, RegisterMemberHandlerTest)
{
    std::string name = "test";
    int data = 5;
    // Create a test object and register a member function handler
    class TestObject
    {
    public:
        int memberFunc(const int &data) { return data * 3; }
    } testObj;
    dispatcher1.RegisterMemberHandler(name, &testObj, &TestObject::memberFunc);

    // Check if the member handler is registered
    it = dispatcher1.handlers_.find(dispatcher1.Djb2Hash(name.c_str()));
    ASSERT_NE(it, dispatcher1.handlers_.end());
    ASSERT_EQ(it->second(data), 15);
}

// Test case for HasHandler function
TEST_F(CommandDispatcherTest, HasHandlerTest)
{
    std::string name = "test";
    // Register a handler
    dispatcher1.RegisterHandler(name, [](const int &data)
                                { return data * 2; });

    // Check if the handler exists
    ASSERT_TRUE(dispatcher1.HasHandler(name));
}

// Test case for Dispatch function
TEST_F(CommandDispatcherTest, DispatchTest)
{
    std::string name = "test";
    int data = 5;
    // Register a handler
    dispatcher1.RegisterHandler(name, [](const int &data)
                                { return data * 2; });

    // Execute the dispatch
    int result = dispatcher1.Dispatch(name, data);

    // Check the result
    ASSERT_EQ(result, 10);
}

// Test case for Undo function
TEST_F(CommandDispatcherTest, UndoTest)
{
    std::string name = "test";
    int data = 5;
    // Register a handler and execute a command
    dispatcher1.RegisterHandler(name, [](const int &data)
                                { return data * 2; });
    dispatcher1.Dispatch(name, data);

    // Check if the undo is successful
    ASSERT_TRUE(dispatcher1.Undo());
}

// Test case for Redo function
TEST_F(CommandDispatcherTest, RedoTest)
{
    std::string name = "test";
    int data = 5;
    // Register a handler and execute a command
    dispatcher1.RegisterHandler(name, [](const int &data)
                                { return data * 2; });
    dispatcher1.Dispatch(name, data);
    ASSERT_TRUE(dispatcher1.Undo());

    // Check if the redo is successful
    ASSERT_TRUE(dispatcher1.Redo());
}