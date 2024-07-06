#include <gtest/gtest.h>
#include <atomic>
#include <chrono>

#include "atom/async/thread_wrapper.hpp"

using namespace std::chrono_literals;
using namespace atom::async;

TEST(ThreadWrapperTest, StartAndJoin) {
    Thread thread;
    std::atomic_bool executed{false};

    thread.start([&executed] { executed = true; });

    ASSERT_TRUE(thread.running());
    thread.join();
    ASSERT_FALSE(thread.running());
    ASSERT_TRUE(executed);
}

TEST(ThreadWrapperTest, StartWithStopToken) {
    Thread thread;
    std::atomic_bool stopRequested{false};

    thread.start([&stopRequested](std::stop_token stopToken) {
        while (!stopToken.stop_requested()) {
            std::this_thread::sleep_for(10ms);
        }
        stopRequested = true;
    });

    ASSERT_TRUE(thread.running());
    std::this_thread::sleep_for(50ms);
    thread.requestStop();
    thread.join();
    ASSERT_FALSE(thread.running());
    ASSERT_TRUE(stopRequested);
}

TEST(ThreadWrapperTest, RequestStopWithoutStart) {
    Thread thread;
    thread.requestStop();  // Should not cause any issues.
    ASSERT_FALSE(thread.running());
}

TEST(ThreadWrapperTest, JoinWithoutStart) {
    Thread thread;
    thread.join();  // Should not cause any issues.
    ASSERT_FALSE(thread.running());
}

TEST(ThreadWrapperTest, GetThreadId) {
    Thread thread;
    auto mainThreadId = std::this_thread::get_id();
    auto threadId = thread.getId();
    ASSERT_EQ(mainThreadId, threadId);

    std::atomic_bool executed{false};
    thread.start([&executed, &threadId] {
        threadId = std::this_thread::get_id();
        executed = true;
    });

    thread.join();
    ASSERT_TRUE(executed);
    ASSERT_NE(mainThreadId, threadId);
}

TEST(ThreadWrapperTest, SwapThreads) {
    Thread thread1;
    Thread thread2;

    std::atomic_bool executed1{false};
    std::atomic_bool executed2{false};

    thread1.start([&executed1] { executed1 = true; });

    thread2.start([&executed2] { executed2 = true; });

    thread1.swap(thread2);

    thread1.join();
    thread2.join();

    ASSERT_TRUE(executed1);
    ASSERT_TRUE(executed2);
}
