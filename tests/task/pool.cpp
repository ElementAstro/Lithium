#include "task/pool.hpp"
#include <gtest/gtest.h>

class TaskPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        taskPool =
            lithium::TaskPool::createShared(4);  // 创建一个具有4个线程的任务池
    }

    std::shared_ptr<lithium::TaskPool> taskPool;
};

TEST_F(TaskPoolTest, EnqueueTask) {
    std::atomic<int> counter{0};

    auto task = [&counter]() { counter++; };

    auto future = taskPool->enqueue(task);
    future.get();  // 等待任务完成

    EXPECT_EQ(counter.load(), 1);
}

TEST_F(TaskPoolTest, EnqueueMultipleTasks) {
    std::atomic<int> counter{0};

    auto task = [&counter]() { counter++; };

    const int taskCount = 10;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < taskCount; ++i) {
        futures.emplace_back(taskPool->enqueue(task));
    }

    for (auto& future : futures) {
        future.get();  // 等待所有任务完成
    }

    EXPECT_EQ(counter.load(), taskCount);
}

/*
TEST_F(TaskPoolTest, ResizePool) {
    taskPool->resize(8);
    EXPECT_EQ(taskPool->getThreadCount(), 8);

    taskPool->resize(2);
    EXPECT_EQ(taskPool->getThreadCount(), 2);
}
*/

TEST_F(TaskPoolTest, TaskStealing) {
    // 测试任务窃取机制
    std::atomic<int> counter{0};

    auto task = [&counter]() { counter++; };

    const int taskCount = 20;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < taskCount; ++i) {
        futures.emplace_back(taskPool->enqueue(task));
    }

    for (auto& future : futures) {
        future.get();  // 等待所有任务完成
    }

    EXPECT_EQ(counter.load(), taskCount);
}