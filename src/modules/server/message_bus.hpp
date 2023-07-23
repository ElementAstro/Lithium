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

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <any>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>
#include "loguru/loguru.hpp"

class MessageBus
{
public:
    template <typename T>
    void Subscribe(const std::string &topic, std::function<void(const T &)> callback, int priority = 0)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        subscribers_[topic].push_back({priority, std::any(callback)});
        std::sort(subscribers_[topic].begin(), subscribers_[topic].end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.first > b.first;
                  });

        LOG_F(INFO, "Subscribed to topic: %s", topic.c_str());
    }

    template <typename T>
    void Unsubscribe(const std::string &topic, std::function<void(const T &)> callback)
    {
        std::unique_lock<std::mutex> lock(mutex_);
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

            // LOG_F(INFO, "Unsubscribed from topic: %s", topic.c_str());
        }
    }

    template <typename T>
    void Publish(const std::string &topic, const T &message)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        messageQueue_.push({topic, std::any(message)});
        conditionVariable_.notify_one();

        // LOG_F(INFO, "Published message to topic: %s", topic.c_str());
    }

    template <typename T>
    void StartProcessingThread()
    {
        processingThread_ = std::thread([&]()
                                        {
            while (isRunning_) {
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

                const std::string& topic = message.first;
                const std::any& data = message.second;
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

                //LOG_F(INFO, "Processed message on topic: %s", topic.c_str());
            } });
    }

    void StopProcessingThread()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            isRunning_ = false;
        }

        conditionVariable_.notify_one();

        if (processingThread_.joinable())
        {
            processingThread_.join();
            LOG_F(INFO, "Processing thread stopped");
        }
    }

private:
    std::unordered_map<std::string, std::vector<std::pair<int, std::any>>> subscribers_;
    std::queue<std::pair<std::string, std::any>> messageQueue_;
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::thread processingThread_;
    bool isRunning_ = true;
};
/*
int main(int argc, char *argv[])
{
    loguru::init(argc, argv);
    MessageBus bus;

    auto callback1 = [](const std::string &message)
    {
        LOG_F(INFO, "Callback 1 received message: %s", message.c_str());
    };
    auto callback2 = [](const std::string &message)
    {
        LOG_F(INFO, "Callback 2 received message: %s", message.c_str());
    };

    OtherClass otherObj;
    bus.Subscribe("my_topic", std::function<void(const std::string &)>(std::bind(&OtherClass::OnMyMessageReceived, &otherObj, std::placeholders::_1)));

    bus.Subscribe("my_topic", static_cast<std::function<void(const std::string &)>>(callback1), 1);
    bus.Subscribe("my_topic", static_cast<std::function<void(const std::string &)>>(callback2), -1);


    bus.StartProcessingThread<std::string>();

    for (int i = 0; i < 10; i++)
    {
        bus.Publish("my_topic", std::string("Hello, world!") + std::to_string(i));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   bus.Unsubscribe("my_topic", static_cast<std::function<void(const std::string &)>>(callback1));

    for (int i = 0; i < 10; i++)
    {
        bus.Publish("my_topic", std::string("Hello again!") + std::to_string(i));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bus.StopProcessingThread();

    return 0;
}

*/
