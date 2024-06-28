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
#include <mutex>
#include <optional>
#include <queue>
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
    auto take() -> std::optional<T>;

    /**
     * @brief Removes all elements from the queue and returns them in a
     * std::queue.
     * @return A std::queue containing all the elements removed from the queue.
     */
    auto destroy() -> std::queue<T>;

    /**
     * @brief Returns the number of elements currently in the queue.
     * @return The number of elements in the queue.
     */
    [[nodiscard]] auto size() const -> size_t;

    /**
     * @brief Checks if the queue is empty.
     * @return True if the queue is empty, false otherwise.
     */
    [[nodiscard]] auto empty() const -> bool;

    /**
     * @brief Removes all elements from the queue.
     */
    void clear();

    /**
     * @brief Returns the element at the front of the queue without removing it.
     * @return An optional containing the element at the front of the queue, or
     * empty if the queue is empty.
     */
    auto front() -> std::optional<T>;

    /**
     * @brief Returns the element at the back of the queue without removing it.
     * @return An optional containing the element at the back of the queue, or
     * empty if the queue is empty.
     */
    auto back() -> std::optional<T>;

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
    auto waitFor(Predicate predicate) -> std::optional<T>;

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
    auto extractIf(UnaryPredicate pred) -> std::vector<T>;

    /**
     * @brief Sorts the elements in the queue using a custom comparison
     * function.
     * @tparam Compare The type of comparison function or functor.
     * @param comp The comparison function used to sort elements.
     */
    template <typename Compare>
    void sort(Compare comp);

private:
    std::queue<T> m_queue_;       ///< The underlying queue.
    mutable std::mutex m_mutex_;  ///< Mutex for ensuring thread safety.
    std::condition_variable
        m_conditionVariable_;  ///< Condition variable for blocking and waking
                               ///< threads.
    std::atomic<bool> m_mustReturnNullptr_{
        false};  ///< Atomic flag indicating whether the queue should return
                 ///< nullptr on take() when empty.
};

template <typename T>
void ThreadSafeQueue<T>::put(T element) {
    {
        std::lock_guard lock(m_mutex_);
        m_queue_.push(std::move(element));
    }
    m_conditionVariable_.notify_one();
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::take() {
    std::unique_lock lock(m_mutex_);
    m_conditionVariable_.wait(
        lock, [this] { return m_mustReturnNullptr_ || !m_queue_.empty(); });

    if (m_mustReturnNullptr_) {
        return std::nullopt;
    }

    T ret = std::move(m_queue_.front());
    m_queue_.pop();

    return ret;
}

template <typename T>
std::queue<T> ThreadSafeQueue<T>::destroy() {
    {
        std::lock_guard lock(m_mutex_);
        m_mustReturnNullptr_ = true;
    }
    m_conditionVariable_.notify_all();

    std::queue<T> result;
    {
        std::lock_guard lock(m_mutex_);
        std::swap(result, m_queue_);
    }
    return result;
}

template <typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard lock(m_mutex_);
    return m_queue_.size();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard lock(m_mutex_);
    return m_queue_.empty();
}

template <typename T>
void ThreadSafeQueue<T>::clear() {
    std::lock_guard lock(m_mutex_);
    std::queue<T> empty;
    std::swap(m_queue_, empty);
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::front() {
    std::lock_guard lock(m_mutex_);
    if (m_queue_.empty()) {
        return std::nullopt;
    }
    return m_queue_.front();
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::back() {
    std::lock_guard lock(m_mutex_);
    if (m_queue_.empty()) {
        return std::nullopt;
    }
    return m_queue_.back();
}

template <typename T>
template <typename... Args>
void ThreadSafeQueue<T>::emplace(Args&&... args) {
    {
        std::lock_guard lock(m_mutex_);
        m_queue_.emplace(std::forward<Args>(args)...);
    }
    m_conditionVariable_.notify_one();
}

template <typename T>
template <typename Predicate>
std::optional<T> ThreadSafeQueue<T>::waitFor(Predicate predicate) {
    std::unique_lock lock(m_mutex_);
    m_conditionVariable_.wait(lock, [this, &predicate] {
        return m_mustReturnNullptr_ || predicate(m_queue_);
    });

    if (m_mustReturnNullptr_)
        return std::nullopt;

    T ret = std::move(m_queue_.front());
    m_queue_.pop();

    return ret;
}

template <typename T>
void ThreadSafeQueue<T>::waitUntilEmpty() {
    std::unique_lock lock(m_mutex_);
    m_conditionVariable_.wait(
        lock, [this] { return m_mustReturnNullptr_ || m_queue_.empty(); });
}

template <typename T>
template <typename UnaryPredicate>
std::vector<T> ThreadSafeQueue<T>::extractIf(UnaryPredicate pred) {
    std::vector<T> result;
    {
        std::lock_guard lock(m_mutex_);
        auto it = std::remove_if(
            m_queue_.front(), m_queue_.back(), [&](const T& item) {
                if (pred(item)) {
                    result.push_back(std::move(const_cast<T&>(item)));
                    return true;
                }
                return false;
            });
        m_queue_.pop();
    }
    return result;
}

template <typename T>
template <typename Compare>
void ThreadSafeQueue<T>::sort(Compare comp) {
    std::lock_guard lock(m_mutex_);

    // 移动元素到临时向量并排序
    std::vector<T> temp;
    while (!m_queue_.empty()) {
        temp.push_back(std::move(m_queue_.front()));
        m_queue_.pop();
    }
    std::sort(temp.begin(), temp.end(), comp);

    // 将排序后的元素移动到新的队列中
    std::queue<T> newQueue;
    for (auto& elem : temp) {
        newQueue.push(std::move(elem));
    }

    // 交换新旧队列
    std::swap(m_queue_, newQueue);
}

/*
template <typename T>
template <typename ResultType>
ThreadSafeQueue<ResultType> ThreadSafeQueue<T>::transform(
    std::function<ResultType(T)> func) {
    ThreadSafeQueue<ResultType> resultQueue;
    {
        std::lock_guard lock(m_mutex_);
        std::transform(std::make_move_iterator(m_queue_.front()),
                       std::make_move_iterator(m_queue_.back()),
                       std::back_inserter(resultQueue.m_queue), func);
        std::queue<T> empty;
        std::swap(m_queue, empty);
    }
    // return resultQueue;
}

template <typename T>
template <typename GroupKey>
std::vector<ThreadSafeQueue<T>> ThreadSafeQueue<T>::groupBy(
    std::function<GroupKey(T)> func) {
    std::unordered_map<GroupKey, ThreadSafeQueue<T>> resultMap;
    {
        std::lock_guard lock(m_mutex_);
        while (!m_queue_.empty()) {
            T item = std::move(m_queue_.front());
            m_queue_.pop();
            GroupKey key = func(item);
            resultMap[key].put(std::move(item));
        }
    }

    std::vector<ThreadSafeQueue<T>> resultQueues;
    resultQueues.reserve(resultMap.size());
    for (auto& pair : resultMap) {
        resultQueues.push_back(std::move(pair.second));
    }

    return resultQueues;
}
*/
}  // namespace atom::async
#endif
