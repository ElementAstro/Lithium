/*
 * message_bus.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-23

Description: Main Message Bus with Asio support and additional features

**************************************************/

#ifndef ATOM_ASYNC_MESSAGE_BUS_HPP
#define ATOM_ASYNC_MESSAGE_BUS_HPP

#include <algorithm>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/steady_timer.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "atom/macro.hpp"

namespace atom::async {

/**
 * @brief The MessageBus class provides a message bus system with Asio support.
 */
class MessageBus {
public:
    using Token = std::size_t;
    static constexpr std::size_t K_MAX_HISTORY_SIZE =
        100;  ///< Maximum number of messages to keep in history.

    /**
     * @brief Constructs a MessageBus with the given io_context.
     * @param io_context The Asio io_context to use for asynchronous operations.
     */
    explicit MessageBus(asio::io_context& io_context)
        : io_context_(io_context) {}

    /**
     * @brief Creates a shared instance of MessageBus.
     * @param io_context The Asio io_context to use for asynchronous operations.
     * @return A shared pointer to the created MessageBus instance.
     */
    static auto createShared(asio::io_context& io_context)
        -> std::shared_ptr<MessageBus> {
        return std::make_shared<MessageBus>(io_context);
    }

    /**
     * @brief Publishes a message to the bus, optionally with a delay.
     * @tparam MessageType The type of the message.
     * @param name The name of the message.
     * @param message The message to publish.
     * @param delay Optional delay before publishing the message.
     */
    template <typename MessageType>
    void publish(
        const std::string& name, const MessageType& message,
        std::optional<std::chrono::milliseconds> delay = std::nullopt) {
        auto publishTask = [this, name, message]() {
            std::shared_lock lock(mutex_);
            std::unordered_set<Token>
                calledSubscribers;  // Track called subscribers

            // Publish to directly matching subscribers
            publishToSubscribers<MessageType>(name, message, calledSubscribers);

            // Publish to namespace matching subscribers
            for (const auto& namespaceName : namespaces_) {
                if (name.find(namespaceName + ".") ==
                    0) {  // Namespace match must start with namespaceName + dot
                    publishToSubscribers<MessageType>(namespaceName, message,
                                                      calledSubscribers);
                }
            }

            // Record the message in history
            recordMessageHistory<MessageType>(name, message);

            // 记录日志
            std::cout << "[MessageBus] Published message: " << name
                      << std::endl;
        };

        if (delay) {
            // Use Asio's steady_timer for delayed publishing
            auto timer =
                std::make_shared<asio::steady_timer>(io_context_, *delay);
            timer->async_wait(
                [timer, publishTask](const asio::error_code& errorCode) {
                    if (!errorCode) {
                        publishTask();
                    }
                });
        } else {
            // Immediately publish asynchronously using asio::post
            asio::post(io_context_, publishTask);
        }
    }

    /**
     * @brief Publishes a message to all subscribers globally.
     * @tparam MessageType The type of the message.
     * @param message The message to publish.
     */
    template <typename MessageType>
    void publishGlobal(const MessageType& message) {
        std::shared_lock lock(mutex_);
        for (const auto& [type, subscribersMap] : subscribers_) {
            for (const auto& [name, subscribersList] : subscribersMap) {
                publish<MessageType>(name, message);
            }
        }
    }

    /**
     * @brief Subscribes to a message.
     * @tparam MessageType The type of the message.
     * @param name The name of the message or namespace (supports wildcard).
     * @param handler The handler function to call when the message is received.
     * @param async Whether to call the handler asynchronously.
     * @param once Whether to unsubscribe after the first message is received.
     * @param filter Optional filter function to determine whether to call the
     * handler.
     * @return A token representing the subscription.
     */
    template <typename MessageType>
    auto subscribe(
        const std::string& name,
        std::function<void(const MessageType&)> handler, bool async = true,
        bool once = false,
        std::function<bool(const MessageType&)> filter =
            [](const MessageType&) { return true; }) -> Token {
        std::unique_lock lock(mutex_);
        Token token = nextToken_++;
        subscribers_[std::type_index(typeid(MessageType))][name].emplace_back(
            Subscriber{std::move(handler), async, once, std::move(filter),
                       token});
        namespaces_.insert(extractNamespace(name));  // Record namespace
        std::cout << "[MessageBus] Subscribed to: " << name
                  << " with token: " << token << std::endl;
        return token;
    }

    /**
     * @brief Unsubscribes from a message using the given token.
     * @tparam MessageType The type of the message.
     * @param token The token representing the subscription.
     */
    template <typename MessageType>
    void unsubscribe(Token token) {
        std::unique_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            for (auto& [name, subscribersList] : iterator->second) {
                removeSubscription(subscribersList, token);
            }
        }
        std::cout << "[MessageBus] Unsubscribed token: " << token << std::endl;
    }

    /**
     * @brief Unsubscribes all handlers for a given message name or namespace.
     * @tparam MessageType The type of the message.
     * @param name The name of the message or namespace.
     */
    template <typename MessageType>
    void unsubscribeAll(const std::string& name) {
        std::unique_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                size_t count = nameIterator->second.size();
                iterator->second.erase(nameIterator);
                std::cout << "[MessageBus] Unsubscribed all handlers for: "
                          << name << " (" << count << " subscribers)"
                          << std::endl;
            }
        }
    }

    /**
     * @brief Gets the number of subscribers for a given message name or
     * namespace.
     * @tparam MessageType The type of the message.
     * @param name The name of the message or namespace.
     * @return The number of subscribers.
     */
    template <typename MessageType>
    auto getSubscriberCount(const std::string& name) -> std::size_t {
        std::shared_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                return nameIterator->second.size();
            }
        }
        return 0;
    }

    /**
     * @brief Checks if there are any subscribers for a given message name or
     * namespace.
     * @tparam MessageType The type of the message.
     * @param name The name of the message or namespace.
     * @return True if there are subscribers, false otherwise.
     */
    template <typename MessageType>
    auto hasSubscriber(const std::string& name) -> bool {
        std::shared_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            auto nameIterator = iterator->second.find(name);
            return nameIterator != iterator->second.end() &&
                   !nameIterator->second.empty();
        }
        return false;
    }

    /**
     * @brief Clears all subscribers.
     */
    void clearAllSubscribers() {
        std::unique_lock lock(mutex_);
        subscribers_.clear();
        namespaces_.clear();
        std::cout << "[MessageBus] Cleared all subscribers." << std::endl;
    }

    /**
     * @brief Gets the list of active namespaces.
     * @return A vector of active namespace names.
     */
    auto getActiveNamespaces() const -> std::vector<std::string> {
        std::shared_lock lock(mutex_);
        return {namespaces_.begin(), namespaces_.end()};
    }

    /**
     * @brief Gets the message history for a given message name.
     * @tparam MessageType The type of the message.
     * @param name The name of the message.
     * @return A vector of messages.
     */
    template <typename MessageType>
    auto getMessageHistory(const std::string& name,
                           std::size_t count = K_MAX_HISTORY_SIZE) const
        -> std::vector<MessageType> {
        std::shared_lock lock(mutex_);
        auto iterator =
            messageHistory_.find(std::type_index(typeid(MessageType)));
        if (iterator != messageHistory_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                std::vector<MessageType> history;
                std::size_t start = (nameIterator->second.size() > count)
                                        ? nameIterator->second.size() - count
                                        : 0;
                for (std::size_t i = start; i < nameIterator->second.size();
                     ++i) {
                    history.emplace_back(
                        std::any_cast<MessageType>(nameIterator->second[i]));
                }
                return history;
            }
        }
        return {};
    }

private:
    struct Subscriber {
        std::function<void(const std::any&)>
            handler;  ///< The handler function.
        bool async;   ///< Whether to call the handler asynchronously.
        bool once;    ///< Whether to unsubscribe after the first message.
        std::function<bool(const std::any&)> filter;  ///< The filter function.
        Token token;  ///< The subscription token.
    } ATOM_ALIGNAS(64);

    /**
     * @brief Publishes a message to the subscribers.
     * @tparam MessageType The type of the message.
     * @param name The name of the message.
     * @param message The message to publish.
     * @param calledSubscribers The set of already called subscribers.
     */
    template <typename MessageType>
    void publishToSubscribers(const std::string& name,
                              const MessageType& message,
                              std::unordered_set<Token>& calledSubscribers) {
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                auto& subscribersList = nameIterator->second;
                for (auto it = subscribersList.begin();
                     it != subscribersList.end();) {
                    if (it->filter(message) &&
                        calledSubscribers.insert(it->token).second) {
                        auto handler = [handlerFunc = it->handler, message]() {
                            std::any msg = message;
                            handlerFunc(msg);
                        };
                        if (it->async) {
                            asio::post(io_context_, handler);
                        } else {
                            handler();
                        }
                        if (it->once) {
                            it = subscribersList.erase(it);
                            continue;
                        }
                    }
                    ++it;
                }
            }
        }
    }

    /**
     * @brief Removes a subscription from the list.
     * @param subscribersList The list of subscribers.
     * @param token The token representing the subscription.
     */
    static void removeSubscription(std::vector<Subscriber>& subscribersList,
                                   Token token) {
        subscribersList.erase(
            std::remove_if(
                subscribersList.begin(), subscribersList.end(),
                [token](const Subscriber& sub) { return sub.token == token; }),
            subscribersList.end());
    }

    /**
     * @brief Records a message in the history.
     * @tparam MessageType The type of the message.
     * @param name The name of the message.
     * @param message The message to record.
     */
    template <typename MessageType>
    void recordMessageHistory(const std::string& name,
                              const MessageType& message) {
        auto& history =
            messageHistory_[std::type_index(typeid(MessageType))][name];
        history.emplace_back(message);
        if (history.size() > K_MAX_HISTORY_SIZE) {
            history.erase(history.begin());
        }
    }

    /**
     * @brief Extracts the namespace from the message name.
     * @param name The message name.
     * @return The namespace part of the name.
     */
    std::string extractNamespace(const std::string& name) const {
        auto pos = name.find('.');
        if (pos != std::string::npos) {
            return name.substr(0, pos);
        }
        return name;
    }

    std::unordered_map<std::type_index,
                       std::unordered_map<std::string, std::vector<Subscriber>>>
        subscribers_;  ///< Map of subscribers.
    std::unordered_map<std::type_index,
                       std::unordered_map<std::string, std::vector<std::any>>>
        messageHistory_;                          ///< Map of message history.
    std::unordered_set<std::string> namespaces_;  ///< Set of namespaces.
    mutable std::shared_mutex mutex_;             ///< Mutex for thread safety.
    Token nextToken_ = 0;                         ///< Next token value.

    asio::io_context&
        io_context_;  ///< Asio io_context for asynchronous operations.
};

}  // namespace atom::async

#endif  // ATOM_ASYNC_MESSAGE_BUS_HPP