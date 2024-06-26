/*
 * message_queue.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-1-3

Description: A simple message queue (just learn something)

**************************************************/

#ifndef ATOM_ASYNC_MESSAGE_QUEUE_HPP
#define ATOM_ASYNC_MESSAGE_QUEUE_HPP

#include <any>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <typeinfo>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

namespace atom::async {
/**
 * @brief A message queue that allows subscribers to receive messages of type T.
 *
 * @tparam T The type of messages that can be published and subscribed to.
 */
template <typename T>
class MessageQueue {
public:
    /**
     * @brief The callback function type that will be called when a new message
     * is received.
     */
    using CallbackType = std::function<void(const T &)>;

    /**
     * @brief Subscribe a callback function to receive messages.
     *
     * @param callback The callback function to be called when a new message is
     * received.
     * @param subscriberName The name of the subscriber to be added.
     * @param priority The priority of the subscriber. Higher priority
     * subscribers will receive messages before lower priority subscribers.
     */
    void subscribe(CallbackType callback, const std::string &subscriberName,
                   int priority = 0);

    /**
     * @brief Unsubscribe a callback function from receiving messages.
     *
     * @param callback The callback function to be removed.
     */
    void unsubscribe(CallbackType callback);

    /**
     * @brief Publish a new message to all subscribed callback functions.
     *
     * @param message The message to be published.
     */
    void publish(const T &message);

    /**
     * @brief Start the processing thread(s) to receive and handle messages.
     *
     * @param numThreads The number of processing threads to spawn. If not
     * specified, the number of hardware threads will be used.
     */
    void startProcessingThread(
        int numThreads = std::thread::hardware_concurrency());

    /**
     * @brief Stop the processing thread(s) from receiving and handling
     * messages.
     */
    void stopProcessingThread();

    /**
     * @brief Get the number of messages currently in the queue.
     *
     * @return The number of messages in the queue.
     */
    size_t getMessageCount() const;

    /**
     * @brief Get the number of subscribers currently registered.
     *
     * @return The number of subscribers.
     */
    size_t getSubscriberCount() const;

private:
    /**
     * @brief The Subscriber struct contains information about a subscribed
     * callback function.
     */
    struct Subscriber {
        std::string name;      /**< The name of the subscriber. */
        CallbackType callback; /**< The callback function to be called when a
                                  new message is received. */
        int priority;          /**< The priority of the subscriber. */

        /**
         * @brief Construct a new Subscriber object.
         *
         * @param name The name of the subscriber.
         * @param callback The callback function to be called when a new message
         * is received.
         * @param priority The priority of the subscriber.
         */
        Subscriber(const std::string &name, const CallbackType &callback,
                   int priority)
            : name(name), callback(callback), priority(priority) {}

        /**
         * @brief Compare two Subscriber objects based on their priority
         * *
         * @param other The other Subscriber object to compare with.
         * @return True if this Subscriber has a higher priority than the other
         * Subscriber, false otherwise.
         */
        bool operator<(const Subscriber &other) const {
            return priority > other.priority;
        }
    };

    std::deque<T>
        m_messages; /**< The queue containing all published messages. */
    std::vector<Subscriber> m_subscribers; /**< The vector containing all
                                              subscribed callback functions. */
    mutable std::mutex m_mutex; /**< The mutex used to protect access to the
                                   message queue and subscriber vector. */
    std::condition_variable
        m_condition; /**< The condition variable used to notify processing
                        threads of new messages. */
    std::atomic<bool> m_isRunning{
        true}; /**< The flag used to indicate whether the processing thread(s)
                  should continue running. */
    std::vector<std::thread> m_processingThreads; /**< The vector containing all
                                                     processing threads. */
};

template <typename T>
void MessageQueue<T>::subscribe(CallbackType callback,
                                const std::string &subscriberName,
                                int priority) {
    std::lock_guard lock(m_mutex);
    m_subscribers.emplace_back(subscriberName, callback, priority);
    std::sort(m_subscribers.begin(), m_subscribers.end());
}

template <typename T>
void MessageQueue<T>::unsubscribe(CallbackType callback) {
    std::lock_guard lock(m_mutex);
    auto it = std::remove_if(m_subscribers.begin(), m_subscribers.end(),
                             [&callback](const auto &subscriber) {
                                 return subscriber.callback.target_type() ==
                                        callback.target_type();
                             });
    m_subscribers.erase(it, m_subscribers.end());
}

template <typename T>
void MessageQueue<T>::publish(const T &message) {
    {
        std::lock_guard lock(m_mutex);
        m_messages.emplace_back(message);
    }
    m_condition.notify_one();
}

template <typename T>
void MessageQueue<T>::startProcessingThread(int numThreads) {
    for (int i = 0; i < numThreads; ++i) {
        m_processingThreads.emplace_back([this]() {
            while (m_isRunning.load()) {
                std::optional<T> message;

                {
                    std::unique_lock lock(m_mutex);
                    m_condition.wait(lock, [this]() {
                        return !m_messages.empty() || !m_isRunning.load();
                    });

                    if (!m_isRunning.load())
                        return;

                    if (!m_messages.empty()) {
                        message = std::move(m_messages.front());
                        m_messages.pop_front();
                    }
                }

                if (message) {
                    std::vector<CallbackType> subscribersCopy;

                    {
                        std::lock_guard lock(m_mutex);
                        subscribersCopy.reserve(m_subscribers.size());
                        for (const auto &subscriber : m_subscribers) {
                            subscribersCopy.emplace_back(subscriber.callback);
                        }
                    }

                    for (const auto &subscriber : subscribersCopy) {
                        subscriber(*message);
                    }
                }
            }
        });
    }
}

template <typename T>
void MessageQueue<T>::stopProcessingThread() {
    m_isRunning.store(false);
    m_condition.notify_all();
    for (auto &thread : m_processingThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_processingThreads.clear();
}

template <typename T>
size_t MessageQueue<T>::getMessageCount() const {
    std::lock_guard lock(m_mutex);
    return m_messages.size();
}

template <typename T>
size_t MessageQueue<T>::getSubscriberCount() const {
    std::lock_guard lock(m_mutex);
    return m_subscribers.size();
}

}  // namespace atom::async

#endif
