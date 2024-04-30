/*
 * queue.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-13

Description: A simple thread safe queue

**************************************************/

#ifndef ATOM_ASYNC_QUEUE_HPP
#define ATOM_ASYNC_QUEUE_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <vector>

#include "atom/type/noncopyable.hpp"

namespace atom::async {
/**
 * @brief A thread-safe queue data structure that supports concurrent access
 * from multiple threads.
 *
 * This class provides a thread-safe implementation of a queue, allowing
 * elements to be added and removed concurrently from multiple threads without
 * causing data races or undefined behavior.
 *
 * @tparam T The type of elements stored in the queue.
 */
template <typename T>
class ThreadSafeQueue : public NonCopyable {
public:
    /**
     * @brief Default constructor.
     */
    ThreadSafeQueue() = default;

    /**
     * @brief Adds an element to the end of the queue.
     * @param element The element to be added to the queue.
     */
    void put(T element);

    /**
     * @brief Removes and returns an element from the front of the queue.
     * @return An optional containing the removed element, or empty if the queue
     * is empty.
     */
    std::optional<T> take();

    /**
     * @brief Removes all elements from the queue and returns them in a
     * std::queue.
     * @return A std::queue containing all the elements removed from the queue.
     */
    std::queue<T> destroy();

    /**
     * @brief Returns the number of elements currently in the queue.
     * @return The number of elements in the queue.
     */
    [[nodiscard]] size_t size() const;

    /**
     * @brief Checks if the queue is empty.
     * @return True if the queue is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const;

    /**
     * @brief Removes all elements from the queue.
     */
    void clear();

    /**
     * @brief Returns the element at the front of the queue without removing it.
     * @return An optional containing the element at the front of the queue, or
     * empty if the queue is empty.
     */
    std::optional<T> front();

    /**
     * @brief Returns the element at the back of the queue without removing it.
     * @return An optional containing the element at the back of the queue, or
     * empty if the queue is empty.
     */
    std::optional<T> back();

    /**
     * @brief Constructs and adds an element to the end of the queue.
     * @tparam Args The types of arguments used to construct the element.
     * @param args The arguments used to construct the element.
     */
    template <typename... Args>
    void emplace(Args&&... args);

    /**
     * @brief Waits until a predicate becomes true for an element in the queue,
     * then removes and returns that element.
     * @tparam Predicate The type of predicate function or functor.
     * @param predicate The predicate to wait for.
     * @return An optional containing the element that satisfied the predicate,
     * or empty if the queue is destroyed or the timeout expires.
     */
    template <typename Predicate>
    std::optional<T> waitFor(Predicate predicate);

    /**
     * @brief Blocks until the queue becomes empty.
     */
    void waitUntilEmpty();

    /**
     * @brief Removes and returns all elements from the queue that satisfy a
     * given unary predicate.
     * @tparam UnaryPredicate The type of unary predicate function or functor.
     * @param pred The unary predicate used to test elements.
     * @return A vector containing all elements removed from the queue that
     * satisfy the predicate.
     */
    template <typename UnaryPredicate>
    std::vector<T> extractIf(UnaryPredicate pred);

    /**
     * @brief Sorts the elements in the queue using a custom comparison
     * function.
     * @tparam Compare The type of comparison function or functor.
     * @param comp The comparison function used to sort elements.
     */
    template <typename Compare>
    void sort(Compare comp);

private:
    std::queue<T> m_queue;       ///< The underlying queue.
    mutable std::mutex m_mutex;  ///< Mutex for ensuring thread safety.
    std::condition_variable
        m_conditionVariable;  ///< Condition variable for blocking and waking
                              ///< threads.
    std::atomic<bool> m_mustReturnNullptr{
        false};  ///< Atomic flag indicating whether the queue should return
                 ///< nullptr on take() when empty.
};

}  // namespace atom::async

#include "queue.inl"

#endif
