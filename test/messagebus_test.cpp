#include <atomic>
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <any>
#include <queue>
#include <mutex>
#include <thread>
#include <algorithm>
#include <condition_variable>
#include "loguru/loguru.hpp"

class MessageBus
{
public:
    template <typename T>
    void Subscribe(const std::string &topic, std::function<void(const T &)> callback, int priority = 0)
    {
        std::lock_guard<std::mutex> lock(subscribersLock_);

        if (topic.empty())
        {
            globalSubscribers_.push_back({priority, std::make_any<std::function<void(const T &)>>(callback)});
            std::sort(globalSubscribers_.begin(), globalSubscribers_.end(),
                      [](const auto &a, const auto &b)
                      {
                          return a.first > b.first;
                      });
        }
        else
        {
            subscribers_[topic].push_back({priority, std::make_any<std::function<void(const T &)>>(callback)});
            std::sort(subscribers_[topic].begin(), subscribers_[topic].end(),
                      [](const auto &a, const auto &b)
                      {
                          return a.first > b.first;
                      });
        }

        LOG_F(INFO, "Subscribed to topic: %s", topic.c_str());
    }

    template <typename T>
    void Unsubscribe(const std::string &topic, std::function<void(const T &)> callback)
    {
        std::lock_guard<std::mutex> lock(subscribersLock_);

        if (topic.empty())
        {
            globalSubscribers_.erase(
                std::remove_if(
                    globalSubscribers_.begin(), globalSubscribers_.end(),
                    [&](const auto &subscriber)
                    {
                        if (subscriber.second.type() == typeid(std::function<void(const T &)>))
                        {
                            return callback.target<std::function<void(const T &)>>() ==
                                   std::any_cast<std::function<void(const T &)>>(
                                       subscriber.second)
                                       .target<std::function<void(const T &)>>();
                        }
                        return false;
                    }),
                globalSubscribers_.end());
        }
        else
        {
            auto it = subscribers_.find(topic);
            if (it != subscribers_.end())
            {
                auto &topicSubscribers = it->second;
                topicSubscribers.erase(
                    std::remove_if(
                        topicSubscribers.begin(), topicSubscribers.end(),
                        [&](const auto &subscriber)
                        {
                            if (subscriber.second.type() == typeid(std::function<void(const T &)>))
                            {
                                return callback.target<std::function<void(const T &)>>() ==
                                       std::any_cast<std::function<void(const T &)>>(
                                           subscriber.second)
                                           .target<std::function<void(const T &)>>();
                            }
                            return false;
                        }),
                    topicSubscribers.end());
            }
        }
    }

    template <typename T>
    void Publish(const std::string &topic, const T &message)
    {
        std::lock_guard<std::mutex> lock(messageQueueLock_);

        if (topic.empty())
        {
            for (const auto &subscriber : globalSubscribers_)
            {
                if (subscriber.second.type() == typeid(std::function<void(const T &)>))
                {
                    std::any_cast<std::function<void(const T &)>>(subscriber.second)(message);
                }
            }
        }
        else
        {
            auto it = subscribers_.find(topic);
            if (it != subscribers_.end())
            {
                const auto &topicSubscribers = it->second;
                for (const auto &subscriber : topicSubscribers)
                {
                    if (subscriber.second.type() == typeid(std::function<void(const T &)>))
                    {
                        std::any_cast<std::function<void(const T &)>>(subscriber.second)(message);
                    }
                }
            }
        }
    }

    void StartProcessingThread()
    {
        processingThread_ = std::thread([&]()
                                        {
            while (isRunning_.load()) {
                std::pair<std::string, std::any> message;
                bool hasMessage = false;

                while (isRunning_.load()) {
                    std::unique_lock<std::mutex> lock(messageQueueLock_);
                    if (!messageQueue_.empty()) {
                        message = std::move(messageQueue_.front());
                        messageQueue_.pop();
                        hasMessage = true;
                    }
                    lock.unlock();

                    if (hasMessage) {
                        break;
                    }

                    std::unique_lock<std::mutex> waitingLock(waitingMutex_);
                    messageAvailableFlag_.wait(waitingLock);
                }

                if (hasMessage) {
                    const std::string& topic = message.first;
                    const std::any& data = message.second;

                    std::lock_guard<std::mutex> lock(subscribersLock_);
                    if (topic.empty()) {
                        ProcessSubscribers(globalSubscribers_, data);
                    } else {
                        auto it = subscribers_.find(topic);
                        if (it != subscribers_.end()) {
                            ProcessSubscribers(it->second, data);
                        }
                    }
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
    std::vector<std::pair<int, std::any>> globalSubscribers_;
    std::mutex subscribersLock_;
    std::queue<std::pair<std::string, std::any>> messageQueue_;
    std::mutex messageQueueLock_;
    std::condition_variable messageAvailableFlag_;
    std::mutex waitingMutex_;
    std::thread processingThread_;
    std::atomic<bool> isRunning_{true};

    template <typename T>
    void ProcessSubscribers(const std::vector<std::pair<int, std::any>> &subscribers, const std::any &data)
    {
        try
        {
            for (const auto &subscriber : subscribers)
            {
                if (subscriber.second.type() == typeid(std::function<void(const T &)>))
                {
                    std::any_cast<std::function<void(const T &)>>(subscriber.second)(std::any_cast<const T &>(data));
                }
            }
        }
        catch (const std::bad_any_cast &e)
        {
            LOG_F(ERROR, "Message type mismatch: %s", e.what());
        }
        catch (...)
        {
            LOG_F(ERROR, "Unknown error occurred during message processing");
        }
    }
};

class OtherClass
{
public:
    void OnMyMessageReceived(const std::string &message)
    {
        try
        {
            // 处理接收到的消息
            LOG_F(INFO, "OtherClass received message with content: %s", message.c_str());
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Exception caught in OnMyMessageReceived: %s", e.what());
        }
        catch (...)
        {
            LOG_F(ERROR, "Unknown exception caught in OnMyMessageReceived");
        }
    }
};

void GlobalCallback(const std::string &message)
{
    std::cout << "Received global message: " << message << std::endl;
}

void LocalCallback(const std::string &message)
{
    std::cout << "Received local message: " << message << std::endl;
}

int main()
{
    MessageBus messageBus;

    // 订阅全局主题
    messageBus.Subscribe<std::string>("", GlobalCallback);

    // 订阅局部主题
    messageBus.Subscribe<std::string>("topic1", LocalCallback);

    // 发布全局消息
    messageBus.Publish("", "Hello, global!");

    // 发布局部消息
    messageBus.Publish("topic1", "Hello, local!");

    // 取消订阅全局主题
    messageBus.Unsubscribe<std::string>("", GlobalCallback);

    // 取消订阅局部主题
    messageBus.Unsubscribe<std::string>("topic1", LocalCallback);

    // 再次发布消息，但由于没有订阅者，将不会有输出
    messageBus.Publish("", "This message won't be received");
    messageBus.Publish("topic1", "This message won't be received either");

    return 0;
}
