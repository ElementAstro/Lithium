#include <gtest/gtest.h>

#include "atom/async/limiter.hpp"

#include <future>

using namespace atom::async;

// 测试 RateLimiter 构造函数
class RateLimiterTest : public ::testing::Test {
protected:
    RateLimiter limiter;
};

// 辅助函数：模拟异步操作
auto simulateAsyncOperation(RateLimiter& limiter,
                            const std::string& functionName)
    -> std::future<void> {
    return std::async(std::launch::async, [&limiter, functionName]() -> void {
        auto awaiter = limiter.acquire(functionName);
        awaiter.await_ready();
        awaiter.await_suspend(std::noop_coroutine());
        awaiter.await_resume();
    });
}

TEST_F(RateLimiterTest, DefaultSettings) {
    auto awaiter = limiter.acquire("test_function");
    EXPECT_FALSE(awaiter.await_ready());
}

TEST_F(RateLimiterTest, SetFunctionLimit) {
    limiter.setFunctionLimit("test_function", 10, std::chrono::seconds(1));

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 15; ++i) {
        futures.push_back(simulateAsyncOperation(limiter, "test_function"));
    }

    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(limiter.get_rejected_requests("test_function"), 5);
}

TEST_F(RateLimiterTest, PauseResume) {
    limiter.setFunctionLimit("test_function", 5, std::chrono::seconds(1));

    limiter.pause();

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(simulateAsyncOperation(limiter, "test_function"));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(limiter.get_rejected_requests("test_function"), 0);

    limiter.resume();

    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(limiter.get_rejected_requests("test_function"), 5);
}

TEST_F(RateLimiterTest, MultipleFunction) {
    limiter.setFunctionLimit("function1", 5, std::chrono::seconds(1));
    limiter.setFunctionLimit("function2", 10, std::chrono::seconds(1));

    std::vector<std::future<void>> futures1, futures2;
    for (int i = 0; i < 10; ++i) {
        futures1.push_back(simulateAsyncOperation(limiter, "function1"));
        futures2.push_back(simulateAsyncOperation(limiter, "function2"));
    }

    for (auto& future : futures1) {
        future.wait();
    }
    for (auto& future : futures2) {
        future.wait();
    }

    EXPECT_EQ(limiter.get_rejected_requests("function1"), 5);
    EXPECT_EQ(limiter.get_rejected_requests("function2"), 0);
}

TEST_F(RateLimiterTest, TimeWindowReset) {
    limiter.setFunctionLimit("test_function", 5, std::chrono::seconds(1));

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.push_back(simulateAsyncOperation(limiter, "test_function"));
    }

    for (auto& future : futures) {
        future.wait();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    futures.clear();
    for (int i = 0; i < 5; ++i) {
        futures.push_back(simulateAsyncOperation(limiter, "test_function"));
    }

    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(limiter.get_rejected_requests("test_function"), 0);
}