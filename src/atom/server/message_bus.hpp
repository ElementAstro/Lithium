/*
 * message_bus.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-23

Description: Main Message Bus

**************************************************/

#pragma once

#define HAS_MESSAGE_BUS

#include <string>
#include <vector>
#include <functional>
#include <any>
#include <queue>
#include <atomic>
#include <thread>
#include <algorithm>
#include <mutex>
#include <typeindex>
#include <condition_variable>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/log/loguru.hpp"

namespace Atom::Server
{
    class MessageBus
    {
    public:
        MessageBus()
        {
        }

        MessageBus(const int &max_queue_size)
        {
            maxMessageBusSize_.store(max_queue_size);
        }

        ~MessageBus()
        {
            if (!subscribers_.empty())
            {
                subscribers_.clear();
            }
            if (!processingThreads_.empty())
            {
                for (auto &thread : processingThreads_)
                {
                    if (thread.second.joinable())
                    {
#if __cplusplus >= 202002L
                        thread.second.request_stop();
#endif
                        thread.second.join();
                    }
                }
                processingThreads_.clear();
            }
        }
        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        static std::shared_ptr<MessageBus> createShared()
        {
            return std::make_shared<MessageBus>();
        }

        static std::unique_ptr<MessageBus> createUnique()
        {
            return std::make_unique<MessageBus>();
        }

    public:
        // -------------------------------------------------------------------
        // MessageBus methods
        // -------------------------------------------------------------------

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

            DLOG_F(INFO, "Subscribed to topic: {}", fullTopic);
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

                DLOG_F(INFO, "Unsubscribed from topic: {}", fullTopic);
            }
            subscribersLock_.unlock();
        }

        template <typename T>
        void UnsubscribeFromNamespace(const std::string &namespaceName, std::function<void(const T &)> callback)
        {
            std::string topic = namespaceName + ".*";
            Unsubscribe<T>(topic, callback, namespaceName);
        }

        void UnsubscribeAll(const std::string &namespace_ = "")
        {
            std::string fullTopic = namespace_.empty() ? "*" : (namespace_ + "::*");

            subscribersLock_.lock();
            subscribers_.erase(fullTopic);
            subscribersLock_.unlock();

            DLOG_F(INFO, "Unsubscribed from all topics");
        }

        template <typename T>
        void Publish(const std::string &topic, const T &message, const std::string &namespace_ = "")
        {
            std::string fullTopic = namespace_.empty() ? topic : (namespace_ + "::" + topic);

            messageQueueLock_.lock();
            messageQueue_.push({fullTopic, std::any(message)});
            messageQueueLock_.unlock();
            messageAvailableFlag_.notify_one();

            DLOG_F(INFO, "Published message to topic: {}", fullTopic);
        }

        template <typename T>
        bool TryPublish(const std::string &topic, const T &message, const std::string &namespace_ = "", std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
        {
            std::string fullTopic = namespace_.empty() ? topic : (namespace_ + "::" + topic);

            if (messageQueueLock_.try_lock_until(std::chrono::steady_clock::now() + timeout))
            {
                messageQueue_.push({fullTopic, std::any(message)});
                messageQueueLock_.unlock();
                messageAvailableFlag_.notify_one();

                DLOG_F(INFO, "Published message to topic: {}", fullTopic);
                return true;
            }
            else
            {
                LOG_F(WARNING, "Failed to publish message to topic: {} due to timeout", fullTopic);
                return false;
            }
        }

        template <typename T>
        bool TryReceive(T &outMessage, std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
        {
            std::unique_lock<std::mutex> lock(waitingMutex_);
            if (messageAvailableFlag_.wait_for(lock, timeout, [this]
                                               { return !messageQueue_.empty(); }))
            {
                auto message = std::move(messageQueue_.front());
                messageQueue_.pop();
                outMessage = std::any_cast<T>(message.second);
                return true;
            }
            else
            {
                LOG_F(WARNING, "Failed to receive message due to timeout");
                return false;
            }
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
            std::type_index typeIndex = typeid(T);
#if __cplusplus >= 202002L
            processingThreads_.emplace(typeIndex, std::jthread([&]()
#else
            processingThreads_.emplace(typeIndex, std::thread([&]()
#endif
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
                                LOG_F(ERROR, "Message type mismatch: {}", e.what());
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
                            LOG_F(ERROR, "Global message type mismatch: {}", e.what());
                        } catch (...) {
                            LOG_F(ERROR, "Unknown error occurred during global message processing");
                        }
                        globalSubscribersLock_.unlock();

                        DLOG_F(INFO, "Processed message on topic: {}", topic);
                    }
                } }));
        }

        template <typename T>
        void StopProcessingThread()
        {
            std::type_index typeIndex = typeid(T);
            auto it = processingThreads_.find(typeIndex);
            if (it != processingThreads_.end())
            {
#if __cplusplus >= 202002L
                it->second.request_stop();
#endif
                it->second.join();
                processingThreads_.erase(it);
                DLOG_F(INFO, "Processing thread for type {} stopped", typeid(T).name());
            }
        }

        void StopAllProcessingThreads()
        {
            isRunning_.store(false);
            messageAvailableFlag_.notify_one();
            for (auto &thread : processingThreads_)
            {
#if __cplusplus >= 202002L
                thread.second.request_stop();
#endif
                thread.second.join();
            }
            processingThreads_.clear();
            DLOG_F(INFO, "All processing threads stopped");
        }

    private:
        std::unordered_map<std::string, std::vector<std::pair<int, std::any>>> subscribers_;
        std::mutex subscribersLock_;
        std::queue<std::pair<std::string, std::any>> messageQueue_;
        std::timed_mutex messageQueueLock_;
        std::condition_variable messageAvailableFlag_;
        std::mutex waitingMutex_;
#if __cplusplus >= 202002L
#if ENABLE_FASTHASH
        emhash8::HashMap<std::type_index, std::thread> processingThreads_;
#else
        std::unordered_map<std::type_index, std::jthread> processingThreads_;
#endif
#else
#if ENABLE_FASTHASH
        emhash8::HashMap<std::type_index, std::thread> processingThreads_;
#else
        std::unordered_map<std::type_index, std::thread> processingThreads_;
#endif
#endif
        std::atomic<bool> isRunning_{true};
        std::atomic_int maxMessageBusSize_{1000};

        std::vector<std::any> globalSubscribers_;
        std::mutex globalSubscribersLock_;
    };
}
