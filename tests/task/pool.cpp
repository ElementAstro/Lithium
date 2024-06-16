#include "task/pool.hpp"
#include <gtest/gtest.h>


TEST(TaskPoolTest, EnqueueTask) {
    lithium::TaskPool pool;
    bool taskExecuted = false;

    auto future = pool.enqueue([&taskExecuted]() { taskExecuted = true; });

    ASSERT_TRUE(future.wait_for(std::chrono::seconds(1)) ==
                std::future_status::ready);
    ASSERT_TRUE(taskExecuted);
}

TEST(TaskPoolTest, ResizeTaskPool) {
    lithium::TaskPool pool;
    size_t initialThreadCount = pool.getThreadCount();

    pool.resize(initialThreadCount + 1);
    size_t newThreadCount = pool.getThreadCount();

    ASSERT_GT(newThreadCount, initialThreadCount);
}