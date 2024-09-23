/*
 * message_bus.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-23

Description: Main Message Bus

**************************************************/

#ifndef ATOM_ASYNC_MESSAGE_BUS_HPP
#define ATOM_ASYNC_MESSAGE_BUS_HPP

#include <any>
#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "atom/macro.hpp"

namespace atom::async {
class MessageBus {
public:
    using Token = std::size_t;
    static constexpr std::size_t kMaxHistorySize =
        100;  // 最大保存的消息历史数量

    static auto createShared() -> std::shared_ptr<MessageBus> {
        return std::make_shared<MessageBus>();
    }

    // 发布消息（可延迟）
    template <typename MessageType>
    void publish(
        const std::string& name, const MessageType& message,
        std::optional<std::chrono::milliseconds> delay = std::nullopt) {
        auto publishTask = [this, name, message]() {
            std::shared_lock lock(mutex_);
            std::unordered_set<Token> calledSubscribers;  // 记录已调用的订阅者

            // 并行发布给直接匹配的订阅者
            publishToSubscribers<MessageType>(name, message, calledSubscribers);

// 并行发布给命名空间匹配的订阅者
            for (const auto& namespaceName : namespaces_) {
                if (name.find(namespaceName + ".") ==
                    0) {  // 命名空间匹配必须以 namespaceName+点 开头
                    publishToSubscribers<MessageType>(namespaceName, message,
                                                      calledSubscribers);
                }
            }

            // 将消息记录到历史中
            recordMessageHistory<MessageType>(name, message);
        };

        if (delay) {
            std::jthread([delay,
                          publishTask](const std::stop_token& stopToken) {
                if (std::this_thread::sleep_for(*delay);
                    !stopToken.stop_requested()) {
                    publishTask();
                }
            }).detach();
        } else {
            publishTask();
        }
    }

    template <typename MessageType>
    void publishGlobal(const MessageType& message) {
        std::shared_lock lock(mutex_);
        for (const auto& subscribers : subscribers_) {
            for (const auto& [name, _] : subscribers.second) {
                publish(name, message);
            }
        }
    }

    // 订阅消息
    template <typename MessageType>
    auto subscribe(
        const std::string& name,
        std::function<void(const MessageType&)> handler, bool async = true,
        bool once = false,
        std::function<bool(const MessageType&)> filter =
            [](const MessageType&) { return true; }) -> Token {
        std::unique_lock lock(mutex_);
        Token token = nextToken_++;
        auto filterWrapper = [filter](const std::any& msg) {
            return filter(std::any_cast<const MessageType&>(msg));
        };
        subscribers_[std::type_index(typeid(MessageType))][name].emplace_back(
            Subscriber{std::move(handler), async, once, filterWrapper, token});
        namespaces_.insert(name);  // 记录命名空间
        return token;
    }

    template <typename MessageType>
    void unsubscribe(Token token) {
        std::unique_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            for (auto& [name, subscribersList] : iterator->second) {
                removeSubscription(subscribersList, token);
            }
        }
    }

    template <typename MessageType>
    void unsubscribeAll(const std::string& name) {
        std::unique_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                iterator->second.erase(nameIterator);
            }
        }
    }

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

    template <typename MessageType>
    auto getNamespaceSubscriberCount(const std::string& namespaceName)
        -> std::size_t {
        std::shared_lock lock(mutex_);
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        std::size_t count = 0;
        if (iterator != subscribers_.end()) {
            for (const auto& [name, subscribersList] : iterator->second) {
                if (name.find(namespaceName + ".") == 0) {
                    count += subscribersList.size();
                }
            }
        }
        return count;
    }

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

    // 清空所有订阅者
    void clearAllSubscribers() {
        std::unique_lock lock(mutex_);
        subscribers_.clear();
        namespaces_.clear();
    }

    // 获取当前活动的命名空间列表
    auto getActiveNamespaces() const -> std::vector<std::string> {
        std::shared_lock lock(mutex_);
        return {namespaces_.begin(), namespaces_.end()};
    }

    // 获取消息历史
    template <typename MessageType>
    auto getMessageHistory(const std::string& name) const
        -> std::vector<MessageType> {
        std::shared_lock lock(mutex_);
        auto iterator =
            messageHistory_.find(std::type_index(typeid(MessageType)));
        if (iterator != messageHistory_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                std::vector<MessageType> history;
                for (const auto& message : nameIterator->second) {
                    history.push_back(std::any_cast<MessageType>(message));
                }
                return history;
            }
        }
        static const std::vector<MessageType> EMPTY_HISTORY;
        return EMPTY_HISTORY;
    }

private:
    struct Subscriber {
        std::any handler;
        bool async;
        bool once;
        std::function<bool(const std::any&)> filter;
        Token token;
    } ATOM_ALIGNAS(64);

    template <typename MessageType>
    void publishToSubscribers(const std::string& name,
                              const MessageType& message,
                              std::unordered_set<Token>& calledSubscribers) {
        auto iterator = subscribers_.find(std::type_index(typeid(MessageType)));
        if (iterator != subscribers_.end()) {
            auto nameIterator = iterator->second.find(name);
            if (nameIterator != iterator->second.end()) {
                auto& subscribersList = nameIterator->second;
                for (auto& subscriber : subscribersList) {
                    if (subscriber.filter(message) &&
                        calledSubscribers.insert(subscriber.token).second) {
                        auto handler = std::any_cast<
                            std::function<void(const MessageType&)>>(
                            subscriber.handler);
                        if (handler) {
                            if (subscriber.async) {
                                std::async(std::launch::async, handler, message)
                                    .get();
                            } else {
                                handler(message);
                            }
                            if (subscriber.once) {
                                removeSubscription(subscribersList,
                                                   subscriber.token);
                            }
                        }
                    }
                }
            }
        }
    }

    static void removeSubscription(std::vector<Subscriber>& subscribersList,
                                   Token token) {
        subscribersList.erase(
            std::remove_if(
                subscribersList.begin(), subscribersList.end(),
                [token](const Subscriber& sub) { return sub.token == token; }),
            subscribersList.end());
    }

    template <typename MessageType>
    void recordMessageHistory(const std::string& name,
                              const MessageType& message) {
        auto& history =
            messageHistory_[std::type_index(typeid(MessageType))][name];
        history.push_back(message);
        if (history.size() > kMaxHistorySize) {
            history.erase(history.begin());
        }
    }

    std::unordered_map<std::type_index,
                       std::unordered_map<std::string, std::vector<Subscriber>>>
        subscribers_;
    std::unordered_map<std::type_index,
                       std::unordered_map<std::string, std::vector<std::any>>>
        messageHistory_;
    std::unordered_set<std::string> namespaces_;
    mutable std::shared_mutex mutex_;
    Token nextToken_ = 0;
};
}  // namespace atom::async

#endif  // ATOM_ASYNC_MESSAGE_BUS_HPP
