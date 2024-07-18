#include "debug/progress.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>

namespace lithium::debug {

class MockProgressBar : public ProgressBar {
public:
    using ProgressBar::ProgressBar;
    MOCK_METHOD(void, printProgressBar, ());
};

TEST(ProgressBarTest, Start) {
    MockProgressBar progressBar(100, 50, '#', '-', true, Color::Green);

    EXPECT_CALL(progressBar, printProgressBar()).Times(testing::AtLeast(1));

    progressBar.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    progressBar.stop();
    progressBar.wait();
}

TEST(ProgressBarTest, PauseResume) {
    MockProgressBar progressBar(100, 50, '#', '-', true, Color::Green);

    EXPECT_CALL(progressBar, printProgressBar()).Times(testing::AtLeast(1));

    progressBar.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    progressBar.pause();
    int current1 = progressBar.getCurrent();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    int current2 = progressBar.getCurrent();
    ASSERT_EQ(current1, current2);
    progressBar.resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_GT(progressBar.getCurrent(), current2);
    progressBar.stop();
    progressBar.wait();
}

TEST(ProgressBarTest, Reset) {
    MockProgressBar progressBar(100, 50, '#', '-', true, Color::Green);

    EXPECT_CALL(progressBar, printProgressBar()).Times(testing::AtLeast(1));

    progressBar.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    progressBar.reset();
    ASSERT_EQ(progressBar.getCurrent(), 0);
    progressBar.stop();
    progressBar.wait();
}

}  // namespace lithium::debug
