#include "task/task.hpp"
#include <gtest/gtest.h>


using json = nlohmann::json;

// Test fixture for Task
class TaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here
    }

    void TearDown() override {
        // Cleanup code here
    }
};

// Test case to check Task creation
TEST_F(TaskTest, TaskCreation) {
    json params = {{"key", "value"}};
    Task task("TestTask", params, [](const json& p) { return p; });

    EXPECT_EQ(task.getName(), "TestTask");
    EXPECT_EQ(task.getParams(), params);
    EXPECT_EQ(task.getStatus(), Task::Status::Pending);
}

// Test case to check Task execution success
TEST_F(TaskTest, TaskExecutionSuccess) {
    json params = {{"key", "value"}};
    Task task("TestTask", params, [](const json& p) { return p; });

    task.run();

    EXPECT_EQ(task.getStatus(), Task::Status::Completed);
    ASSERT_TRUE(task.getResult().has_value());
    EXPECT_EQ(task.getResult().value(), params);
}

// Test case to check Task execution failure
TEST_F(TaskTest, TaskExecutionFailure) {
    json params = {{"key", "value"}};
    bool onTerminateCalled = false;
    Task task(
        "TestTask", params,
        [](const json&) -> json { throw std::runtime_error("error"); },
        [&onTerminateCalled](const std::exception&) {
            onTerminateCalled = true;
        });

    task.run();

    EXPECT_EQ(task.getStatus(), Task::Status::Failed);
    EXPECT_FALSE(task.getResult().has_value());
    EXPECT_TRUE(onTerminateCalled);
}

// Test case to check Task execution without onTerminate handler
TEST_F(TaskTest, TaskExecutionFailureWithoutOnTerminate) {
    json params = {{"key", "value"}};
    Task task("TestTask", params,
              [](const json&) -> json { throw std::runtime_error("error"); });

    task.run();

    EXPECT_EQ(task.getStatus(), Task::Status::Failed);
    EXPECT_FALSE(task.getResult().has_value());
}