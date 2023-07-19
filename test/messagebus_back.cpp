#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <string>
#include <condition_variable>
#include <queue>
#include <any>
#include <algorithm>

class MessageBus
{
public:
    template <typename T>
    void Subscribe(const std::string &topic, std::function<void(const T &)> callback, int priority = 0)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[topic].push_back(std::make_pair(priority, std::any(callback)));

        std::sort(subscribers_[topic].begin(), subscribers_[topic].end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.first > b.first;
                  });
    }

    template <typename T>
    void Unsubscribe(const std::string &topic, std::function<void(const T &)> callback)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(topic);
        if (it != subscribers_.end())
        {
            auto &topicSubscribers = it->second;
            topicSubscribers.erase(
                std::remove_if(topicSubscribers.begin(), topicSubscribers.end(),
                               [&](const auto &subscriber)
                               {
                                   return subscriber.second.type() == typeid(callback);
                               }),
                topicSubscribers.end());
        }
    }

    template <typename T>
    void Publish(const std::string &topic, const T &message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        messageQueue_.push(std::make_pair(topic, std::any(message)));
        conditionVariable_.notify_one();
    }

    template <typename T>
    void StartProcessingThread()
    {
        processingThread_ = std::thread([&]()
                                        {
                while (isRunning_)
                {
                    std::pair<std::string, std::any> message;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        conditionVariable_.wait(lock, [&]() {
                            return !messageQueue_.empty() || !isRunning_;
                        });

                        if (!isRunning_) {
                            return;
                        }

                        message = std::move(messageQueue_.front());
                        messageQueue_.pop();
                    }

                    const std::string &topic = message.first;
                    const std::any &data = message.second;
                    auto it = subscribers_.find(topic);
                    if (it != subscribers_.end())
                    {
                        for (const auto &subscriber : it->second)
                        {
                            if (subscriber.second.type() == typeid(std::function<void(const T &)>))
                            {
                                std::any_cast<std::function<void(const T &)>>(subscriber.second)(std::any_cast<const T &>(data));
                            }
                        }
                    }
                } });
    }

    void StopProcessingThread()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            isRunning_ = false;
        }

        conditionVariable_.notify_one();

        if (processingThread_.joinable())
        {
            processingThread_.join();
        }
    }

private:
    std::unordered_map<std::string, std::vector<std::pair<int, std::any>>> subscribers_;
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::queue<std::pair<std::string, std::any>> messageQueue_;
    bool isRunning_ = true;
    std::thread processingThread_;
};

// 示例用法
struct MyMessage
{
    std::string content;
};

// 生成指定长度的随机字符串
std::string GenerateRandomString(int length)
{
    static const char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static const int numCharacters = sizeof(characters) - 1;
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> distribution(0, numCharacters - 1);

    std::string randomString;
    randomString.reserve(length);
    for (int i = 0; i < length; ++i)
    {
        randomString += characters[distribution(rng)];
    }

    return randomString;
}

class OtherClass
{
public:
    void OnMyMessageReceived(const std::string &message)
    {
        // 处理接收到的消息
        std::cout << "OtherClass received message with content: " << message << std::endl;
    }
};

int main()
{
    MessageBus messageBus;
    OtherClass otherObject;

    messageBus.Subscribe<std::string>("MyTopic", std::bind(&OtherClass::OnMyMessageReceived, &otherObject, std::placeholders::_1));

    messageBus.StartProcessingThread<std::string>();

    std::thread publisher([&]()
                          {
            for (int i = 0; i < 10; ++i)
            {
                std::string message = GenerateRandomString(10);
                messageBus.Publish("MyTopic", message);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } });

    publisher.join();

    messageBus.StopProcessingThread();

    return 0;
}