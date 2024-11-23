#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "task/simple/target.hpp"

using namespace lithium::sequencer;
using json = nlohmann::json;

class TargetTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(TargetTest, TargetInitialization) {
    Target target("TestTarget");

    EXPECT_EQ(target.getName(), "TestTarget");
    EXPECT_EQ(target.getStatus(), TargetStatus::Pending);
    EXPECT_TRUE(target.isEnabled());
    EXPECT_EQ(target.getProgress(), 0.0);
}

TEST_F(TargetTest, AddTask) {
    Target target("TestTarget");
    auto action = []([[maybe_unused]] const json& params) {};
    auto task = std::make_unique<Task>("TestTask", action);

    target.addTask(std::move(task));

    EXPECT_EQ(target.getProgress(), 0.0);
}

TEST_F(TargetTest, SetCooldown) {
    Target target("TestTarget");
    target.setCooldown(std::chrono::seconds(5));

    // No direct way to verify cooldown, but we can ensure no exceptions are
    // thrown
    SUCCEED();
}

TEST_F(TargetTest, SetEnabled) {
    Target target("TestTarget");
    target.setEnabled(false);

    EXPECT_FALSE(target.isEnabled());
}

TEST_F(TargetTest, SetMaxRetries) {
    Target target("TestTarget");
    target.setMaxRetries(3);

    // No direct way to verify maxRetries, but we can ensure no exceptions are
    // thrown
    SUCCEED();
}

TEST_F(TargetTest, SetStatus) {
    Target target("TestTarget");
    target.setStatus(TargetStatus::InProgress);

    EXPECT_EQ(target.getStatus(), TargetStatus::InProgress);
}

TEST_F(TargetTest, SetOnStartCallback) {
    Target target("TestTarget");
    bool callbackCalled = false;
    target.setOnStart(
        [&callbackCalled](const std::string&) { callbackCalled = true; });

    target.execute();

    EXPECT_TRUE(callbackCalled);
}

TEST_F(TargetTest, SetOnEndCallback) {
    Target target("TestTarget");
    bool callbackCalled = false;
    target.setOnEnd([&callbackCalled](const std::string&, TargetStatus) {
        callbackCalled = true;
    });

    target.execute();

    EXPECT_TRUE(callbackCalled);
}

TEST_F(TargetTest, SetOnErrorCallback) {
    Target target("TestTarget");
    bool callbackCalled = false;
    target.setOnError(
        [&callbackCalled](const std::string&, const std::exception&) {
            callbackCalled = true;
        });

    auto action = [](const json&) { throw std::runtime_error("Task failed"); };
    auto task = std::make_unique<Task>("TestTask", action);
    target.addTask(std::move(task));

    EXPECT_THROW(target.execute(), std::runtime_error);
    EXPECT_TRUE(callbackCalled);
}

TEST_F(TargetTest, ExecuteTarget) {
    Target target("TestTarget");
    bool taskExecuted = false;
    auto action = [&taskExecuted](const json&) { taskExecuted = true; };
    auto task = std::make_unique<Task>("TestTask", action);
    target.addTask(std::move(task));

    target.execute();

    EXPECT_TRUE(taskExecuted);
    EXPECT_EQ(target.getStatus(), TargetStatus::Completed);
}

TEST_F(TargetTest, LoadTasksFromJson) {
    Target target("TestTarget");
    json tasksJson = R"([
        {"name": "Task1", "action": "action1"},
        {"name": "Task2", "action": "action2"}
    ])"_json;

    target.loadTasksFromJson(tasksJson);

    EXPECT_EQ(target.getProgress(), 0.0);
}
