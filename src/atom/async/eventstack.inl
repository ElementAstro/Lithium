/*
 * eventstack_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-26

Description: A thread-safe stack data structure for managing events.

**************************************************/

#ifndef ATOM_SERVER_EVENTSTACK_INL
#define ATOM_SERVER_EVENTSTACK_INL

#include "eventstack.hpp"

template <typename T>
EventStack<T>::EventStack() {}

template <typename T>
EventStack<T>::~EventStack() {}

template <typename T>
void EventStack<T>::pushEvent(T event) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    events.push_back(std::move(event));
    ++eventCount;
}

template <typename T>
std::optional<T> EventStack<T>::popEvent() {
    std::unique_lock<std::shared_mutex> lock(mtx);
    if (!events.empty()) {
        T event = std::move(events.back());
        events.pop_back();
        --eventCount;
        return event;
    }
    return std::nullopt;
}

template <typename T>
void EventStack<T>::printEvents() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    std::cout << "Events in stack:" << std::endl;
    for (const T& event : events) {
        std::cout << event << std::endl;
    }
}

template <typename T>
bool EventStack<T>::isEmpty() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return events.empty();
}

template <typename T>
size_t EventStack<T>::size() const {
    return eventCount.load();
}

template <typename T>
void EventStack<T>::clearEvents() {
    std::unique_lock<std::shared_mutex> lock(mtx);
    events.clear();
    eventCount.store(0);
}

template <typename T>
std::optional<T> EventStack<T>::peekTopEvent() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    if (!events.empty()) {
        return events.back();
    }
    return std::nullopt;
}

template <typename T>
EventStack<T> EventStack<T>::copyStack() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    EventStack<T> newStack;
    newStack.events = events;
    newStack.eventCount.store(eventCount.load());
    return newStack;
}

template <typename T>
void EventStack<T>::filterEvents(std::function<bool(const T&)> filterFunc) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    events.erase(
        std::remove_if(events.begin(), events.end(),
                       [&](const T& event) { return !filterFunc(event); }),
        events.end());
    eventCount.store(events.size());
}

template <typename T>
std::string EventStack<T>::serializeStack() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    std::string serializedStack;
    for (const T& event : events) {
        serializedStack += event + ";";
    }
    return serializedStack;
}

template <typename T>
void EventStack<T>::deserializeStack(std::string_view serializedData) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    events.clear();
    size_t pos = 0;
    size_t nextPos = 0;
    while ((nextPos = serializedData.find(";", pos)) !=
           std::string_view::npos) {
        T event = serializedData.substr(pos, nextPos - pos);
        events.push_back(std::move(event));
        pos = nextPos + 1;
    }
    eventCount.store(events.size());
}

template <typename T>
void EventStack<T>::removeDuplicates() {
    std::unique_lock<std::shared_mutex> lock(mtx);
    std::sort(events.begin(), events.end());
    events.erase(std::unique(events.begin(), events.end()), events.end());
    eventCount.store(events.size());
}

template <typename T>
void EventStack<T>::sortEvents(
    std::function<bool(const T&, const T&)> compareFunc) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    std::sort(events.begin(), events.end(), compareFunc);
}

template <typename T>
void EventStack<T>::reverseEvents() {
    std::unique_lock<std::shared_mutex> lock(mtx);
    std::reverse(events.begin(), events.end());
}

template <typename T>
size_t EventStack<T>::countEvents(
    std::function<bool(const T&)> predicate) const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return std::count_if(events.begin(), events.end(), predicate);
}

template <typename T>
std::optional<T> EventStack<T>::findEvent(
    std::function<bool(const T&)> predicate) const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    auto it = std::find_if(events.begin(), events.end(), predicate);
    if (it != events.end()) {
        return *it;
    }
    return std::nullopt;
}

template <typename T>
bool EventStack<T>::anyEvent(std::function<bool(const T&)> predicate) const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return std::any_of(events.begin(), events.end(), predicate);
}

template <typename T>
bool EventStack<T>::allEvents(std::function<bool(const T&)> predicate) const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return std::all_of(events.begin(), events.end(), predicate);
}

#endif
