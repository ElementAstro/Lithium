#include "task/pool.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace lithium;

// Helper function for delay
void sleep_for_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// A simple task that returns a value
int simpleTask() {
    sleep_for_ms(100);
    return 42;
}

// A task that throws an exception
void exceptionTask() { throw std::runtime_error("Intentional exception"); }

// A task that just sleeps
void longTask() { sleep_for_ms(500); }

// Test case for creating and destroying a TaskPool
TEST(TaskPoolTest, CreateDestroyPool) {
    EXPECT_NO_THROW({ auto pool = TaskPool::createShared(4); });
}

// Test case for basic task execution
TEST(TaskPoolTest, BasicTaskExecution) {
    auto pool = TaskPool::createShared(4);
    auto future = pool->enqueue(simpleTask);
    EXPECT_EQ(future.get(), 42);
}

// Test case for exception handling in tasks
TEST(TaskPoolTest, ExceptionHandling) {
    auto pool = TaskPool::createShared(4);
    EXPECT_NO_THROW({
        auto future = pool->enqueue(exceptionTask);
        // The exception should be handled inside the task, not propagated
        EXPECT_NO_THROW(future.get());
    });
}

// Test case for resizing the pool
TEST(TaskPoolTest, ResizePool) {
    auto pool = TaskPool::createShared(4);
    EXPECT_EQ(pool->getThreadCount(), 4);
    pool->resize(8);
    EXPECT_EQ(pool->getThreadCount(), 8);
    pool->resize(2);
    EXPECT_EQ(pool->getThreadCount(), 2);
}

// Test case for submitting multiple tasks
TEST(TaskPoolTest, MultipleTasks) {
    auto pool = TaskPool::createShared(4);

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool->enqueue([i]() {
            sleep_for_ms(50);
            return i;
        }));
    }

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(futures[i].get(), i);
    }
}

// Test case for submitting tasks after stopping acceptance
TEST(TaskPoolTest, StopAcceptingTasks) {
    auto pool = TaskPool::createShared(4);
    pool->stopAcceptingTasks();

    EXPECT_THROW(pool->enqueue(simpleTask), std::runtime_error);
}

// Test case for task stealing across threads
TEST(TaskPoolTest, TaskStealing) {
    auto pool = TaskPool::createShared(4);

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool->enqueue(longTask));
    }

    for (auto& future : futures) {
        EXPECT_NO_THROW(future.get());
    }
}

// Test case for zero threads (should throw)
TEST(TaskPoolTest, ZeroThreads) {
    EXPECT_THROW(
        { auto pool = TaskPool::createShared(0); }, std::invalid_argument);
}

// Test case for task execution order and load balancing
TEST(TaskPoolTest, ExecutionOrderAndLoadBalancing) {
    auto pool = TaskPool::createShared(4);
    std::atomic<int> counter{0};

    auto future1 = pool->enqueue([&counter]() {
        sleep_for_ms(200);
        return ++counter;
    });

    auto future2 = pool->enqueue([&counter]() {
        sleep_for_ms(100);
        return ++counter;
    });

    auto future3 = pool->enqueue([&counter]() {
        sleep_for_ms(50);
        return ++counter;
    });

    EXPECT_EQ(future1.get(), 3);
    EXPECT_EQ(future2.get(), 2);
    EXPECT_EQ(future3.get(), 1);
}

// Test case for checking if pool stops correctly
TEST(TaskPoolTest, StopPool) {
    auto pool = TaskPool::createShared(4);

    auto future = pool->enqueue(simpleTask);
    EXPECT_EQ(future.get(), 42);

    pool->resize(0);
    EXPECT_EQ(pool->getThreadCount(), 0);
}

// Test case for stealing tasks when one queue is empty
TEST(TaskPoolTest, StealTasksWhenEmpty) {
    auto pool = TaskPool::createShared(4);
    std::atomic<int> counter{0};

    // Simulate a situation where one thread has tasks, others do not
    for (int i = 0; i < 10; ++i) {
        pool->enqueue([&counter]() {
            sleep_for_ms(50);
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    sleep_for_ms(600);  // Give enough time for all tasks to finish

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 10);
}

// Test case for very large number of tasks
TEST(TaskPoolTest, LargeNumberOfTasks) {
    auto pool = TaskPool::createShared(8);

    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;

    for (int i = 0; i < 1000; ++i) {
        futures.push_back(pool->enqueue(
            [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); }));
    }

    for (auto& future : futures) {
        future.get();
    }

    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1000);
}
