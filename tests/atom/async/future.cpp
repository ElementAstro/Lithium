#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <thread>


#include "atom/async/future.hpp"

using namespace atom::async;

TEST(EnhancedFutureTest, Constructor) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));
    EXPECT_FALSE(enhancedFuture.isCancelled());
}

TEST(EnhancedFutureTest, Then) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));
    auto nextFuture = enhancedFuture.then([](int value) { return value + 1; });

    promise.set_value(42);
    EXPECT_EQ(nextFuture.wait(), 43);
}

TEST(EnhancedFutureTest, WaitFor) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    std::thread([&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        promise.set_value(42);
    }).detach();

    auto result = enhancedFuture.waitFor(std::chrono::milliseconds(200));
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(EnhancedFutureTest, IsDone) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    EXPECT_FALSE(enhancedFuture.isDone());
    promise.set_value(42);
    EXPECT_TRUE(enhancedFuture.isDone());
}

TEST(EnhancedFutureTest, OnComplete) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    bool callbackCalled = false;
    enhancedFuture.onComplete([&callbackCalled](int value) {
        callbackCalled = true;
        EXPECT_EQ(value, 42);
    });

    promise.set_value(42);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(callbackCalled);
}

TEST(EnhancedFutureTest, Wait) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    promise.set_value(42);
    EXPECT_EQ(enhancedFuture.wait(), 42);
}

TEST(EnhancedFutureTest, Cancel) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    enhancedFuture.cancel();
    EXPECT_TRUE(enhancedFuture.isCancelled());
}

TEST(EnhancedFutureTest, GetException) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    promise.set_exception(std::make_exception_ptr(std::runtime_error("error")));
    auto exceptionPtr = enhancedFuture.getException();
    EXPECT_TRUE(exceptionPtr != nullptr);
}

TEST(EnhancedFutureTest, Retry) {
    std::promise<int> promise;
    auto future = promise.get_future().share();
    EnhancedFuture<int> enhancedFuture(std::move(future));

    auto retryFuture =
        enhancedFuture.retry([](int value) { return value + 1; }, 3);
    promise.set_value(42);
    EXPECT_EQ(retryFuture.wait(), 43);
}

TEST(MakeEnhancedFutureTest, CreateEnhancedFuture) {
    auto enhancedFuture = makeEnhancedFuture([]() { return 42; });
    EXPECT_EQ(enhancedFuture.wait(), 42);
}

TEST(WhenAllTest, RangeOfFutures) {
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() { return i; }));
    }

    auto resultFuture = whenAll(futures.begin(), futures.end());
    auto results = resultFuture.get();
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(results[i].get(), i);
    }
}

TEST(WhenAllTest, VariadicFutures) {
    auto future1 = std::async(std::launch::async, []() { return 1; });
    auto future2 = std::async(std::launch::async, []() { return 2; });
    auto future3 = std::async(std::launch::async, []() { return 3; });

    auto resultFuture =
        whenAll(std::move(future1), std::move(future2), std::move(future3));
    auto results = resultFuture.get();
    EXPECT_EQ(std::get<0>(results), 1);
    EXPECT_EQ(std::get<1>(results), 2);
    EXPECT_EQ(std::get<2>(results), 3);
}