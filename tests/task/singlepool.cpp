#include "task/singlepool.hpp"
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>


using namespace lithium;

class SingleThreadPoolTest : public ::testing::Test {
protected:
    SingleThreadPool pool;

    virtual void SetUp() {
        // Setup code if needed
    }

    virtual void TearDown() {
        // Cleanup code if needed
        pool.quit();
    }
};

TEST_F(SingleThreadPoolTest, TestStart) {
    std::atomic<bool> taskStarted = false;

    auto task = [&taskStarted](const std::atomic_bool& quitting) {
        taskStarted = true;
        while (!quitting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    EXPECT_TRUE(pool.start(task));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(taskStarted);
}

TEST_F(SingleThreadPoolTest, TestStartDetach) {
    std::atomic<bool> taskStarted = false;

    auto task = [&taskStarted](const std::atomic_bool& quitting) {
        taskStarted = true;
        while (!quitting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    pool.startDetach(task);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(taskStarted);
}

TEST_F(SingleThreadPoolTest, TestTryStart) {
    std::atomic<bool> taskStarted = false;

    auto task = [&taskStarted](const std::atomic_bool& quitting) {
        taskStarted = true;
        while (!quitting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    EXPECT_TRUE(pool.tryStart(task));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(taskStarted);
}

TEST_F(SingleThreadPoolTest, TestTryStartDetach) {
    std::atomic<bool> taskStarted = false;

    auto task = [&taskStarted](const std::atomic_bool& quitting) {
        taskStarted = true;
        while (!quitting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    pool.tryStartDetach(task);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(taskStarted);
}

TEST_F(SingleThreadPoolTest, TestQuit) {
    std::atomic<bool> taskStarted = false;
    std::atomic<bool> taskEnded = false;

    auto task = [&taskStarted, &taskEnded](const std::atomic_bool& quitting) {
        taskStarted = true;
        while (!quitting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        taskEnded = true;
    };

    pool.start(task);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(taskStarted);

    pool.quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(taskEnded);
}