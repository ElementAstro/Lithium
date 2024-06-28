/*
 * eventstack_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-26

Description: A thread-safe stack data structure for managing events_.

**************************************************/

#ifndef ATOM_SERVER_EVENTSTACK_INL
#define ATOM_SERVER_EVENTSTACK_INL

#include "eventstack.hpp"

#include <algorithm>
#include <mutex>

namespace atom::async {
template <typename T>
void EventStack<T>::pushEvent(T event) {
    std::unique_lock lock(mtx_);
    events_.push_back(std::move(event));
    ++eventCount_;
}

template <typename T>
auto EventStack<T>::popEvent() -> std::optional<T> {
    std::unique_lock lock(mtx_);
    if (!events_.empty()) {
        T event = std::move(events_.back());
        events_.pop_back();
        --eventCount_;
        return event;
    }
    return std::nullopt;
}

#if ENABLE_DEBUG
template <typename T>
void EventStack<T>::printEvents() const {
    std::shared_lock lock(mtx_);
    std::cout << "Events in stack:" << std::endl;
    for (const T& event : events) {
        std::cout << event << std::endl;
    }
}
#endif

template <typename T>
auto EventStack<T>::isEmpty() const -> bool {
    std::shared_lock lock(mtx_);
    return events_.empty();
}

template <typename T>
auto EventStack<T>::size() const -> size_t {
    return eventCount_.load();
}

template <typename T>
void EventStack<T>::clearEvents() {
    std::unique_lock lock(mtx_);
    events_.clear();
    eventCount_.store(0);
}

template <typename T>
auto EventStack<T>::peekTopEvent() const -> std::optional<T> {
    std::shared_lock lock(mtx_);
    if (!events_.empty()) {
        return events_.back();
    }
    return std::nullopt;
}

template <typename T>
auto EventStack<T>::copyStack() const -> EventStack<T> {
    std::shared_lock lock(mtx_);
    EventStack<T> newStack;
    newStack.events = events_;
    newStack.eventCount_.store(eventCount_.load());
    return newStack;
}

template <typename T>
void EventStack<T>::filterEvents(std::function<bool(const T&)> filterFunc) {
    std::unique_lock lock(mtx_);
    events_.erase(
        std::remove_if(events_.begin(), events_.end(),
                       [&](const T& event) { return !filterFunc(event); }),
        events_.end());
    eventCount_.store(events_.size());
}

template <typename T>
auto EventStack<T>::serializeStack() const -> std::string {
    std::shared_lock lock(mtx_);
    std::string serializedStack;
    for (const T& event : events_) {
        serializedStack += event + ";";
    }
    return serializedStack;
}

template <typename T>
void EventStack<T>::deserializeStack(std::string_view serializedData) {
    std::unique_lock lock(mtx_);
    events_.clear();
    size_t pos = 0;
    size_t nextPos = 0;
    while ((nextPos = serializedData.find(';', pos)) !=
           std::string_view::npos) {
        T event = serializedData.substr(pos, nextPos - pos);
        events_.push_back(std::move(event));
        pos = nextPos + 1;
    }
    eventCount_.store(events_.size());
}

template <typename T>
void EventStack<T>::removeDuplicates() {
    std::unique_lock lock(mtx_);
    std::sort(events_.begin(), events_.end());
    events_.erase(std::unique(events_.begin(), events_.end()), events_.end());
    eventCount_.store(events_.size());
}

template <typename T>
void EventStack<T>::sortEvents(
    std::function<bool(const T&, const T&)> compareFunc) {
    std::unique_lock lock(mtx_);
    std::sort(events_.begin(), events_.end(), compareFunc);
}

template <typename T>
void EventStack<T>::reverseEvents() {
    std::unique_lock lock(mtx_);
    std::reverse(events_.begin(), events_.end());
}

template <typename T>
auto EventStack<T>::countEvents(std::function<bool(const T&)> predicate) const
    -> size_t {
    std::shared_lock lock(mtx_);
    return std::count_if(events_.begin(), events_.end(), predicate);
}

template <typename T>
auto EventStack<T>::findEvent(std::function<bool(const T&)> predicate) const
    -> std::optional<T> {
    std::shared_lock lock(mtx_);
    auto it = std::find_if(events_.begin(), events_.end(), predicate);
    if (it != events_.end()) {
        return *it;
    }
    return std::nullopt;
}

template <typename T>
auto EventStack<T>::anyEvent(std::function<bool(const T&)> predicate) const
    -> bool {
    std::shared_lock lock(mtx_);
    return std::any_of(events_.begin(), events_.end(), predicate);
}

template <typename T>
auto EventStack<T>::allEvents(std::function<bool(const T&)> predicate) const
    -> bool {
    std::shared_lock lock(mtx_);
    return std::all_of(events_.begin(), events_.end(), predicate);
}
}  // namespace atom::async

#endif
