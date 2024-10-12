#include <gtest/gtest.h>
#include <stdexcept>

#include "atom/async/packaged_task.hpp"

using namespace atom::async;

TEST(EnhancedPackagedTaskTest, Initialization) {
    EnhancedPackagedTask<int, int> task([](int x) { return x * 2; });
    auto future = task.getEnhancedFuture();
    EXPECT_FALSE(future.isReady());
}

TEST(EnhancedPackagedTaskTest, Execution) {
    EnhancedPackagedTask<int, int> task([](int x) { return x * 2; });
    auto future = task.getEnhancedFuture();
    task(5);
    EXPECT_EQ(future.get(), 10);
}

TEST(EnhancedPackagedTaskTest, VoidExecution) {
    bool executed = false;
    EnhancedPackagedTask<void> task([&executed]() { executed = true; });
    auto future = task.getEnhancedFuture();
    task();
    future.get();  // Ensure the task has completed
    EXPECT_TRUE(executed);
}

TEST(EnhancedPackagedTaskTest, Callbacks) {
    EnhancedPackagedTask<int, int> task([](int x) { return x * 2; });
    bool callbackCalled = false;
    task.onComplete([&callbackCalled](int result) {
        callbackCalled = true;
        EXPECT_EQ(result, 10);
    });
    task(5);
    EXPECT_TRUE(callbackCalled);
}

TEST(EnhancedPackagedTaskTest, VoidCallbacks) {
    EnhancedPackagedTask<void> task([]() {});
    bool callbackCalled = false;
    task.onComplete([&callbackCalled]() { callbackCalled = true; });
    task();
    EXPECT_TRUE(callbackCalled);
}

TEST(EnhancedPackagedTaskTest, Cancellation) {
    EnhancedPackagedTask<int, int> task([](int x) { return x * 2; });
    task.cancel();
    auto future = task.getEnhancedFuture();
    task(5);
    EXPECT_THROW(future.get(), std::runtime_error);
    EXPECT_TRUE(task.isCancelled());
}

TEST(EnhancedPackagedTaskTest, VoidCancellation) {
    EnhancedPackagedTask<void> task([]() {});
    task.cancel();
    auto future = task.getEnhancedFuture();
    task();
    EXPECT_THROW(future.get(), std::runtime_error);
    EXPECT_TRUE(task.isCancelled());
}

TEST(EnhancedPackagedTaskTest, ExceptionHandling) {
    EnhancedPackagedTask<int, int> task(
        [](int) -> int { throw std::runtime_error("error"); });
    auto future = task.getEnhancedFuture();
    task(5);
    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST(EnhancedPackagedTaskTest, VoidExceptionHandling) {
    EnhancedPackagedTask<void> task(
        []() { throw std::runtime_error("error"); });
    auto future = task.getEnhancedFuture();
    task();
    EXPECT_THROW(future.get(), std::runtime_error);
}
