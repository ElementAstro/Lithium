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

#include <string>
#include <mutex>
#include <vector>
#include <algorithm>
#include <memory>

/**
 * @brief A thread-safe stack data structure for managing events.
 * 
 * @tparam T The type of events to store.
 */
template <typename T>
class EventStack {
public:
    EventStack();  /**< Constructor. */
    ~EventStack(); /**< Destructor. */
    
    /**
     * @brief Pushes an event onto the stack.
     * 
     * @param event The event to push.
     */
    void pushEvent(const T& event);

    /**
     * @brief Pops an event from the stack.
     * 
     * @return The popped event.
     * @throw std::out_of_range if the stack is empty.
     */
    T popEvent();

    /**
     * @brief Prints all events in the stack.
     */
    void printEvents() const;

    /**
     * @brief Checks if the stack is empty.
     * 
     * @return true if the stack is empty, false otherwise.
     */
    bool isEmpty() const;

    /**
     * @brief Returns the number of events in the stack.
     * 
     * @return The number of events.
     */
    size_t size() const;

    /**
     * @brief Clears all events from the stack.
     */
    void clearEvents();

    /**
     * @brief Returns the top event in the stack without removing it.
     * 
     * @return The top event.
     * @throw std::out_of_range if the stack is empty.
     */
    T peekTopEvent() const;

    /**
     * @brief Copies the current stack.
     * 
     * @return A copy of the stack.
     */
    EventStack<T> copyStack() const;

    /**
     * @brief Filters events based on a custom filter function.
     * 
     * @param filterFunc The filter function.
     */
    void filterEvents(bool (*filterFunc)(const T&));

    /**
     * @brief Serializes the stack into a string.
     * 
     * @return The serialized stack.
     */
    std::string serializeStack() const;

    /**
     * @brief Deserializes a string into the stack.
     * 
     * @param serializedData The serialized stack data.
     */
    void deserializeStack(const std::string& serializedData);

private:
    class EventStackImpl; /**< Forward declaration of the implementation class. */
    std::unique_ptr<EventStackImpl> impl; /**< Pointer to the implementation. */
    mutable std::mutex mtx; /**< Mutex for thread safety. */
};

#include "eventstack_impl.hpp"

#endif // ATOM_SERVER_EVENTSTACK_HPP
