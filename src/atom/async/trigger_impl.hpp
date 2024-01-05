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
void Trigger<ParamType>::registerCallback(const std::string &event, const Callback &callback, CallbackPriority priority)
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

template <typename ParamType>
void Trigger<ParamType>::unregisterCallback(const std::string &event, const Callback &callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto &callbacks = m_callbacks[event];
    callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(), [&](const std::pair<CallbackPriority, Callback> &cb)
                                   { return cb.second.target_type() == callback.target_type() && cb.second.template target<void(ParamType)>() == callback.template target<void(ParamType)>(); }),
                    callbacks.end());
}

template <typename ParamType>
void Trigger<ParamType>::trigger(const std::string &event, const ParamType &param)
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

template <typename ParamType>
void Trigger<ParamType>::scheduleTrigger(const std::string &event, const ParamType &param, std::chrono::milliseconds delay)
{
    std::thread([=]()
                {
        std::this_thread::sleep_for(delay);
        trigger(event, param); })
        .detach();
}

template <typename ParamType>
std::future<void> Trigger<ParamType>::scheduleAsyncTrigger(const std::string &event, const ParamType &param)
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

template <typename ParamType>
void Trigger<ParamType>::cancelTrigger(const std::string &event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.erase(event);
}

template <typename ParamType>
void Trigger<ParamType>::cancelAllTriggers()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.clear();
}
