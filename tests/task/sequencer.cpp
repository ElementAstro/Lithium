#include "task/sequencer.hpp"
#include "task/task.hpp"

#include <thread>
#include <gtest/gtest.h>

using json = nlohmann::json;

// Test fixture class for setting up common test environment
class TaskSequencerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called before each test is executed
    }

    void TearDown() override {
        // Code here will be called after each test is executed
    }
};

// Test case for Task class
TEST_F(TaskSequencerTest, TaskExecutionSuccess) {
    json params = {{"input", 10}};

    Task task("Success Task", params, [](const json& p) -> json {
        int input = p.at("input");
        return json{{"result", input * input}};
    });

    task.run();

    json result = task.getResult().value();
    EXPECT_EQ(result["result"], 100);
}

TEST_F(TaskSequencerTest, TaskExecutionFailureWithTerminationHandler) {
    json params = {{"input", -10}};
    bool exceptionHandled = false;

    Task task(
        "Failure Task", params,
        [](const json& p) -> json {
            int input = p.at("input");
            if (input < 0) {
                throw std::invalid_argument("Input cannot be negative");
            }
            return json{{"result", input * input}};
        },
        [&](const std::exception& ex) {
            EXPECT_STREQ(ex.what(), "Input cannot be negative");
            exceptionHandled = true;
        });

    task.run();

    EXPECT_TRUE(exceptionHandled);
}

TEST_F(TaskSequencerTest, TaskExecutionFailureWithoutTerminationHandler) {
    json params = {{"input", -10}};

    Task task("Failure Task", params, [](const json& p) -> json {
        int input = p.at("input");
        if (input < 0) {
            throw std::invalid_argument("Input cannot be negative");
        }
        return json{{"result", input * input}};
    });

    EXPECT_THROW(task.run(), std::invalid_argument);
}

TEST_F(TaskSequencerTest, TargetExecutionWithTasks) {
    Target target("Test Target", 2, 1);

    json params1 = {{"input", 5}};
    json params2 = {{"input", 3}};

    auto task1 =
        std::make_shared<Task>("Task 1", params1, [](const json& p) -> json {
            int input = p.at("input");
            return json{{"result", input + 2}};
        });

    auto task2 =
        std::make_shared<Task>("Task 2", params2, [](const json& p) -> json {
            int input = p.at("input");
            return json{{"result", input * 3}};
        });

    target.addTask(task1);
    target.addTask(task2);

    std::stop_source stopSource;
    std::atomic<bool> pauseFlag(false);
    std::mutex mtx;
    std::condition_variable_any cv;

    target.execute(stopSource.get_token(), pauseFlag, cv, mtx);

    EXPECT_EQ(task1->getResult().value()["result"], 7);
    EXPECT_EQ(task2->getResult().value()["result"], 9);
}

TEST_F(TaskSequencerTest, ExposureSequenceExecutionWithTaskFailures) {
    ExposureSequence sequence;

    // First target with a successful task
    Target target1("Target 1", 1, 1);
    json params1 = {{"input", 5}};
    auto task1 =
        std::make_shared<Task>("Task 1", params1, [](const json& p) -> json {
            int input = p.at("input");
            return json{{"result", input + 1}};
        });
    target1.addTask(task1);
    sequence.addTarget(target1);

    // Second target with a failing task
    Target target2("Target 2", 1, 2);
    json params2 = {{"input", -5}};
    bool exceptionHandled = false;
    auto task2 = std::make_shared<Task>(
        "Task 2", params2,
        [](const json& p) -> json {
            int input = p.at("input");
            if (input < 0) {
                throw std::invalid_argument("Negative input");
            }
            return json{{"result", input}};
        },
        [&](const std::exception& ex) {
            EXPECT_STREQ(ex.what(), "Negative input");
            exceptionHandled = true;
        });
    target2.addTask(task2);
    sequence.addTarget(target2);

    sequence.executeAll();

    // Verify results
    EXPECT_EQ(task1->getResult().value()["result"], 6);
    EXPECT_TRUE(exceptionHandled);
}

TEST_F(TaskSequencerTest, ExposureSequenceStopDuringExecution) {
    ExposureSequence sequence;

    Target target1("Target 1", 1, 1);
    json params1 = {{"input", 5}};
    auto task1 =
        std::make_shared<Task>("Task 1", params1, [](const json& p) -> json {
            int input = p.at("input");
            return json{{"result", input + 1}};
        });
    target1.addTask(task1);

    Target target2("Target 2", 1, 2);
    json params2 = {{"input", 3}};
    auto task2 =
        std::make_shared<Task>("Task 2", params2, [](const json& p) -> json {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            int input = p.at("input");
            return json{{"result", input * 2}};
        });
    target2.addTask(task2);

    sequence.addTarget(target1);
    sequence.addTarget(target2);

    // Execute sequence in a separate thread
    std::thread sequenceThread([&]() { sequence.executeAll(); });

    // Stop the sequence after a short delay
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sequence.stop();

    sequenceThread.join();

    // Verify that the first task completed, but the second one did not due to
    // the stop
    EXPECT_EQ(task1->getResult().value()["result"], 6);
    EXPECT_THROW(task2->getResult().value().at("result"),
                 nlohmann::json::out_of_range);
}
