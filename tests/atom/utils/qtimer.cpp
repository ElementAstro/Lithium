#include <gtest/gtest.h>
#include <thread>

#include "atom/utils/qtimer.hpp"

using namespace atom::utils;

class ElapsedTimerTest : public ::testing::Test {
protected:
    ElapsedTimer timer;
};

// Test start and elapsed time
TEST_F(ElapsedTimerTest, StartAndElapsedTime) {
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_GE(timer.elapsedMs(), 100);
}

// Test isValid
TEST_F(ElapsedTimerTest, IsValid) { ASSERT_TRUE(timer.isValid()); }

// Test elapsed time in different units
TEST_F(ElapsedTimerTest, ElapsedTimeUnits) {
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_GE(timer.elapsedNs(), 1000000);  // 1 ms = 1000000 ns
    ASSERT_GE(timer.elapsedUs(), 1000);     // 1 ms = 1000 us
    ASSERT_GE(timer.elapsedMs(), 1);
}

// Test hasExpired
TEST_F(ElapsedTimerTest, HasExpired) {
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_TRUE(timer.hasExpired(30));
    ASSERT_FALSE(timer.hasExpired(100));
}

// Test remainingTimeMs
TEST_F(ElapsedTimerTest, RemainingTimeMs) {
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_GE(timer.remainingTimeMs(100), 50);
    ASSERT_EQ(timer.remainingTimeMs(40), 0);
}

// Test currentTimeMs
TEST(ElapsedTimerStaticTest, CurrentTimeMs) {
    int64_t currentTime = ElapsedTimer::currentTimeMs();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_LE(currentTime, ElapsedTimer::currentTimeMs());
}

// Test comparison operators
TEST_F(ElapsedTimerTest, ComparisonOperators) {
    ElapsedTimer timer2;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timer2.start();

    ASSERT_TRUE(timer < timer2);
    ASSERT_FALSE(timer > timer2);
    ASSERT_TRUE(timer <= timer2);
    ASSERT_FALSE(timer >= timer2);
    ASSERT_FALSE(timer == timer2);
    ASSERT_TRUE(timer != timer2);
}
