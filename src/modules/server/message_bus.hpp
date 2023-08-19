/*
 * message_bus.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-23

Description: Main Message Bus

**************************************************/

#pragma once

#define HAS_MESSAGE_BUS

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
#include <typeindex>
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
        std::type_index typeIndex = typeid(T);
        processingThreads_.emplace(typeIndex, std::jthread([&]()
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
            } }));
    }

    template <typename T>
    void StopProcessingThread()
    {
        std::type_index typeIndex = typeid(T);
        auto it = processingThreads_.find(typeIndex);
        if (it != processingThreads_.end())
        {
            it->second.request_stop();
            it->second.join();
            processingThreads_.erase(it);
            LOG_F(INFO, "Processing thread for type %s stopped", typeid(T).name());
        }
    }

    void StopAllProcessingThreads()
    {
        isRunning_.store(false);
        messageAvailableFlag_.notify_one();
        for (auto &thread : processingThreads_)
        {
            thread.second.request_stop();
            thread.second.join();
        }
        processingThreads_.clear();
        LOG_F(INFO, "All processing threads stopped");
    }

private:
    std::unordered_map<std::string, std::vector<std::pair<int, std::any>>> subscribers_;
    std::mutex subscribersLock_;
    std::queue<std::pair<std::string, std::any>> messageQueue_;
    std::mutex messageQueueLock_;
    std::condition_variable messageAvailableFlag_;
    std::mutex waitingMutex_;
    std::unordered_map<std::type_index, std::jthread> processingThreads_;
    std::atomic<bool> isRunning_{true};

    std::vector<std::any> globalSubscribers_;
    std::mutex globalSubscribersLock_;
};