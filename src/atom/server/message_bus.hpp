/*
 * message_bus.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-23

Description: Main Message Bus

**************************************************/

#ifndef ATOM_SERVER_MESSAGE_BUS_HPP
#define ATOM_SERVER_MESSAGE_BUS_HPP

#define HAS_MESSAGE_BUS

#include <algorithm>
#include <any>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <typeindex>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/log/loguru.hpp"

namespace Atom::Server {
class MessageBus {
public:
    MessageBus() {}

    MessageBus(const int &max_queue_size) {
        maxMessageBusSize_.store(max_queue_size);
    }

    ~MessageBus() { StopAllProcessingThreads(); }

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    static std::shared_ptr<MessageBus> createShared() {
        return std::make_shared<MessageBus>();
    }

    static std::unique_ptr<MessageBus> createUnique() {
        return std::make_unique<MessageBus>();
    }

public:
    // -------------------------------------------------------------------
    // MessageBus methods
    // -------------------------------------------------------------------

    template <typename T>
    void Subscribe(const std::string &topic,
                   std::function<void(const T &)> callback, int priority = 0,
                   const std::string &namespace_ = "") {
        std::string fullTopic =
            namespace_.empty() ? topic : (namespace_ + "::" + topic);

        std::scoped_lock lock(subscribersLock_);
        subscribers_[fullTopic].emplace_back(priority, std::move(callback));
        std::sort(
            subscribers_[fullTopic].begin(), subscribers_[fullTopic].end(),
            [](const auto &a, const auto &b) { return a.first > b.first; });

        DLOG_F(INFO, "Subscribed to topic: {}", fullTopic);
    }

    template <typename T>
    void SubscribeToNamespace(const std::string &namespaceName,
                              std::function<void(const T &)> callback,
                              int priority = 0) {
        std::string topic = namespaceName + ".*";
        Subscribe<T>(topic, callback, priority, namespaceName);
    }

    template <typename T>
    void Unsubscribe(const std::string &topic,
                     std::function<void(const T &)> callback,
                     const std::string &namespace_ = "") {
        std::string fullTopic =
            namespace_.empty() ? topic : (namespace_ + "::" + topic);

        std::scoped_lock lock(subscribersLock_);
        auto it = subscribers_.find(fullTopic);
        if (it != subscribers_.end()) {
            auto &topicSubscribers = it->second;
            topicSubscribers.erase(
                std::remove_if(topicSubscribers.begin(), topicSubscribers.end(),
                               [&](const auto &subscriber) {
                                   return subscriber.second.type() ==
                                          typeid(callback);
                               }),
                topicSubscribers.end());

            DLOG_F(INFO, "Unsubscribed from topic: {}", fullTopic);
        }
    }

    template <typename T>
    void UnsubscribeFromNamespace(const std::string &namespaceName,
                                  std::function<void(const T &)> callback) {
        std::string topic = namespaceName + ".*";
        Unsubscribe<T>(topic, callback, namespaceName);
    }

    void UnsubscribeAll(const std::string &namespace_ = "") {
        std::string fullTopic = namespace_.empty() ? "*" : (namespace_ + "::*");

        std::scoped_lock lock(subscribersLock_);
        subscribers_.erase(fullTopic);

        DLOG_F(INFO, "Unsubscribed from all topics");
    }

    template <typename T>
    void Publish(const std::string &topic, const T &message,
                 const std::string &namespace_ = "") {
        std::string fullTopic =
            namespace_.empty() ? topic : (namespace_ + "::" + topic);

        {
            std::scoped_lock lock(messageQueueLock_);
            if (messageQueue_.size() >= maxMessageBusSize_) {
                LOG_F(WARNING,
                      "Message queue is full. Discarding oldest message.");
                messageQueue_.pop();
            }
            messageQueue_.emplace(fullTopic, message);
        }
        messageAvailableFlag_.notify_one();

        DLOG_F(INFO, "Published message to topic: {}", fullTopic);
    }

    template <typename T>
    bool TryPublish(
        const std::string &topic, const T &message,
        const std::string &namespace_ = "",
        std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
        std::string fullTopic =
            namespace_.empty() ? topic : (namespace_ + "::" + topic);

        {
            std::unique_lock lock(messageQueueLock_);
            if (messageQueueCondition_.wait_for(lock, timeout, [this] {
                    return messageQueue_.size() < maxMessageBusSize_;
                })) {
                messageQueue_.emplace(fullTopic, message);
                messageAvailableFlag_.notify_one();
                DLOG_F(INFO, "Published message to topic: {}", fullTopic);
                return true;
            } else {
                LOG_F(WARNING,
                      "Failed to publish message to topic: {} due to timeout",
                      fullTopic);
                return false;
            }
        }
    }

    template <typename T>
    bool TryReceive(T &outMessage, std::chrono::milliseconds timeout =
                                       std::chrono::milliseconds(100)) {
        std::unique_lock lock(waitingMutex_);
        if (messageAvailableFlag_.wait_for(
                lock, timeout, [this] { return !messageQueue_.empty(); })) {
            std::scoped_lock messageLock(messageQueueLock_);
            auto message = std::move(messageQueue_.front());
            messageQueue_.pop();
            outMessage = std::any_cast<T>(message.second);
            return true;
        } else {
            LOG_F(WARNING, "Failed to receive message due to timeout");
            return false;
        }
    }

    template <typename T>
    void GlobalSubscribe(std::function<void(const T &)> callback) {
        std::scoped_lock lock(globalSubscribersLock_);
        globalSubscribers_.emplace_back(std::move(callback));
    }

    template <typename T>
    void GlobalUnsubscribe(std::function<void(const T &)> callback) {
        std::scoped_lock lock(globalSubscribersLock_);
        globalSubscribers_.erase(
            std::remove_if(globalSubscribers_.begin(), globalSubscribers_.end(),
                           [&](const auto &subscriber) {
                               return subscriber.type() == typeid(callback);
                           }),
            globalSubscribers_.end());
    }

    template <typename T>
    void StartProcessingThread() {
        std::type_index typeIndex = typeid(T);
        if (processingThreads_.find(typeIndex) == processingThreads_.end()) {
            processingThreads_.emplace(
                typeIndex,
                std::jthread([this, typeIndex](std::stop_token stopToken) {
                    while (!stopToken.stop_requested()) {
                        std::pair<std::string, std::any> message;
                        bool hasMessage = false;

                        {
                            std::unique_lock lock(waitingMutex_);
                            messageAvailableFlag_.wait(lock, stopToken, [this] {
                                return !messageQueue_.empty();
                            });
                            if (stopToken.stop_requested()) {
                                break;
                            }
                            std::scoped_lock messageLock(messageQueueLock_);
                            if (!messageQueue_.empty()) {
                                message = std::move(messageQueue_.front());
                                messageQueue_.pop();
                                hasMessage = true;
                            }
                        }

                        if (hasMessage) {
                            const std::string &topic = message.first;
                            const std::any &data = message.second;

                            std::shared_lock subscribersLock(subscribersLock_);
                            auto it = subscribers_.find(topic);
                            if (it != subscribers_.end()) {
                                try {
                                    for (const auto &subscriber : it->second) {
                                        if (subscriber.second.type() ==
                                            typeid(std::function<void(
                                                       const T &)>)) {
                                            std::any_cast<
                                                std::function<void(const T &)>>(
                                                subscriber.second)(
                                                std::any_cast<const T &>(data));
                                        }
                                    }
                                } catch (const std::bad_any_cast &e) {
                                    LOG_F(ERROR, "Message type mismatch: {}",
                                          e.what());
                                } catch (...) {
                                    LOG_F(ERROR,
                                          "Unknown error occurred during "
                                          "message processing");
                                }
                            }

                            std::shared_lock globalLock(globalSubscribersLock_);
                            try {
                                for (const auto &subscriber :
                                     globalSubscribers_) {
                                    if (subscriber.type() ==
                                        typeid(
                                            std::function<void(const T &)>)) {
                                        std::any_cast<
                                            std::function<void(const T &)>>(
                                            subscriber)(
                                            std::any_cast<const T &>(data));
                                    }
                                }
                            } catch (const std::bad_any_cast &e) {
                                LOG_F(ERROR, "Global message type mismatch: {}",
                                      e.what());
                            } catch (...) {
                                LOG_F(ERROR,
                                      "Unknown error occurred during global "
                                      "message processing");
                            }

                            DLOG_F(INFO, "Processed message on topic: {}",
                                   topic);
                        }
                    }
                }));
        }
    }

    template <typename T>
    void StopProcessingThread() {
        std::type_index typeIndex = typeid(T);
        auto it = processingThreads_.find(typeIndex);
        if (it != processingThreads_.end()) {
            it->second.request_stop();
            it->second.join();
            processingThreads_.erase(it);
            DLOG_F(INFO, "Processing thread for type {} stopped",
                   typeid(T).name());
        }
    }

    void StopAllProcessingThreads() {
        for (auto &thread : processingThreads_) {
            thread.second.request_stop();
        }
        for (auto &thread : processingThreads_) {
            thread.second.join();
        }
        processingThreads_.clear();
        DLOG_F(INFO, "All processing threads stopped");
    }

private:
    using SubscriberCallback = std::function<void(const std::any &)>;
    using SubscriberList = std::vector<std::pair<int, SubscriberCallback>>;
    using SubscriberMap = std::unordered_map<std::string, SubscriberList>;

    SubscriberMap subscribers_;
    std::shared_mutex subscribersLock_;

    using MessageQueue = std::queue<std::pair<std::string, std::any>>;
    MessageQueue messageQueue_;
    std::mutex messageQueueLock_;
    std::condition_variable messageQueueCondition_;
    std::condition_variable messageAvailableFlag_;
    std::mutex waitingMutex_;

    using ProcessingThread = std::jthread;
    using ProcessingThreadMap =
        std::unordered_map<std::type_index, ProcessingThread>;
    ProcessingThreadMap processingThreads_;

    std::atomic_int maxMessageBusSize_{1000};

    using GlobalSubscriberList = std::vector<SubscriberCallback>;
    GlobalSubscriberList globalSubscribers_;
    std::shared_mutex globalSubscribersLock_;
};
}  // namespace Atom::Server

#endif