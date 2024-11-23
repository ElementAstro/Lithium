#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "task/simple/task.hpp"

using namespace lithium::sequencer;
using json = nlohmann::json;

// Mock action function
void mockAction(const json& params) {
    // Mock implementation
}

// Test fixture for Task
class TaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        task = std::make_unique<Task>("TestTask", mockAction);
    }

    std::unique_ptr<Task> task;
};

// Test Task construction
TEST_F(TaskTest, TaskConstruction) {
    EXPECT_EQ(task->getName(), "TestTask");
    EXPECT_EQ(task->getStatus(), TaskStatus::Pending);
}

// Test Task execution
TEST_F(TaskTest, TaskExecution) {
    json params = {{"key", "value"}};
    task->execute(params);
    EXPECT_EQ(task->getStatus(), TaskStatus::InProgress);
}

// Test Task timeout setting
TEST_F(TaskTest, TaskTimeoutSetting) {
    std::chrono::seconds timeout(5);
    task->setTimeout(timeout);
    // Assuming there's a way to verify the timeout, which is not shown in the
    // provided code
}

// Test Task getters
TEST_F(TaskTest, TaskGetters) {
    EXPECT_EQ(task->getName(), "TestTask");
    EXPECT_EQ(task->getStatus(), TaskStatus::Pending);
    EXPECT_EQ(task->getError(), std::nullopt);
}

// Test Task UUID getter
TEST_F(TaskTest, TaskUUIDGetter) {
    // Assuming UUID is generated in the constructor, which is not shown in the
    // provided code
    EXPECT_FALSE(task->getUUID().empty());
}

// Test Task error handling
TEST_F(TaskTest, TaskErrorHandling) {
    // Simulate an error
    task->execute(json{});
    // Assuming there's a way to set the error, which is not shown in the
    // provided code
    EXPECT_EQ(task->getStatus(), TaskStatus::Failed);
    EXPECT_TRUE(task->getError().has_value());
}
