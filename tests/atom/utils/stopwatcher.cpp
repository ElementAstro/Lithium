#include "atom/utils/stopwatcher.hpp"
#include <gtest/gtest.h>
#include <thread>


using namespace atom::utils;

class StopWatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在测试前执行的初始化代码
    }

    void TearDown() override {
        // 在测试后执行的清理代码
    }

    StopWatcher stopWatcher;
};

TEST_F(StopWatcherTest, StartStopWatch) {
    stopWatcher.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopWatcher.stop();
    EXPECT_GE(stopWatcher.elapsedMilliseconds(), 100);
}

TEST_F(StopWatcherTest, PauseResumeWatch) {
    stopWatcher.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopWatcher.pause();
    double elapsed1 = stopWatcher.elapsedMilliseconds();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopWatcher.resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopWatcher.stop();
    double elapsed2 = stopWatcher.elapsedMilliseconds();
    EXPECT_GE(elapsed1, 100);
    EXPECT_GE(elapsed2, 200);
}

TEST_F(StopWatcherTest, ResetWatch) {
    stopWatcher.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopWatcher.stop();
    stopWatcher.reset();
    EXPECT_EQ(stopWatcher.elapsedMilliseconds(), 0);
}

TEST_F(StopWatcherTest, ElapsedFormatted) {
    stopWatcher.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    stopWatcher.stop();
    std::string formatted = stopWatcher.elapsedFormatted();
    EXPECT_NE(formatted.find("seconds"), std::string::npos);
}

TEST_F(StopWatcherTest, RegisterCallback) {
    bool callbackTriggered = false;
    stopWatcher.registerCallback(
        [&callbackTriggered]() { callbackTriggered = true; }, 100);

    stopWatcher.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    stopWatcher.stop();

    EXPECT_TRUE(callbackTriggered);
}
