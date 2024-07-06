/*
 * eventstack.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-26

Description: A thread-safe stack data structure for managing events.

**************************************************/

#ifndef ATOM_ASYNC_EVENTSTACK_HPP
#define ATOM_ASYNC_EVENTSTACK_HPP

#include <algorithm>
#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

namespace atom::async {
/**
 * @brief A thread-safe stack data structure for managing events.
 *
 * @tparam T The type of events to store.
 */
template <typename T>
class EventStack {
public:
    EventStack() = default;
    ~EventStack() = default;

    /**
     * @brief Pushes an event onto the stack.
     *
     * @param event The event to push.
     */
    void pushEvent(T event);

    /**
     * @brief Pops an event from the stack.
     *
     * @return The popped event, or std::nullopt if the stack is empty.
     */
    auto popEvent() -> std::optional<T>;

#if ENABLE_DEBUG
    /**
     * @brief Prints all events in the stack.
     */
    void printEvents() const;
#endif

    /**
     * @brief Checks if the stack is empty.
     *
     * @return true if the stack is empty, false otherwise.
     */
    auto isEmpty() const -> bool;

    /**
     * @brief Returns the number of events in the stack.
     *
     * @return The number of events.
     */
    auto size() const -> size_t;

    /**
     * @brief Clears all events from the stack.
     */
    void clearEvents();

    /**
     * @brief Returns the top event in the stack without removing it.
     *
     * @return The top event, or std::nullopt if the stack is empty.
     */
    auto peekTopEvent() const -> std::optional<T>;

    /**
     * @brief Copies the current stack.
     *
     * @return A copy of the stack.
     */
    auto copyStack() const -> EventStack<T>;

    /**
     * @brief Filters events based on a custom filter function.
     *
     * @param filterFunc The filter function.
     */
    void filterEvents(std::function<bool(const T&)> filterFunc);

    /**
     * @brief Serializes the stack into a string.
     *
     * @return The serialized stack.
     */
    auto serializeStack() const -> std::string;

    /**
     * @brief Deserializes a string into the stack.
     *
     * @param serializedData The serialized stack data.
     */
    void deserializeStack(std::string_view serializedData);

    /**
     * @brief Removes duplicate events from the stack.
     */
    void removeDuplicates();

    /**
     * @brief Sorts the events in the stack based on a custom comparison
     * function.
     *
     * @param compareFunc The comparison function.
     */
    void sortEvents(std::function<bool(const T&, const T&)> compareFunc);

    /**
     * @brief Reverses the order of events in the stack.
     */
    void reverseEvents();

    /**
     * @brief Counts the number of events that satisfy a predicate.
     *
     * @param predicate The predicate function.
     * @return The count of events satisfying the predicate.
     */
    auto countEvents(std::function<bool(const T&)> predicate) const -> size_t;

    /**
     * @brief Finds the first event that satisfies a predicate.
     * *
     * @param predicate The predicate function.
     * @return The first event satisfying the predicate, or std::nullopt if not
     * found.
     */
    auto findEvent(std::function<bool(const T&)> predicate) const
        -> std::optional<T>;

    /**
     * @brief Checks if any event in the stack satisfies a predicate.
     *
     * @param predicate The predicate function.
     * @return true if any event satisfies the predicate, false otherwise.
     */
    auto anyEvent(std::function<bool(const T&)> predicate) const -> bool;

    /**
     * @brief Checks if all events in the stack satisfy a predicate.
     *
     * @param predicate The predicate function.
     * @return true if all events satisfy the predicate, false otherwise.
     */
    auto allEvents(std::function<bool(const T&)> predicate) const -> bool;

private:
    std::vector<T> events_;             /**< Vector to store events. */
    mutable std::shared_mutex mtx_;     /**< Mutex for thread safety. */
    std::atomic<size_t> eventCount_{0}; /**< Atomic counter for event count. */
};

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

#endif  // ATOM_ASYNC_EVENTSTACK_HPP
