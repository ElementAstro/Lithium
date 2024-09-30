/*
 * message_queue.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_ASYNC_MESSAGE_QUEUE_HPP
#define ATOM_ASYNC_MESSAGE_QUEUE_HPP

#include <algorithm>
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace atom::async {

/**
 * @brief A message queue that allows subscribers to receive messages of type T.
 *
 * @tparam T The type of messages that can be published and subscribed to.
 */
template <typename T>
class MessageQueue {
public:
    using CallbackType = std::function<void(const T&)>;
    using FilterType = std::function<bool(const T&)>;

    /**
     * @brief Constructs a MessageQueue with the given io_context.
     * @param ioContext The Asio io_context to use for asynchronous operations.
     */
    explicit MessageQueue(asio::io_context& ioContext)
        : ioContext_(ioContext) {}

    /**
     * @brief Subscribe to messages with a callback and optional filter and
     * timeout.
     *
     * @param callback The callback function to be called when a new message is
     * received.
     * @param subscriberName The name of the subscriber.
     * @param priority The priority of the subscriber. Higher priority receives
     * messages first.
     * @param filter An optional filter to only receive messages that match the
     * criteria.
     * @param timeout The maximum time allowed for the subscriber to process a
     * message.
     */
    void subscribe(
        CallbackType callback, const std::string& subscriberName,
        int priority = 0, FilterType filter = nullptr,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    /**
     * @brief Unsubscribe from messages using the given callback.
     *
     * @param callback The callback function used during subscription.
     */
    void unsubscribe(CallbackType callback);

    /**
     * @brief Publish a message to the queue, with an optional priority.
     *
     * @param message The message to publish.
     * @param priority The priority of the message, higher priority messages are
     * handled first.
     */
    void publish(const T& message, int priority = 0);

    /**
     * @brief Start processing messages in the queue.
     */
    void startProcessing();

    /**
     * @brief Stop processing messages in the queue.
     */
    void stopProcessing();

    /**
     * @brief Get the number of messages currently in the queue.
     * @return The number of messages in the queue.
     */
    auto getMessageCount() const -> size_t;

    /**
     * @brief Get the number of subscribers currently subscribed to the queue.
     * @return The number of subscribers.
     */
    auto getSubscriberCount() const -> size_t;

    /**
     * @brief Cancel specific messages that meet a given condition.
     *
     * @param cancelCondition The condition to cancel certain messages.
     */
    void cancelMessages(std::function<bool(const T&)> cancelCondition);

private:
    struct Subscriber {
        std::string name;
        CallbackType callback;
        int priority;
        FilterType filter;
        std::chrono::milliseconds timeout;

        Subscriber(std::string name, const CallbackType& callback, int priority,
                   FilterType filter, std::chrono::milliseconds timeout)
            : name(std::move(name)),
              callback(callback),
              priority(priority),
              filter(filter),
              timeout(timeout) {}

        auto operator<(const Subscriber& other) const -> bool {
            return priority > other.priority;
        }
    };

    struct Message {
        T data;
        int priority;

        Message(T data, int priority)
            : data(std::move(data)), priority(priority) {}

        auto operator<(const Message& other) const -> bool {
            return priority > other.priority;
        }
    };

    std::deque<Message> m_messages_;
    std::vector<Subscriber> m_subscribers_;
    mutable std::mutex m_mutex_;
    std::condition_variable m_condition_;
    std::atomic<bool> m_isRunning_{true};
    asio::io_context& ioContext_;

    /**
     * @brief Process messages in the queue.
     */
    void processMessages();

    /**
     * @brief Apply the filter to a message for a given subscriber.
     * @param subscriber The subscriber to apply the filter for.
     * @param message The message to filter.
     * @return True if the message passes the filter, false otherwise.
     */
    bool applyFilter(const Subscriber& subscriber, const T& message);

    /**
     * @brief Handle the timeout for a given subscriber and message.
     * @param subscriber The subscriber to handle the timeout for.
     * @param message The message to process.
     * @return True if the message was processed within the timeout, false
     * otherwise.
     */
    bool handleTimeout(const Subscriber& subscriber, const T& message);
};

template <typename T>
void MessageQueue<T>::subscribe(CallbackType callback,
                                const std::string& subscriberName, int priority,
                                FilterType filter,
                                std::chrono::milliseconds timeout) {
    std::lock_guard lock(m_mutex_);
    m_subscribers_.emplace_back(subscriberName, callback, priority, filter,
                                timeout);
    std::ranges::sort(m_subscribers_, std::greater{});
}

template <typename T>
void MessageQueue<T>::unsubscribe(CallbackType callback) {
    std::lock_guard lock(m_mutex_);
    auto iterator = std::ranges::remove_if(
        m_subscribers_, [&callback](const auto& subscriber) {
            return subscriber.callback.target_type() == callback.target_type();
        });
    m_subscribers_.erase(iterator.begin(), iterator.end());
}

template <typename T>
void MessageQueue<T>::publish(const T& message, int priority) {
    {
        std::lock_guard lock(m_mutex_);
        m_messages_.emplace_back(message, priority);
    }
    ioContext_.post([this]() { processMessages(); });
}

template <typename T>
void MessageQueue<T>::startProcessing() {
    m_isRunning_.store(true);
    ioContext_.run();
}

template <typename T>
void MessageQueue<T>::stopProcessing() {
    m_isRunning_.store(false);
    ioContext_.stop();
}

template <typename T>
auto MessageQueue<T>::getMessageCount() const -> size_t {
    std::lock_guard lock(m_mutex_);
    return m_messages_.size();
}

template <typename T>
auto MessageQueue<T>::getSubscriberCount() const -> size_t {
    std::lock_guard lock(m_mutex_);
    return m_subscribers_.size();
}

template <typename T>
void MessageQueue<T>::cancelMessages(
    std::function<bool(const T&)> cancelCondition) {
    std::lock_guard lock(m_mutex_);
    auto iterator = std::remove_if(m_messages_.begin(), m_messages_.end(),
                                   [&cancelCondition](const auto& msg) {
                                       return cancelCondition(msg.data);
                                   });
    m_messages_.erase(iterator, m_messages_.end());
}

template <typename T>
bool MessageQueue<T>::applyFilter(const Subscriber& subscriber,
                                  const T& message) {
    if (!subscriber.filter) {
        return true;
    }
    return subscriber.filter(message);
}

template <typename T>
bool MessageQueue<T>::handleTimeout(const Subscriber& subscriber,
                                    const T& message) {
    if (subscriber.timeout == std::chrono::milliseconds::zero()) {
        subscriber.callback(message);
        return true;
    }

    std::packaged_task<void()> task(
        [&subscriber, &message]() { subscriber.callback(message); });
    auto future = task.get_future();
    asio::post(ioContext_, std::move(task));

    if (future.wait_for(subscriber.timeout) == std::future_status::timeout) {
        return false;  // Timeout occurred.
    }

    return true;  // Process completed within timeout.
}

template <typename T>
void MessageQueue<T>::processMessages() {
    while (m_isRunning_.load()) {
        std::optional<Message> message;

        {
            std::lock_guard lock(m_mutex_);
            if (m_messages_.empty()) {
                return;
            }
            message = std::move(m_messages_.front());
            m_messages_.pop_front();
        }

        if (message) {
            std::vector<Subscriber> subscribersCopy;

            {
                std::lock_guard lock(m_mutex_);
                subscribersCopy.reserve(m_subscribers_.size());
                for (const auto& subscriber : m_subscribers_) {
                    subscribersCopy.emplace_back(subscriber);
                }
            }

            for (const auto& subscriber : subscribersCopy) {
                if (applyFilter(subscriber, message->data)) {
                    handleTimeout(subscriber, message->data);
                }
            }
        }
    }
}

}  // namespace atom::async

#endif  // ATOM_ASYNC_MESSAGE_QUEUE_HPP