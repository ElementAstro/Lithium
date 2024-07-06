#include "atom/async/message_queue.hpp"
#include <gtest/gtest.h>


using namespace atom::async;

class MessageQueueTest : public ::testing::Test {};

// 测试订阅和发布消息
TEST_F(MessageQueueTest, SubscribeAndPublish) {
    MessageQueue<int> queue;

    bool received = false;
    queue.subscribe(
        [&](const int &msg) {
            received = true;
            EXPECT_EQ(msg, 42);
        },
        "test_subscriber");

    queue.publish(42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理
    EXPECT_TRUE(received);
}

// 测试取消订阅
TEST_F(MessageQueueTest, Unsubscribe) {
    MessageQueue<int> queue;

    bool received = false;
    auto callback = [&](const int &msg) { received = true; };

    queue.subscribe(callback, "test_subscriber");
    queue.unsubscribe(callback);

    queue.publish(42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    EXPECT_FALSE(received);
}

// 测试多线程环境下的消息发布和订阅
TEST_F(MessageQueueTest, MultiThreadedPublishAndSubscribe) {
    MessageQueue<int> queue;
    std::atomic<int> receivedCount{0};

    queue.startProcessingThread();
    queue.subscribe(
        [&](const int &msg) {
            receivedCount++;
            EXPECT_EQ(msg, 42);
        },
        "test_subscriber");

    std::thread publisher1([&queue]() {
        for (int i = 0; i < 10; ++i) {
            queue.publish(42);
        }
    });

    std::thread publisher2([&queue]() {
        for (int i = 0; i < 10; ++i) {
            queue.publish(42);
        }
    });

    publisher1.join();
    publisher2.join();

    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理
    EXPECT_EQ(receivedCount.load(), 20);

    queue.stopProcessingThread();
}

// 测试获取消息数量
TEST_F(MessageQueueTest, GetMessageCount) {
    MessageQueue<int> queue;

    EXPECT_EQ(queue.getMessageCount(), 0);

    queue.publish(42);
    EXPECT_EQ(queue.getMessageCount(), 1);

    queue.publish(43);
    EXPECT_EQ(queue.getMessageCount(), 2);
}

// 测试获取订阅者数量
TEST_F(MessageQueueTest, GetSubscriberCount) {
    MessageQueue<int> queue;

    EXPECT_EQ(queue.getSubscriberCount(), 0);

    queue.subscribe([](const int &msg) {}, "subscriber1");
    EXPECT_EQ(queue.getSubscriberCount(), 1);

    queue.subscribe([](const int &msg) {}, "subscriber2");
    EXPECT_EQ(queue.getSubscriberCount(), 2);
}

// 测试优先级订阅
TEST_F(MessageQueueTest, PrioritySubscribe) {
    MessageQueue<int> queue;
    std::vector<int> receivedMessages;

    queue.subscribe([&](const int &msg) { receivedMessages.push_back(1); },
                    "subscriber1", 1);

    queue.subscribe([&](const int &msg) { receivedMessages.push_back(2); },
                    "subscriber2", 2);

    queue.publish(42);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 等待消息处理

    ASSERT_EQ(receivedMessages.size(), 2);
    EXPECT_EQ(receivedMessages[0], 2);  // subscriber2 has higher priority
    EXPECT_EQ(receivedMessages[1], 1);  // subscriber1 has lower priority
}