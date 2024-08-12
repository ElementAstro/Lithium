#include "atom/async/message_bus.hpp"
#include <gtest/gtest.h>
#include <string>

// 使用的命名空间
using namespace atom::async;

// 测试套件
class MessageBusTest : public ::testing::Test {
protected:
    void SetUp() override { bus = MessageBus::createUnique(); }

    std::unique_ptr<MessageBus> bus;
};

// 测试基本的订阅和发布
TEST_F(MessageBusTest, BasicSubscribeAndPublish) {
    std::string receivedMessage;
    bus->subscribe<std::string>(
        "test_topic",
        [&receivedMessage](const std::string &msg) { receivedMessage = msg; });

    bus->publish("test_topic", std::string("Hello, World!"));

    // 使用独立线程处理消息
    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(receivedMessage, "Hello, World!");
    bus->stopAllProcessingThreads();
}

// 测试多个订阅者的优先级
TEST_F(MessageBusTest, MultipleSubscribersWithPriority) {
    std::vector<int> receivedPriorities;
    bus->subscribe<std::string>(
        "test_topic",
        [&receivedPriorities](const std::string &msg) {
            receivedPriorities.push_back(1);
        },
        1);
    bus->subscribe<std::string>(
        "test_topic",
        [&receivedPriorities](const std::string &msg) {
            receivedPriorities.push_back(2);
        },
        2);
    bus->subscribe<std::string>(
        "test_topic",
        [&receivedPriorities](const std::string &msg) {
            receivedPriorities.push_back(0);
        },
        0);

    bus->publish("test_topic", std::string("Test"));

    // 使用独立线程处理消息
    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<int> expectedPriorities = {2, 1, 0};
    EXPECT_EQ(receivedPriorities, expectedPriorities);
    bus->stopAllProcessingThreads();
}

// 测试取消订阅
TEST_F(MessageBusTest, Unsubscribe) {
    std::string receivedMessage;
    auto callback = [&receivedMessage](const std::string &msg) {
        receivedMessage = msg;
    };
    bus->subscribe<std::string>("test_topic", callback);
    bus->unsubscribe<std::string>("test_topic", callback);

    bus->publish("test_topic", std::string("This should not be received"));

    // 使用独立线程处理消息
    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(receivedMessage, "");
    bus->stopAllProcessingThreads();
}

// 测试命名空间订阅
TEST_F(MessageBusTest, SubscribeToNamespace) {
    std::string receivedMessage;
    bus->subscribeToNamespace<std::string>(
        "namespace",
        [&receivedMessage](const std::string &msg) { receivedMessage = msg; });

    bus->publish("test_topic", std::string("Test message"), "namespace");

    // 使用独立线程处理消息
    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(receivedMessage, "Test message");
    bus->stopAllProcessingThreads();
}

// 测试全局订阅
TEST_F(MessageBusTest, GlobalSubscribe) {
    std::string receivedMessage;
    bus->globalSubscribe<std::string>(
        [&receivedMessage](const std::string &msg) { receivedMessage = msg; });

    bus->publish("any_topic", std::string("Global message"));

    // 使用独立线程处理消息
    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(receivedMessage, "Global message");
    bus->stopAllProcessingThreads();
}

// 测试消息队列大小限制
TEST_F(MessageBusTest, MessageQueueSizeLimit) {
    bus = MessageBus::createUnique();
    bus->publish("test_topic", std::string("Message 1"));
    bus->publish("test_topic", std::string("Message 2"));
    bus->publish("test_topic", std::string("Message 3"));

    std::string receivedMessage;
    bus->subscribe<std::string>(
        "test_topic",
        [&receivedMessage](const std::string &msg) { receivedMessage = msg; });

    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(receivedMessage, "Message 3");
    bus->stopAllProcessingThreads();
}

// 测试 tryPublish 方法
TEST_F(MessageBusTest, TryPublish) {
    std::string receivedMessage;
    bus->subscribe<std::string>(
        "test_topic",
        [&receivedMessage](const std::string &msg) { receivedMessage = msg; });

    bool published = bus->tryPublish("test_topic", std::string("Try message"),
                                     "", std::chrono::milliseconds(100));

    // 使用独立线程处理消息
    bus->startProcessingThread<std::string>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(published);
    EXPECT_EQ(receivedMessage, "Try message");
    bus->stopAllProcessingThreads();
}

// 测试 tryReceive 方法
TEST_F(MessageBusTest, TryReceive) {
    std::string sentMessage = "Hello, TryReceive!";
    bus->publish("test_topic", sentMessage);

    std::string receivedMessage;
    bool received = bus->tryReceive<std::string>(
        receivedMessage, std::chrono::milliseconds(100));

    EXPECT_TRUE(received);
    EXPECT_EQ(receivedMessage, sentMessage);
}

// 测试停止所有处理线程
TEST_F(MessageBusTest, StopAllProcessingThreads) {
    bus->startProcessingThread<std::string>();
    bus->startProcessingThread<int>();

    bus->stopAllProcessingThreads();
}
