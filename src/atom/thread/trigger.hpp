/*
 * trigger.hpp
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

Date: 2023-12-14

Description: Trigger class for C++

**************************************************/

#pragma once

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <future>
#include <unordered_map>

template <typename ParamType>
class Trigger
{
public:
    using Callback = std::function<void(ParamType)>;

    enum class CallbackPriority
    {
        High,
        Normal,
        Low
    };

    void registerCallback(const std::string &event, const Callback &callback, CallbackPriority priority = CallbackPriority::Normal)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto &callbacks = m_callbacks[event];
        auto pos = std::find_if(callbacks.begin(), callbacks.end(), [&](const std::pair<CallbackPriority, Callback> &cb)
                                { return cb.second.target_type() == callback.target_type() && cb.second.template target<void(ParamType)>() == callback.template target<void(ParamType)>(); });
        if (pos != callbacks.end())
        {
            pos->first = priority;
        }
        else
        {
            callbacks.emplace_back(priority, callback);
        }
    }

    void unregisterCallback(const std::string &event, const Callback &callback)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto &callbacks = m_callbacks[event];
        callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [&](const std::pair<CallbackPriority, Callback> &cb)
                                       { return cb.second.target_type() == callback.target_type() && cb.second.template target<void(ParamType)>() == callback.template target<void(ParamType)>(); }),
                        callbacks.end());
    }

    void trigger(const std::string &event, const ParamType &param)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto &callbacks = m_callbacks[event];
        std::sort(callbacks.begin(), callbacks.end(), [](const std::pair<CallbackPriority, Callback> &cb1, const std::pair<CallbackPriority, Callback> &cb2)
                  { return static_cast<int>(cb1.first) > static_cast<int>(cb2.first); });
        for (auto &callback : callbacks)
        {
            try
            {
                callback.second(param);
            }
            catch (std::exception &e)
            {
                throw std::throw_with_nested(std::nested_exception(e));
            }
        }
    }

    void scheduleTrigger(const std::string &event, const ParamType &param, std::chrono::milliseconds delay)
    {
        std::thread([=]()
                    {
            std::this_thread::sleep_for(delay);
            trigger(event, param); })
            .detach();
    }

    std::future<void> scheduleAsyncTrigger(const std::string &event, const ParamType &param)
    {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();
        std::thread([=]() mutable
                    {
            try {
                trigger(event, param);
                promise->set_value();
            } catch (...) {
                promise->set_exception(std::current_exception());
            } })
            .detach();
        return future;
    }

    void cancelTrigger(const std::string &event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.erase(event);
    }

    void cancelAllTriggers()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.clear();
    }

private:
    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<std::pair<CallbackPriority, Callback>>> m_callbacks;
};