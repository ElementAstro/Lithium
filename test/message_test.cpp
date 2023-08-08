#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <any>
#include <queue>
#include <atomic>
#include <thread>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include "loguru/loguru.hpp"

class MessageBus
{
public:
    template <typename T>
    void Subscribe(const std::string &topic, std::function<void(const T &)> callback, int priority = 0)
    {
        subscribersLock_.lock();
        subscribers_[topic].push_back({priority, std::any(callback)});
        std::sort(subscribers_[topic].begin(), subscribers_[topic].end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.first > b.first;
                  });
        subscribersLock_.unlock();

        LOG_F(INFO, "Subscribed to topic: %s", topic.c_str());
    }

    template <typename T>
    void Unsubscribe(const std::string &topic, std::function<void(const T &)> callback)
    {
        subscribersLock_.lock();
        auto it = subscribers_.find(topic);
        if (it != subscribers_.end())
        {
            auto &topicSubscribers = it->second;
            topicSubscribers.erase(
                std::remove_if(
                    topicSubscribers.begin(), topicSubscribers.end(),
                    [&](const auto &subscriber)
                    {
                        return subscriber.second.type() == typeid(callback);
                    }),
                topicSubscribers.end());

            LOG_F(INFO, "Unsubscribed from topic: %s", topic.c_str());
        }
        subscribersLock_.unlock();
    }

    template <typename T>
    void Publish(const std::string &topic, const T &message)
    {
        messageQueueLock_.lock();
        messageQueue_.push({topic, std::any(message)});
        messageQueueLock_.unlock();
        messageAvailableFlag_.notify_one();

        LOG_F(INFO, "Published message to topic: %s", topic.c_str());
    }

    template <typename T>
    void GlobalSubscribe(std::function<void(const T &)> callback)
    {
        globalSubscribersLock_.lock();
        globalSubscribers_.push_back({std::any(callback)});
        globalSubscribersLock_.unlock();
    }

    template <typename T>
    void GlobalUnsubscribe(std::function<void(const T &)> callback)
    {
        globalSubscribersLock_.lock();
        globalSubscribers_.erase(
            std::remove_if(
                globalSubscribers_.begin(), globalSubscribers_.end(),
                [&](const auto &subscriber)
                {
                    return subscriber.type() == typeid(callback);
                }),
            globalSubscribers_.end());
        globalSubscribersLock_.unlock();
    }

    template <typename T>
    void StartProcessingThread()
    {
        processingThread_ = std::thread([&]()
                                        {
            while (isRunning_.load()) {
                std::pair<std::string, std::any> message;
                bool hasMessage = false;

                while (isRunning_.load()) {
                    messageQueueLock_.lock();
                    if (!messageQueue_.empty()) {
                        message = std::move(messageQueue_.front());
                        messageQueue_.pop();
                        hasMessage = true;
                    }
                    messageQueueLock_.unlock();

                    if (hasMessage) {
                        break;
                    }

                    std::unique_lock<std::mutex> lock(waitingMutex_);
                    messageAvailableFlag_.wait(lock);
                }

                if (hasMessage) {
                    const std::string& topic = message.first;
                    const std::any& data = message.second;

                    subscribersLock_.lock();
                    auto it = subscribers_.find(topic);
                    if (it != subscribers_.end()) {
                        try {
                            for (const auto& subscriber : it->second) {
                                if (subscriber.second.type() == typeid(std::function<void(const T&)>)) {
                                    std::any_cast<std::function<void(const T&)>>(subscriber.second)(std::any_cast<const T&>(data));
                                }
                            }
                        } catch (const std::bad_any_cast& e) {
                            LOG_F(ERROR, "Message type mismatch: %s", e.what());
                        } catch (...) {
                            LOG_F(ERROR, "Unknown error occurred during message processing");
                        }
                    }
                    subscribersLock_.unlock();

                    globalSubscribersLock_.lock();
                    try {
                        for (const auto& subscriber : globalSubscribers_) {
                            if (subscriber.type() == typeid(std::function<void(const T&)>)) {
                                std::any_cast<std::function<void(const T&)>>(subscriber)(std::any_cast<const T&>(data));
                            }
                        }
                    } catch (const std::bad_any_cast& e) {
                        LOG_F(ERROR, "Global message type mismatch: %s", e.what());
                    } catch (...) {
                        LOG_F(ERROR, "Unknown error occurred during global message processing");
                    }
                    globalSubscribersLock_.unlock();

                    LOG_F(INFO, "Processed message on topic: %s", topic.c_str());
                }
            } });
    }

    void StopProcessingThread()
    {
        isRunning_.store(false);
        messageAvailableFlag_.notify_one();

        if (processingThread_.joinable())
        {
            processingThread_.join();
            LOG_F(INFO, "Processing thread stopped");
        }
    }

private:
    std::unordered_map<std::string, std::vector<std::pair<int, std::any>>> subscribers_;
    std::mutex subscribersLock_;
    std::queue<std::pair<std::string, std::any>> messageQueue_;
    std::mutex messageQueueLock_;
    std::condition_variable messageAvailableFlag_;
    std::mutex waitingMutex_;
    std::thread processingThread_;
    std::atomic<bool> isRunning_{true};

    std::vector<std::any> globalSubscribers_;
    std::mutex globalSubscribersLock_;
};

class TestSubscriber
{
public:
    void Callback(const std::string &message)
    {
        LOG_F(INFO, "Test Subscriber: %s", message.c_str());
    }
};

int main()
{

    // 创建MessageBus对象
    MessageBus messageBus;

    // 创建一个测试订阅者对象
    TestSubscriber testSubscriber;
    
    messageBus.StartProcessingThread<std::string>();

    // 订阅主题为 "topic1" 的消息，并设置优先级为0
    messageBus.Subscribe<std::string>(
        "topic1", [&](const std::string &message)
        { LOG_F(INFO, "Subscriber 1: %s", message.c_str()); },
        0);

    // 订阅主题为 "topic1" 的消息，并设置优先级为1
    messageBus.Subscribe<std::string>(
        "topic1", [&](const std::string &message)
        { LOG_F(INFO, "Subscriber 2: %s", message.c_str()); },
        1);

    // 订阅主题为 "topic2" 的消息，并设置优先级为0
    messageBus.Subscribe<std::string>(
        "topic2", [&](const std::string &message)
        { LOG_F(INFO, "Subscriber 3: %s", message.c_str()); },
        0);

    // 全局订阅std::string类型的消息，并使用testSubscriber的成员函数Callback作为回调函数
    messageBus.GlobalSubscribe<std::string>(std::bind(&TestSubscriber::Callback, &testSubscriber, std::placeholders::_1));

    // 发布一条消息到主题 "topic1"
    messageBus.Publish<std::string>("topic1", "Hello, topic1!");

    // 发布一条消息到主题 "topic2"
    messageBus.Publish<std::string>("topic2", "Hello, topic2!");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 停止MessageBus的处理线程
    messageBus.StopProcessingThread();

    // 关闭日志库
    loguru::shutdown();

    return 0;
}
