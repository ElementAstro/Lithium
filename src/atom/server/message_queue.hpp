/*
 * message_queue.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-1-3

Description: A simple message queue (just learn something)

**************************************************/

#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <typeinfo>
#include <any>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

template <typename T>
class MessageQueue
{
public:
    using CallbackType = std::function<void(const T &)>;

    void Subscribe(CallbackType callback)
    {
        subscribers_.push_back(std::move(callback));
    }

    void Unsubscribe(CallbackType callback)
    {
        subscribers_.erase(
            std::remove(subscribers_.begin(), subscribers_.end(), callback),
            subscribers_.end());
    }

    void Publish(const T &message)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        messages_.push(message);
        lock.unlock();
        condition_.notify_one();
    }

    void StartProcessingThread()
    {
        processingThread_ = std::thread([this]()
                                        {
            while (isRunning_.load()) {
                std::optional<T> message;
                
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]() { return !messages_.empty() || !isRunning_.load(); });

                    if (!isRunning_.load())
                        return;

                    if (!messages_.empty()) {
                        message = messages_.front();
                        messages_.pop();
                    }
                }

                if (message) {
                    for (const auto& subscriber : subscribers_) {
                        subscriber(*message);
                    }
                }
            } });
    }

    void StopProcessingThread()
    {
        isRunning_.store(false);
        condition_.notify_one();
        if (processingThread_.joinable())
            processingThread_.join();
    }

private:
    std::queue<T> messages_;
    std::vector<CallbackType> subscribers_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> isRunning_{true};
    std::thread processingThread_;
};