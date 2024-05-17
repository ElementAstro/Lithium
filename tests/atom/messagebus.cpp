#include <gtest/gtest.h>
#include "../../src/atom/server/message_bus.hpp"

using namespace Lithium;

class MessageBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Test setup code
    }

    void TearDown() override
    {
        // Test teardown code
    }

    // Helper functions
};

TEST_F(MessageBusTest, SubscribeToTopic)
{
    MessageBus messageBus;
    std::string topic = "test_topic";
    std::function<void(int)> callback = [](int message)
    {
        // Test callback code
    };
    int priority = 0;
    std::string namespace_ = "";

    messageBus.Subscribe<int>(topic, callback, priority, namespace_);
}

TEST_F(MessageBusTest, UnsubscribeFromTopic)
{
    MessageBus messageBus;
    std::string topic = "test_topic";
    std::function<void(int)> callback = [](int message)
    {
        // Test callback code
    };
    int priority = 0;
    std::string namespace_ = "";

    messageBus.Subscribe<int>(topic, callback, priority, namespace_);

    messageBus.Publish<int>(topic, 123);

    messageBus.Unsubscribe<int>(topic, callback, namespace_);
}

TEST_F(MessageBusTest, PublishMessageToTopic)
{
    MessageBus messageBus;
    std::string topic = "test_topic";
    std::function<void(int)> callback = [](int message)
    {
        // Test callback code
    };
    int priority = 0;
    std::string namespace_ = "";

    messageBus.Subscribe<int>(topic, callback, priority, namespace_);

    int message = 123;
    messageBus.Publish<int>(topic, message);

    // Test if the published message is received by the callback
    // ASSERT_XXXX
}

TEST_F(MessageBusTest, GlobalSubscribe)
{
    MessageBus messageBus;
    std::function<void(int)> callback = [](int message)
    {
        // Test callback code
    };
    int priority = 0;

    messageBus.GlobalSubscribe<int>(callback);
}

TEST_F(MessageBusTest, GlobalUnsubscribe)
{
    MessageBus messageBus;
    std::function<void(int)> callback = [](int message)
    {
        // Test callback code
    };
    int priority = 0;

    messageBus.GlobalSubscribe<int>(callback);

    messageBus.GlobalUnsubscribe<int>(callback);
}

TEST_F(MessageBusTest, StartProcessingThread)
{
    MessageBus messageBus;
    std::type_index typeIndex = typeid(int);
#if __cplusplus >= 202002L
    std::thread thread = std::thread([&]()
                                     { messageBus.StartProcessingThread<int>(); });
    thread.detach();
#else
    std::thread thread([&]()
                       { messageBus.StartProcessingThread<int>(); });
    thread.detach();
#endif

    // Test if the processing thread is started successfully
    // ASSERT_XXXX
}

TEST_F(MessageBusTest, StopProcessingThread)
{
    MessageBus messageBus;
    std::type_index typeIndex = typeid(int);
    std::thread thread;
#if __cplusplus >= 202002L
    std::thread::id tid = std::this_thread::get_id();
    thread = std::thread([&]()
                         {
        messageBus.StartProcessingThread<int>();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        messageBus.StopProcessingThread<int>(); });
    thread.detach();
    while (std::this_thread::get_id() != tid)
    {
        std::this_thread::yield();
    }
#else
    std::thread::id tid = std::this_thread::get_id();
    thread = std::thread([&]()
                         {
        messageBus.StartProcessingThread<int>();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        messageBus.StopProcessingThread<int>(); });
    thread.detach();
    while (std::this_thread::get_id() != tid)
    {
        std::this_thread::yield();
    }
#endif

    // Test if the processing thread is stopped successfully
    // ASSERT_XXXX
}

TEST_F(MessageBusTest, StopAllProcessingThreads)
{
    MessageBus messageBus;
    std::type_index typeIndex = typeid(int);
    std::thread thread;
#if __cplusplus >= 202002L
    std::thread::id tid = std::this_thread::get_id();
    thread = std::thread([&]()
                         {
        messageBus.StartProcessingThread<int>();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        messageBus.StopAllProcessingThreads(); });
    thread.detach();
    while (std::this_thread::get_id() != tid)
    {
        std::this_thread::yield();
    }
#else
    std::thread::id tid = std::this_thread::get_id();
    thread = std::thread([&]()
                         {
        messageBus.StartProcessingThread<int>();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        messageBus.StopAllProcessingThreads(); });
    thread.detach();
    while (std::this_thread::get_id() != tid)
    {
        std::this_thread::yield();
    }
#endif

    // Test if all processing threads are stopped successfully
    // ASSERT_XXXX
}
