#include "task/tick.hpp"  // 假设 tick.hpp 是包含 TickScheduler 类的头文件
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace lithium;
using namespace std::chrono_literals;

class TickSchedulerTest : public ::testing::Test {
protected:
    void SetUp() override { scheduler = TickScheduler::createShared(); }

    void TearDown() override {
        scheduler->pause();  // 确保调度器在测试结束时暂停
    }

    std::shared_ptr<TickScheduler> scheduler;
};

// 基本任务调度测试
TEST_F(TickSchedulerTest, BasicTaskScheduling) {
    bool taskExecuted = false;
    scheduler->scheduleTask(1, false, 0, 0ms, std::nullopt, std::nullopt,
                            std::nullopt,
                            [&taskExecuted]() { taskExecuted = true; });

    std::this_thread::sleep_for(200ms);
    EXPECT_TRUE(taskExecuted);
}

// 测试任务延迟调度
TEST_F(TickSchedulerTest, DelayTask) {
    bool taskExecuted = false;
    auto task = scheduler->scheduleTask(
        1, false, 0, 0ms, std::nullopt, std::nullopt, std::nullopt,
        [&taskExecuted]() { taskExecuted = true; });

    scheduler->delayTask(task->id, 10);
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(taskExecuted);

    std::this_thread::sleep_for(1s);
    EXPECT_TRUE(taskExecuted);
}

// 测试任务依赖关系
TEST_F(TickSchedulerTest, TaskWithDependencies) {
    bool task1Executed = false;
    bool task2Executed = false;

    auto task1 = scheduler->scheduleTask(
        1, false, 0, 0ms, std::nullopt, std::nullopt, std::nullopt,
        [&task1Executed]() { task1Executed = true; });

    auto task2 = scheduler->scheduleTask(
        2, false, 0, 0ms, std::nullopt, std::nullopt, std::nullopt,
        [&task2Executed]() { task2Executed = true; });

    scheduler->addDependency(task2, task1);

    std::this_thread::sleep_for(200ms);
    EXPECT_TRUE(task1Executed);
    EXPECT_FALSE(task2Executed);

    std::this_thread::sleep_for(200ms);
    EXPECT_TRUE(task2Executed);
}

// 测试取消任务
TEST_F(TickSchedulerTest, CancelTask) {
    bool taskExecuted = false;
    auto task = scheduler->scheduleTask(
        1, false, 0, 0ms, std::nullopt, std::nullopt, std::nullopt,
        [&taskExecuted]() { taskExecuted = true; });

    scheduler->cancelTask(task->id);
    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(taskExecuted);
}

// 测试超时重试功能
TEST_F(TickSchedulerTest, RetryTaskOnFailure) {
    static int executionCount = 0;
    executionCount = 0;

    scheduler->scheduleTask(
        1, false, 2, 100ms, std::nullopt, std::nullopt, std::nullopt, []() {
            if (++executionCount < 3) {
                throw std::runtime_error("Simulated failure");
            }
        });

    std::this_thread::sleep_for(500ms);
    EXPECT_EQ(executionCount, 3);
}

// 测试手动模式下的任务触发
TEST_F(TickSchedulerTest, ManualModeTaskTriggering) {
    bool taskExecuted = false;
    scheduler->switchToManualMode();

    scheduler->scheduleTask(1, false, 0, 0ms, std::nullopt, std::nullopt,
                            std::nullopt,
                            [&taskExecuted]() { taskExecuted = true; });

    scheduler->triggerTasks();
    EXPECT_TRUE(taskExecuted);
}

// 测试多任务并发限制
TEST_F(TickSchedulerTest, MaxConcurrentTasks) {
    scheduler->setMaxConcurrentTasks(1);

    bool task1Executed = false;
    bool task2Executed = false;

    scheduler->scheduleTask(1, false, 0, 0ms, std::nullopt, std::nullopt,
                            std::nullopt, [&task1Executed]() {
                                std::this_thread::sleep_for(200ms);
                                task1Executed = true;
                            });

    scheduler->scheduleTask(1, false, 0, 0ms, std::nullopt, std::nullopt,
                            std::nullopt,
                            [&task2Executed]() { task2Executed = true; });

    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(task1Executed);
    EXPECT_FALSE(task2Executed);

    std::this_thread::sleep_for(200ms);
    EXPECT_TRUE(task2Executed);
}

// 测试边缘情况：在暂停状态下调度任务
TEST_F(TickSchedulerTest, ScheduleTaskWhilePaused) {
    scheduler->pause();

    bool taskExecuted = false;
    scheduler->scheduleTask(1, false, 0, 0ms, std::nullopt, std::nullopt,
                            std::nullopt,
                            [&taskExecuted]() { taskExecuted = true; });

    std::this_thread::sleep_for(200ms);
    EXPECT_FALSE(taskExecuted);

    scheduler->resume();
    std::this_thread::sleep_for(200ms);
    EXPECT_TRUE(taskExecuted);
}

// 测试边缘情况：任务的最大重试次数为0
TEST_F(TickSchedulerTest, MaxRetryCountZero) {
    static int executionCount = 0;
    executionCount = 0;

    scheduler->scheduleTask(1, false, 0, 100ms, std::nullopt, std::nullopt,
                            std::nullopt, []() {
                                ++executionCount;
                                throw std::runtime_error("Simulated failure");
                            });

    std::this_thread::sleep_for(200ms);
    EXPECT_EQ(executionCount, 1);
}
