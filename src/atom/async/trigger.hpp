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

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <future>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

/**
 * @brief The Trigger class provides a mechanism to register callbacks for specific events and trigger those callbacks when the events occur.
 *
 * @tparam ParamType The parameter type passed to the callback functions.
 */
template <typename ParamType>
class Trigger
{
public:
    using Callback = std::function<void(ParamType)>; /**< Type definition for callback functions. */

    /**
     * @brief Enumeration for defining the priority of callback functions.
     */
    enum class CallbackPriority
    {
        High,   /**< High priority */
        Normal, /**< Normal priority */
        Low     /**< Low priority */
    };

    /**
     * @brief Register a callback function for a specific event.
     *
     * @param event The name of the event.
     * @param callback The callback function to be registered.
     * @param priority The priority of the callback function (default: CallbackPriority::Normal).
     */
    void registerCallback(const std::string &event, const Callback &callback, CallbackPriority priority = CallbackPriority::Normal);

    /**
     * @brief Unregister a callback function for a specific event.
     *
     * @param event The name of the event.
     * @param callback The callback function to be unregistered.
     */
    void unregisterCallback(const std::string &event, const Callback &callback);

    /**
     * @brief Trigger the callback functions registered for a specific event.
     *
     * @param event The name of the event to trigger.
     * @param param The parameter to be passed to the callback functions.
     */
    void trigger(const std::string &event, const ParamType &param);

    /**
     * @brief Schedule the triggering of an event with a specified delay.
     *
     * @param event The name of the event to schedule.
     * @param param The parameter to be passed to the callback functions when the event is triggered.
     * @param delay The delay in milliseconds before triggering the event.
     */
    void scheduleTrigger(const std::string &event, const ParamType &param, std::chrono::milliseconds delay);

    /**
     * @brief Schedule the asynchronous triggering of an event.
     *
     * @param event The name of the event to schedule.
     * @param param The parameter to be passed to the callback functions when the event is triggered.
     * @return A future object that can be used to track the completion of the asynchronous trigger.
     */
    std::future<void> scheduleAsyncTrigger(const std::string &event, const ParamType &param);

    /**
     * @brief Cancel the scheduled triggering of a specific event.
     *
     * @param event The name of the event to cancel.
     */
    void cancelTrigger(const std::string &event);

    /**
     * @brief Cancel the scheduled triggering of all events.
     */
    void cancelAllTriggers();

private:
    std::mutex m_mutex; /**< Mutex used to synchronize access to the callback data structure. */

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::vector<std::pair<CallbackPriority, Callback>>> m_callbacks; /**< Hash map to store registered callbacks for events. */
#else
    std::unordered_map<std::string, std::vector<std::pair<CallbackPriority, Callback>>> m_callbacks; /**< Hash map to store registered callbacks for events. */
#endif
};

#include "trigger_impl.hpp"

#endif
