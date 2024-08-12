#include <gtest/gtest.h>
#include <string>

#include "atom/async/slot.hpp"
using namespace atom::async;

auto captureOutput(const std::function<void()>& func) -> std::string {
    std::stringstream buffer;
    std::streambuf* prevbuf = std::cout.rdbuf(buffer.rdbuf());
    func();
    std::cout.rdbuf(prevbuf);
    return buffer.str();
}

TEST(SignalTest, BasicSignal) {
    Signal<int, std::string> signal;
    std::string output = captureOutput([&]() {
        signal.connect([](int x, const std::string& s) {
            std::cout << "Signal with parameters: " << x << ", " << s << '\n';
        });
        signal.emit(42, "Hello");
    });
    EXPECT_EQ(output, "Signal with parameters: 42, Hello\n");
}

TEST(SignalTest, AsyncSignal) {
    AsyncSignal<int> asyncSignal;
    std::string output = captureOutput([&]() {
        asyncSignal.connect(
            [](int x) { std::cout << "Async Signal: " << x << '\n'; });
        asyncSignal.emit(84);
    });
    EXPECT_EQ(output, "Async Signal: 84\n");
}

TEST(SignalTest, AutoDisconnectSignal) {
    AutoDisconnectSignal<int> autoDisconnectSignal;
    int id = autoDisconnectSignal.connect(
        [](int x) { std::cout << "Auto Disconnect Slot: " << x << '\n'; });

    std::string output1 =
        captureOutput([&]() { autoDisconnectSignal.emit(100); });
    EXPECT_EQ(output1, "Auto Disconnect Slot: 100\n");

    autoDisconnectSignal.disconnect(id);

    std::string output2 =
        captureOutput([&]() { autoDisconnectSignal.emit(200); });
    EXPECT_EQ(output2, "");
}

TEST(SignalTest, ChainedSignal) {
    ChainedSignal<int> chain1;
    ChainedSignal<int> chain2;
    std::string output = captureOutput([&]() {
        chain1.connect([](int x) { std::cout << "Chain 1: " << x << '\n'; });
        chain2.connect([](int x) { std::cout << "Chain 2: " << x << '\n'; });
        chain1.addChain(chain2);
        chain1.emit(300);
    });
    EXPECT_EQ(output, "Chain 1: 300\nChain 2: 300\n");
}

TEST(SignalTest, TemplateSignal) {
    TemplateSignal<int, std::string> templateSignal;
    std::string output = captureOutput([&]() {
        templateSignal.connect([](int x, const std::string& s) {
            std::cout << "Template Signal: " << x << ", " << s << '\n';
        });
        templateSignal.emit(400, "World");
    });
    EXPECT_EQ(output, "Template Signal: 400, World\n");
}

TEST(SignalTest, ThreadSafeSignal) {
    ThreadSafeSignal<int> threadSafeSignal;
    std::string output = captureOutput([&]() {
        threadSafeSignal.connect(
            [](int x) { std::cout << "ThreadSafe Signal: " << x << '\n'; });
        threadSafeSignal.emit(42);
    });
    EXPECT_EQ(output, "ThreadSafe Signal: 42\n");
}

TEST(SignalTest, BroadcastSignal) {
    BroadcastSignal<int> broadcastSignal1, broadcastSignal2;
    std::string output = captureOutput([&]() {
        broadcastSignal1.connect(
            [](int x) { std::cout << "Broadcast Signal 1: " << x << '\n'; });
        broadcastSignal2.connect(
            [](int x) { std::cout << "Broadcast Signal 2: " << x << '\n'; });
        broadcastSignal1.addChain(broadcastSignal2);
        broadcastSignal1.emit(84);
    });
    EXPECT_EQ(output, "Broadcast Signal 1: 84\nBroadcast Signal 2: 84\n");
}

TEST(SignalTest, LimitedSignal) {
    LimitedSignal<int> limitedSignal(3);
    std::string output = captureOutput([&]() {
        limitedSignal.connect(
            [](int x) { std::cout << "Limited Signal: " << x << '\n'; });
        limitedSignal.emit(100);
        limitedSignal.emit(200);
        limitedSignal.emit(300);
        limitedSignal.emit(400);  // 不会被调用
    });
    EXPECT_EQ(
        output,
        "Limited Signal: 100\nLimited Signal: 200\nLimited Signal: 300\n");
}

TEST(SignalTest, DynamicSignal) {
    DynamicSignal<int> dynamicSignal;
    auto slot = std::make_shared<std::function<void(int)>>(
        [](int x) { std::cout << "Dynamic Signal: " << x << '\n'; });

    std::string output1 = captureOutput([&]() {
        dynamicSignal.connect(*slot);
        dynamicSignal.emit(500);
    });
    EXPECT_EQ(output1, "Dynamic Signal: 500\n");

    dynamicSignal.disconnect(*slot);

    std::string output2 = captureOutput([&]() {
        dynamicSignal.emit(600);  // 不会被调用
    });
    EXPECT_EQ(output2, "");
}

TEST(SignalTest, ScopedSignal) {
    ScopedSignal<int> scopedSignal;

    std::string output1 = captureOutput([&]() {
        auto scopedSlot = std::make_shared<std::function<void(int)>>(
            [](int x) { std::cout << "Scoped Signal: " << x << '\n'; });
        scopedSignal.connect(scopedSlot);
        scopedSignal.emit(700);
    });
    EXPECT_EQ(output1, "Scoped Signal: 700\n");

    std::string output2 = captureOutput([&]() { scopedSignal.emit(800); });
    EXPECT_EQ(output2, "Scoped Signal: 800\n");
}
