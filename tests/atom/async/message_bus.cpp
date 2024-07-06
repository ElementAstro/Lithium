#include "atom/async/message_bus.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace atom::async;

class MessageBusTest : public ::testing::Test {
protected:
};

// 测试订阅和发布消息
TEST_F(MessageBusTest, SubscribeAndPublish) {
    MessageBus bus;

    bool received = false;
    bus.subscribe<int>("test_topic", [&](const int &msg) {
        received = true;
        EXPECT_EQ(msg, 42);
    });

    bus.publish("test_topic", 42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    EXPECT_TRUE(received);
}

// 测试取消订阅
TEST_F(MessageBusTest, Unsubscribe) {
    MessageBus bus;

    bool received = false;
    auto callback = [&](const int &msg) { received = true; };

    bus.subscribe<int>("test_topic", callback);
    bus.unsubscribe<int>("test_topic", callback);

    bus.publish("test_topic", 42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    EXPECT_FALSE(received);
}

// 测试全局订阅和发布消息
TEST_F(MessageBusTest, GlobalSubscribeAndPublish) {
    MessageBus bus;

    bool received = false;
    bus.globalSubscribe<int>([&](const int &msg) {
        received = true;
        EXPECT_EQ(msg, 42);
    });

    bus.publish("any_topic", 42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    EXPECT_TRUE(received);
}

// 测试尝试发布消息
TEST_F(MessageBusTest, TryPublish) {
    MessageBus bus;

    bool received = false;
    bus.subscribe<int>("test_topic", [&](const int &msg) {
        received = true;
        EXPECT_EQ(msg, 42);
    });

    bool published =
        bus.tryPublish("test_topic", 42, "", std::chrono::milliseconds(100));
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    EXPECT_TRUE(published);
    EXPECT_TRUE(received);
}

// 测试尝试接收消息
TEST_F(MessageBusTest, TryReceive) {
    MessageBus bus;

    bus.publish("test_topic", 42);

    int receivedMessage = 0;
    bool received =
        bus.tryReceive(receivedMessage, std::chrono::milliseconds(100));

    EXPECT_TRUE(received);
    EXPECT_EQ(receivedMessage, 42);
}

// 测试处理线程的启动和停止
TEST_F(MessageBusTest, StartAndStopProcessingThread) {
    MessageBus bus;
    bool received = false;

    bus.startProcessingThread<int>();
    bus.subscribe<int>("test_topic", [&](const int &msg) {
        received = true;
        EXPECT_EQ(msg, 42);
    });

    bus.publish("test_topic", 42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    EXPECT_TRUE(received);

    bus.stopProcessingThread<int>();
}

// 测试多线程环境下的消息发布和订阅
TEST_F(MessageBusTest, MultiThreadedPublishAndSubscribe) {
    MessageBus bus;
    std::atomic<int> receivedCount{0};

    bus.startProcessingThread<int>();
    bus.subscribe<int>("test_topic", [&](const int &msg) {
        receivedCount++;
        EXPECT_EQ(msg, 42);
    });

    std::thread publisher1([&bus]() {
        for (int i = 0; i < 10; ++i) {
            bus.publish("test_topic", 42);
        }
    });

    std::thread publisher2([&bus]() {
        for (int i = 0; i < 10; ++i) {
            bus.publish("test_topic", 42);
        }
    });

    publisher1.join();
    publisher2.join();

    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理
    EXPECT_EQ(receivedCount, 20);

    bus.stopProcessingThread<int>();
}