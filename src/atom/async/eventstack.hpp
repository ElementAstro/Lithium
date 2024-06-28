/*
 * eventstack.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-26

Description: A thread-safe stack data structure for managing events.

**************************************************/

#ifndef ATOM_SERVER_EVENTSTACK_HPP
#define ATOM_SERVER_EVENTSTACK_HPP

#include <atomic>
#include <functional>
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
}  // namespace atom::async

#include "eventstack.inl"

#endif  // ATOM_SERVER_EVENTSTACK_HPP
