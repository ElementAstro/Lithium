#include "atom/async/timer.hpp"
#include <gtest/gtest.h>

TEST(TimerTest, setTimeout) {
    atom::async::Timer timer;
    bool funcCalled = false;

    auto future = timer.setTimeout([&funcCalled]() { funcCalled = true; }, 100);

    // Wait for the task to complete
    future.wait();

    EXPECT_TRUE(funcCalled);
}

TEST(TimerTest, setInterval) {
    atom::async::Timer timer;
    int funcCalls = 0;

    timer.setInterval([&funcCalls]() { funcCalls++; }, 100, 5, 0);

    // Run the timer for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    EXPECT_GE(funcCalls, 5);
}

TEST(TimerTest, cancelAllTasks) {
    atom::async::Timer timer;
    bool funcCalled = false;

    timer.setTimeout([&funcCalled]() { funcCalled = true; }, 100);
    timer.cancelAllTasks();

    // Wait for the task to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(funcCalled);
}

TEST(TimerTest, pause) {
    atom::async::Timer timer;
    bool funcCalled = false;

    timer.setTimeout([&funcCalled]() { funcCalled = true; }, 100);
    timer.pause();

    // Wait for the task to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(funcCalled);
}

TEST(TimerTest, resume) {
    atom::async::Timer timer;
    bool funcCalled = false;

    timer.setTimeout([&funcCalled]() { funcCalled = true; }, 100);
    timer.pause();
    timer.resume();

    // Wait for the task to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(funcCalled);
}

TEST(TimerTest, stop) {
    atom::async::Timer timer;
    bool funcCalled = false;

    timer.setTimeout([&funcCalled]() { funcCalled = true; }, 100);
    timer.stop();

    // Wait for the task to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(funcCalled);
}

TEST(TimerTest, setCallback) {
    atom::async::Timer timer;
    bool callbackCalled = false;

    timer.setCallback([&callbackCalled]() { callbackCalled = true; });

    // Trigger the callback
    timer.now();

    EXPECT_TRUE(callbackCalled);
}

TEST(TimerTest, getTaskCount) {
    atom::async::Timer timer;

    EXPECT_EQ(timer.getTaskCount(), 0);

    timer.setTimeout([]() {}, 100);
    EXPECT_EQ(timer.getTaskCount(), 1);

    timer.setInterval([]() {}, 100, 5, 0);
    EXPECT_EQ(timer.getTaskCount(), 2);
}