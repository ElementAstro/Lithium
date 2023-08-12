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
    void Subscribe(const std::string &topic, std::function<void(const T &)> callback, int priority = 0, const std::string &namespace_ = "")
    {
        std::string fullTopic = namespace_.empty() ? topic : (namespace_ + "::" + topic);

        subscribersLock_.lock();
        subscribers_[fullTopic].push_back({priority, std::any(callback)});
        std::sort(subscribers_[fullTopic].begin(), subscribers_[fullTopic].end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.first > b.first;
                  });
        subscribersLock_.unlock();

        LOG_F(INFO, "Subscribed to topic: %s", fullTopic.c_str());
    }

    template <typename T>
    void SubscribeToNamespace(const std::string &namespaceName, std::function<void(const T &)> callback, int priority = 0)
    {
        std::string topic = namespaceName + ".*";
        Subscribe<T>(topic, callback, priority, namespaceName);
    }

    template <typename T>
    void Unsubscribe(const std::string &topic, std::function<void(const T &)> callback, const std::string &namespace_ = "")
    {
        std::string fullTopic = namespace_.empty() ? topic : (namespace_ + "::" + topic);

        subscribersLock_.lock();
        auto it = subscribers_.find(fullTopic);
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

            LOG_F(INFO, "Unsubscribed from topic: %s", fullTopic.c_str());
        }
        subscribersLock_.unlock();
    }

    template <typename T>
    void Publish(const std::string &topic, const T &message, const std::string &namespace_ = "")
    {
        std::string fullTopic = namespace_.empty() ? topic : (namespace_ + "::" + topic);

        messageQueueLock_.lock();
        messageQueue_.push({fullTopic, std::any(message)});
        messageQueueLock_.unlock();
        messageAvailableFlag_.notify_one();

        LOG_F(INFO, "Published message to topic: %s", fullTopic.c_str());
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
        processingThread_ = std::jthread([&]()
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
            processingThread_.request_stop();
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
    std::jthread processingThread_;
    std::atomic<bool> isRunning_{true};

    std::vector<std::any> globalSubscribers_;
    std::mutex globalSubscribersLock_;
};

void callback1(const std::string &message)
{
    LOG_F(INFO, "Callback 1: %s", message.c_str());
}

void callback2(const std::string &message)
{
    LOG_F(INFO, "Callback 2: %s", message.c_str());
}

void callback3(const std::string &message)
{
    LOG_F(INFO, "Callback 3: %s", message.c_str());
}

int main()
{
    // 创建 MessageBus 对象
    MessageBus messageBus;

    // 定义回调函数

    messageBus.StartProcessingThread<std::string>();

    // 订阅主题 'topic1' 的回调函数 callback1，属于命名空间 'NamespaceA'
    messageBus.Subscribe<std::string>("topic1", callback1, 0, "NamespaceA");

    // 订阅主题 'topic1' 的回调函数 callback2，属于命名空间 'NamespaceB'
    messageBus.Subscribe<std::string>("topic1", callback2, 0, "NamespaceB");

    // 订阅主题 'topic2' 的回调函数 callback3，属于命名空间 'NamespaceB'
    messageBus.Subscribe<std::string>("topic2", callback3, 0, "NamespaceB");

    // 订阅命名空间 'NamespaceA' 的所有主题的回调函数 callback1
    messageBus.SubscribeToNamespace<std::string>("NamespaceA", callback1);

    // 订阅命名空间 'NamespaceB' 的所有主题的回调函数 callback2
    messageBus.SubscribeToNamespace<std::string>("NamespaceB", callback2);

    // 发布消息到命名空间 'NamespaceA' 的主题 'topic1'
    messageBus.Publish("topic1", std::string("Message for topic1 in NamespaceA"), "NamespaceA");
    // 输出:
    // Callback 1: Message for topic1 in NamespaceA

    // 发布消息到命名空间 'NamespaceB' 的主题 'topic1'
    messageBus.Publish("topic1", std::string("Message for topic1 in NamespaceB"), "NamespaceB");
    // 输出:
    // Callback 2: Message for topic1 in NamespaceB

    // 发布消息到命名空间 'NamespaceB' 的主题 'topic2'
    messageBus.Publish("topic2", std::string("Message for topic2 in NamespaceB"), "NamespaceB");
    // 输出:
    // Callback 3: Message for topic2 in NamespaceB

    // 取消订阅命名空间 'NamespaceB' 的主题 'topic1' 的 callback2
    messageBus.Unsubscribe<std::string>("topic1", callback2, "NamespaceB");

    // 再次发布消息到命名空间 'NamespaceB' 的主题 'topic1'
    messageBus.Publish("topic1", std::string("Another message for topic1 in NamespaceB"), "NamespaceB");
    // 输出为空，因为 callback2 已被取消订阅

    // 停止消息处理线程
    messageBus.StopProcessingThread();

    return 0;
}
