#include "atom/async/message_bus.hpp"

#include <gtest/gtest.h>
#include <string>
#include <thread>

using namespace atom::async;

// 示例消息类型
struct MyMessage {
    int data;
};

// Test Suite for MessageBus
class MessageBusTest : public ::testing::Test {
protected:
    MessageBus bus;

    void SetUp() override {
        // 可以在每个测试之前设置一些初始状态
    }

    void TearDown() override {
        // 可以在每个测试之后进行清理工作
    }
};

// 测试同步发布和订阅消息
TEST_F(MessageBusTest, SynchronousSubscriptionAndPublishing) {
    MyMessage message{42};
    bool messageReceived = false;

    auto token = bus.subscribe<MyMessage>(
        "testSync",
        [&](const MyMessage& msg) {
            EXPECT_EQ(msg.data, message.data);
            messageReceived = true;
        },
        false);

    bus.publish("testSync", message);

    EXPECT_TRUE(messageReceived);
    bus.unsubscribe<MyMessage>(token);
}

// 测试异步发布和订阅消息
TEST_F(MessageBusTest, AsynchronousSubscriptionAndPublishing) {
    MyMessage message{42};
    std::atomic<bool> messageReceived(false);

    auto token = bus.subscribe<MyMessage>(
        "testAsync",
        [&](const MyMessage& msg) {
            EXPECT_EQ(msg.data, message.data);
            messageReceived = true;
        },
        true);

    bus.publish("testAsync", message);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(50));  // 等待异步执行完成

    EXPECT_TRUE(messageReceived.load());
    bus.unsubscribe<MyMessage>(token);
}

TEST_F(MessageBusTest, PublishGlobalSubscription) {
    std::atomic_int receivedCount = 0;
    MyMessage message{42};
    auto token = bus.subscribe<MyMessage>(
        "testGlobal", [&](const MyMessage& msg) { receivedCount++; });

    auto token1 = bus.subscribe<MyMessage>(
        "testGlobal", [&](const MyMessage& msg) { receivedCount++; });

    bus.publishGlobal(message);
    EXPECT_EQ(receivedCount, 2);
    bus.unsubscribe<MyMessage>(token);
    bus.unsubscribe<MyMessage>(token1);
}

// 测试一次性订阅
TEST_F(MessageBusTest, OnceSubscription) {
    MyMessage message{42};
    int receivedCount = 0;

    auto token = bus.subscribe<MyMessage>(
        "testOnce",
        [&](const MyMessage& msg) {
            EXPECT_EQ(msg.data, message.data);
            receivedCount++;
        },
        false, true);  // once = true

    bus.publish("testOnce", message);
    bus.publish("testOnce", message);  // 第二次发布消息

    EXPECT_EQ(receivedCount, 1);  // 处理程序应仅运行一次
}

// 测试带过滤器的订阅
TEST_F(MessageBusTest, FilteredSubscription) {
    MyMessage message{42};
    MyMessage filteredMessage{100};
    int receivedCount = 0;

    auto token = bus.subscribe<MyMessage>(
        "testFilter",
        [&](const MyMessage& msg) {
            EXPECT_EQ(msg.data, message.data);
            receivedCount++;
        },
        false, false,
        [&](const MyMessage& msg) {
            return msg.data == 42;  // 过滤器只匹配 data == 42 的消息
        });

    bus.publish("testFilter", message);          // 这个消息应该被处理
    bus.publish("testFilter", filteredMessage);  // 这个消息应该被过滤

    EXPECT_EQ(receivedCount, 1);  // 处理程序应仅运行一次
}

// 测试取消订阅
TEST_F(MessageBusTest, Unsubscribe) {
    MyMessage message{42};
    bool messageReceived = false;

    auto token = bus.subscribe<MyMessage>(
        "testUnsubscribe",
        [&](const MyMessage& msg) { messageReceived = true; });

    bus.unsubscribe<MyMessage>(token);
    bus.publish("testUnsubscribe", message);

    EXPECT_FALSE(messageReceived);  // 应该没有接收到消息
}

// 测试命名空间发布
TEST_F(MessageBusTest, NamespaceSubscription) {
    MyMessage message{42};
    bool messageReceived = false;
    auto token = bus.subscribe<MyMessage>(
        "namespace", [&](const MyMessage& msg) { messageReceived = true; });

    bus.publish("namespace.subspace", message);  // 命名空间匹配应接收到消息

    EXPECT_TRUE(messageReceived);
    bus.unsubscribe<MyMessage>(token);
}

// 测试延迟发布
TEST_F(MessageBusTest, DelayedPublishing) {
    MyMessage message{42};
    bool messageReceived = false;

    auto token =
        bus.subscribe<MyMessage>("testDelay", [&](const MyMessage& msg) {
            EXPECT_EQ(msg.data, message.data);
            messageReceived = true;
        });

    bus.publish("testDelay", message, std::chrono::milliseconds(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_FALSE(messageReceived);  // 消息不应在50ms时接收到

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(messageReceived);  // 消息应在150ms时接收到
    bus.unsubscribe<MyMessage>(token);
}

// 测试同时多个订阅者接收消息
TEST_F(MessageBusTest, MultipleSubscribers) {
    MyMessage message{42};
    int receivedCount = 0;

    auto token1 = bus.subscribe<MyMessage>(
        "testMulti", [&](const MyMessage& msg) { receivedCount++; });

    auto token2 = bus.subscribe<MyMessage>(
        "testMulti", [&](const MyMessage& msg) { receivedCount++; });

    bus.publish("testMulti", message);

    // 确保计数器正确反映消息处理的次数
    EXPECT_EQ(receivedCount, 2);  // 应该有两个订阅者接收到消息
    bus.unsubscribe<MyMessage>(token1);
    bus.unsubscribe<MyMessage>(token2);
}

// 测试订阅相同名称但不同类型的消息
TEST_F(MessageBusTest, DifferentMessageTypes) {
    struct AnotherMessage {
        std::string text;
    };

    MyMessage myMessage{42};
    AnotherMessage anotherMessage{"Hello"};
    bool myMessageReceived = false;
    bool anotherMessageReceived = false;

    auto token1 = bus.subscribe<MyMessage>(
        "testTypes", [&](const MyMessage& msg) { myMessageReceived = true; });

    auto token2 = bus.subscribe<AnotherMessage>(
        "testTypes",
        [&](const AnotherMessage& msg) { anotherMessageReceived = true; });

    bus.publish("testTypes", myMessage);
    bus.publish("testTypes", anotherMessage);

    EXPECT_TRUE(myMessageReceived);
    EXPECT_TRUE(anotherMessageReceived);

    bus.unsubscribe<MyMessage>(token1);
    bus.unsubscribe<AnotherMessage>(token2);
}

// 测试消息处理时抛出异常
TEST_F(MessageBusTest, ExceptionInHandler) {
    MyMessage message{42};
    bool exceptionCaught = false;

    auto token =
        bus.subscribe<MyMessage>("testException", [&](const MyMessage& msg) {
            throw std::runtime_error("Handler exception");
        });

    try {
        bus.publish("testException", message);
    } catch (const std::runtime_error& e) {
        exceptionCaught = true;
        EXPECT_STREQ(e.what(), "Handler exception");
    }

    EXPECT_TRUE(exceptionCaught);
    bus.unsubscribe<MyMessage>(token);
}

TEST_F(MessageBusTest, UnsubscribeAllByName) {
    MyMessage message{42};
    bool messageReceived1 = false;
    bool messageReceived2 = false;

    auto token1 = bus.subscribe<MyMessage>(
        "test.unsubscribeAll",
        [&](const MyMessage& msg) { messageReceived1 = true; });

    auto token2 = bus.subscribe<MyMessage>(
        "test.unsubscribeAll",
        [&](const MyMessage& msg) { messageReceived2 = true; });

    bus.unsubscribeAll<MyMessage>("test.unsubscribeAll");
    bus.publish("test.unsubscribeAll", message);

    EXPECT_FALSE(messageReceived1);
    EXPECT_FALSE(messageReceived2);
}

// 测试获取特定名称的订阅者数量
TEST_F(MessageBusTest, GetSubscriberCount) {
    auto token1 =
        bus.subscribe<MyMessage>("test.count", [](const MyMessage& msg) {});
    auto token2 =
        bus.subscribe<MyMessage>("test.count", [](const MyMessage& msg) {});

    EXPECT_EQ(bus.getSubscriberCount<MyMessage>("test.count"), 2);

    bus.unsubscribe<MyMessage>(token1);
    EXPECT_EQ(bus.getSubscriberCount<MyMessage>("test.count"), 1);

    bus.unsubscribe<MyMessage>(token2);
    EXPECT_EQ(bus.getSubscriberCount<MyMessage>("test.count"), 0);
}

// 测试获取命名空间的订阅者数量
TEST_F(MessageBusTest, GetNamespaceSubscriberCount) {
    auto token1 =
        bus.subscribe<MyMessage>("namespace.sub1", [](const MyMessage& msg) {});
    auto token2 =
        bus.subscribe<MyMessage>("namespace.sub2", [](const MyMessage& msg) {});

    EXPECT_EQ(bus.getNamespaceSubscriberCount<MyMessage>("namespace"), 2);

    bus.unsubscribe<MyMessage>(token1);
    EXPECT_EQ(bus.getNamespaceSubscriberCount<MyMessage>("namespace"), 1);

    bus.unsubscribe<MyMessage>(token2);
    EXPECT_EQ(bus.getNamespaceSubscriberCount<MyMessage>("namespace"), 0);
}

// 测试是否存在订阅者
TEST_F(MessageBusTest, HasSubscriber) {
    EXPECT_FALSE(bus.hasSubscriber<MyMessage>("test.exists"));

    auto token =
        bus.subscribe<MyMessage>("test.exists", [](const MyMessage& msg) {});
    EXPECT_TRUE(bus.hasSubscriber<MyMessage>("test.exists"));

    bus.unsubscribe<MyMessage>(token);
    EXPECT_FALSE(bus.hasSubscriber<MyMessage>("test.exists"));
}

// 测试清空所有订阅者
TEST_F(MessageBusTest, ClearAllSubscribers) {
    auto token1 =
        bus.subscribe<MyMessage>("test.clear1", [](const MyMessage& msg) {});
    auto token2 =
        bus.subscribe<MyMessage>("test.clear2", [](const MyMessage& msg) {});

    EXPECT_TRUE(bus.hasSubscriber<MyMessage>("test.clear1"));
    EXPECT_TRUE(bus.hasSubscriber<MyMessage>("test.clear2"));

    bus.clearAllSubscribers();

    EXPECT_FALSE(bus.hasSubscriber<MyMessage>("test.clear1"));
    EXPECT_FALSE(bus.hasSubscriber<MyMessage>("test.clear2"));
}

// 测试获取当前活动的命名空间
TEST_F(MessageBusTest, GetActiveNamespaces) {
    auto token1 =
        bus.subscribe<MyMessage>("namespace1", [](const MyMessage& msg) {});
    auto token2 = bus.subscribe<MyMessage>("namespace2.sub1",
                                           [](const MyMessage& msg) {});

    auto activeNamespaces = bus.getActiveNamespaces();
    EXPECT_EQ(activeNamespaces.size(), 2);
    EXPECT_TRUE(std::find(activeNamespaces.begin(), activeNamespaces.end(),
                          "namespace1") != activeNamespaces.end());
    EXPECT_TRUE(std::find(activeNamespaces.begin(), activeNamespaces.end(),
                          "namespace2.sub1") != activeNamespaces.end());

    bus.clearAllSubscribers();
    activeNamespaces = bus.getActiveNamespaces();
    EXPECT_TRUE(activeNamespaces.empty());
}

// 测试消息历史记录功能
TEST_F(MessageBusTest, MessageHistory) {
    MyMessage message1{42};
    MyMessage message2{84};

    bus.publish("test.history", message1);
    bus.publish("test.history", message2);

    auto history = bus.getMessageHistory<MyMessage>("test.history");

    ASSERT_EQ(history.size(), 2);
    EXPECT_EQ(history[0].data, 42);
    EXPECT_EQ(history[1].data, 84);

    // 测试消息记录超出最大数量时，是否删除旧消息
    for (int i = 0; i < 101; ++i) {
        bus.publish("test.history", MyMessage{i});
    }

    history = bus.getMessageHistory<MyMessage>("test.history");
    EXPECT_EQ(history.size(), 100);  // 最大记录数为100
    EXPECT_EQ(history[0].data, 1);  // 第一个应为第2条消息（0被丢弃）
    EXPECT_EQ(history.back().data, 100);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
