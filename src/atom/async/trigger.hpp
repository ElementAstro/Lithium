/*
 * trigger.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-14

Description: Trigger class for C++

**************************************************/

#ifndef ATOM_ASYNC_TRIGGER_HPP
#define ATOM_ASYNC_TRIGGER_HPP

#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace atom::async {

/**
 * @brief Concept to check if a type can be invoked with a given parameter type.
 *
 * This concept checks if a std::function taking a parameter of type ParamType
 * is invocable with an instance of ParamType.
 *
 * @tparam ParamType The parameter type to check for.
 */
template <typename ParamType>
concept CallableWithParam = requires(ParamType p) {
    std::invoke(std::declval<std::function<void(ParamType)>>(), p);
};

/**
 * @brief A class for handling event-driven callbacks with parameter support.
 *
 * This class allows users to register, unregister, and trigger callbacks for
 * different events, providing a mechanism to manage callbacks with priorities
 * and delays.
 *
 * @tparam ParamType The type of parameter to be passed to the callbacks.
 */
template <typename ParamType>
    requires CallableWithParam<ParamType>
class Trigger {
public:
    using Callback = std::function<void(ParamType)>;  ///< Type alias for the
                                                      ///< callback function.

    /// Enumeration for callback priority levels.
    enum class CallbackPriority { High, Normal, Low };

    /**
     * @brief Registers a callback for a specified event.
     *
     * @param event The name of the event for which the callback is registered.
     * @param callback The callback function to be executed when the event is
     * triggered.
     * @param priority The priority level of the callback (default is Normal).
     */
    void registerCallback(const std::string& event, Callback callback,
                          CallbackPriority priority = CallbackPriority::Normal);

    /**
     * @brief Unregisters a callback for a specified event.
     *
     * @param event The name of the event from which the callback is
     * unregistered.
     * @param callback The callback function to be removed.
     *
     * If the callback is not registered for the event, no action is taken.
     */
    void unregisterCallback(const std::string& event, Callback callback);

    /**
     * @brief Triggers the callbacks associated with a specified event.
     *
     * @param event The name of the event to trigger.
     * @param param The parameter to be passed to the callbacks.
     *
     * All callbacks registered for the event are executed with the provided
     * parameter.
     */
    void trigger(const std::string& event, const ParamType& param);

    /**
     * @brief Schedules a trigger for a specified event after a delay.
     *
     * @param event The name of the event to trigger.
     * @param param The parameter to be passed to the callbacks.
     * @param delay The delay after which to trigger the event, specified in
     * milliseconds.
     */
    void scheduleTrigger(const std::string& event, const ParamType& param,
                         std::chrono::milliseconds delay);

    /**
     * @brief Schedules an asynchronous trigger for a specified event.
     *
     * @param event The name of the event to trigger.
     * @param param The parameter to be passed to the callbacks.
     * @return A future representing the ongoing operation to trigger the event.
     */
    auto scheduleAsyncTrigger(const std::string& event,
                              const ParamType& param) -> std::future<void>;

    /**
     * @brief Cancels the scheduled trigger for a specified event.
     *
     * @param event The name of the event for which to cancel the trigger.
     *
     * This will prevent the execution of any scheduled callbacks for the event.
     */
    void cancelTrigger(const std::string& event);

    /**
     * @brief Cancels all scheduled triggers.
     *
     * This method clears all scheduled callbacks for any events.
     */
    void cancelAllTriggers();

private:
    std::mutex m_mutex_;  ///< Mutex for thread-safe access to the internal
                          ///< callback structures.
    std::unordered_map<std::string,
                       std::vector<std::pair<CallbackPriority, Callback>>>
        m_callbacks_;  ///< Map of events to their callbacks and priorities.
};

template <typename ParamType>
    requires CallableWithParam<ParamType>
void Trigger<ParamType>::registerCallback(const std::string& event,
                                          Callback callback,
                                          CallbackPriority priority) {
    std::scoped_lock lock(m_mutex_);
    auto& callbacks = m_callbacks_[event];
    if (auto pos = std::ranges::find_if(
            callbacks,
            [&callback](const auto& cb) {
                return cb.second.target_type() == callback.target_type() &&
                       cb.second.template target<void(ParamType)>() ==
                           callback.template target<void(ParamType)>();
            });
        pos != callbacks.end()) {
        pos->first = priority;
    } else {
        callbacks.emplace_back(priority, callback);
    }
}

template <typename ParamType>
    requires CallableWithParam<ParamType>
void Trigger<ParamType>::unregisterCallback(const std::string& event,
                                            Callback callback) {
    std::scoped_lock lock(m_mutex_);
    auto& callbacks = m_callbacks_[event];
    std::erase_if(callbacks, [&callback](const auto& cb) {
        return cb.second.target_type() == callback.target_type() &&
               cb.second.template target<void(ParamType)>() ==
                   callback.template target<void(ParamType)>();
    });
}

template <typename ParamType>
    requires CallableWithParam<ParamType>
void Trigger<ParamType>::trigger(const std::string& event,
                                 const ParamType& param) {
    std::scoped_lock lock(m_mutex_);
    auto& callbacks = m_callbacks_[event];
    std::ranges::sort(callbacks, [](const auto& cb1, const auto& cb2) {
        return static_cast<int>(cb1.first) > static_cast<int>(cb2.first);
    });
    for (auto& [priority, callback] : callbacks) {
        try {
            callback(param);
        } catch (...) {
            // Swallow exceptions in callbacks
        }
    }
}

template <typename ParamType>
    requires CallableWithParam<ParamType>
void Trigger<ParamType>::scheduleTrigger(const std::string& event,
                                         const ParamType& param,
                                         std::chrono::milliseconds delay) {
    std::jthread([this, event, param, delay]() {
        std::this_thread::sleep_for(delay);
        trigger(event, param);
    }).detach();
}

template <typename ParamType>
    requires CallableWithParam<ParamType>
auto Trigger<ParamType>::scheduleAsyncTrigger(
    const std::string& event, const ParamType& param) -> std::future<void> {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();
    std::jthread([this, event, param, promise]() mutable {
        try {
            trigger(event, param);
            promise->set_value();
        } catch (...) {
            promise->set_exception(std::current_exception());
        }
    }).detach();
    return future;
}

template <typename ParamType>
    requires CallableWithParam<ParamType>
void Trigger<ParamType>::cancelTrigger(const std::string& event) {
    std::scoped_lock lock(m_mutex_);
    m_callbacks_.erase(event);
}

template <typename ParamType>
    requires CallableWithParam<ParamType>
void Trigger<ParamType>::cancelAllTriggers() {
    std::scoped_lock lock(m_mutex_);
    m_callbacks_.clear();
}

}  // namespace atom::async

#endif  // ATOM_ASYNC_TRIGGER_HPP