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

#include <algorithm>
#include <atomic>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace atom::async {
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    void put(T element) {
        {
            std::lock_guard lock(m_mutex_);
            m_queue_.push(std::move(element));
        }
        m_conditionVariable_.notify_one();
    }

    auto take() -> std::optional<T> {
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

    auto destroy() -> std::queue<T> {
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

    [[nodiscard]] auto size() const -> size_t {
        std::lock_guard lock(m_mutex_);
        return m_queue_.size();
    }

    [[nodiscard]] auto empty() const -> bool {
        std::lock_guard lock(m_mutex_);
        return m_queue_.empty();
    }

    void clear() {
        std::lock_guard lock(m_mutex_);
        std::queue<T> empty;
        std::swap(m_queue_, empty);
    }

    auto front() -> std::optional<T> {
        std::lock_guard lock(m_mutex_);
        if (m_queue_.empty()) {
            return std::nullopt;
        }
        return m_queue_.front();
    }

    auto back() -> std::optional<T> {
        std::lock_guard lock(m_mutex_);
        if (m_queue_.empty()) {
            return std::nullopt;
        }
        return m_queue_.back();
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        {
            std::lock_guard lock(m_mutex_);
            m_queue_.emplace(std::forward<Args>(args)...);
        }
        m_conditionVariable_.notify_one();
    }

    template <std::predicate<const T&> Predicate>
    auto waitFor(Predicate predicate) -> std::optional<T> {
        std::unique_lock lock(m_mutex_);
        m_conditionVariable_.wait(lock, [this, &predicate] {
            return m_mustReturnNullptr_ ||
                   (!m_queue_.empty() && predicate(m_queue_.front()));
        });

        if (m_mustReturnNullptr_)
            return std::nullopt;

        T ret = std::move(m_queue_.front());
        m_queue_.pop();

        return ret;
    }

    void waitUntilEmpty() {
        std::unique_lock lock(m_mutex_);
        m_conditionVariable_.wait(
            lock, [this] { return m_mustReturnNullptr_ || m_queue_.empty(); });
    }

    template <std::predicate<const T&> UnaryPredicate>
    auto extractIf(UnaryPredicate pred) -> std::vector<T> {
        std::vector<T> result;
        {
            std::lock_guard lock(m_mutex_);
            std::queue<T> remaining;
            while (!m_queue_.empty()) {
                T& item = m_queue_.front();
                if (pred(item)) {
                    result.push_back(std::move(item));
                } else {
                    remaining.push(std::move(item));
                }
                m_queue_.pop();
            }
            std::swap(m_queue_, remaining);
        }
        return result;
    }

    template <typename Compare>
        requires std::is_invocable_r_v<bool, Compare, const T&, const T&>
    void sort(Compare comp) {
        std::lock_guard lock(m_mutex_);
        std::vector<T> temp;
        temp.reserve(m_queue_.size());
        while (!m_queue_.empty()) {
            temp.push_back(std::move(m_queue_.front()));
            m_queue_.pop();
        }
        std::sort(temp.begin(), temp.end(), comp);
        for (auto& elem : temp) {
            m_queue_.push(std::move(elem));
        }
    }

    template <typename ResultType>
    auto transform(std::function<ResultType(T)> func)
        -> std::shared_ptr<ThreadSafeQueue<ResultType>> {
        std::shared_ptr<ThreadSafeQueue<ResultType>> resultQueue;
        {
            std::lock_guard lock(m_mutex_);
            std::vector<T> original;
            original.reserve(m_queue_.size());

            while (!m_queue_.empty()) {
                original.push_back(std::move(m_queue_.front()));
                m_queue_.pop();
            }

            std::vector<ResultType> transformed(original.size());
            std::transform(original.begin(), original.end(),
                           transformed.begin(), func);

            for (auto& item : transformed) {
                resultQueue->put(std::move(item));
            }
        }
        return resultQueue;
    }

    template <typename GroupKey>
    auto groupBy(std::function<GroupKey(const T&)> func)
        -> std::vector<std::shared_ptr<ThreadSafeQueue<T>>> {
        std::unordered_map<GroupKey, std::shared_ptr<ThreadSafeQueue<T>>>
            resultMap;
        {
            std::lock_guard lock(m_mutex_);
            while (!m_queue_.empty()) {
                T item = std::move(m_queue_.front());
                m_queue_.pop();
                GroupKey key = func(item);
                if (!resultMap.contains(key)) {
                    resultMap[key] = std::make_shared<ThreadSafeQueue<T>>();
                }
                resultMap[key]->put(std::move(item));
            }
        }

        std::vector<std::shared_ptr<ThreadSafeQueue<T>>> resultQueues;
        resultQueues.reserve(resultMap.size());
        for (auto& [_, queue_ptr] : resultMap) {
            resultQueues.push_back(queue_ptr);
        }

        return resultQueues;
    }

    auto toVector() const -> std::vector<T> {
        std::lock_guard lock(m_mutex_);
        return std::vector<T>(m_queue_.front(), m_queue_.back());
    }

    template <typename Func>
        requires std::is_invocable_r_v<void, Func, T&>
    void forEach(Func func, bool parallel = false) {
        std::lock_guard lock(m_mutex_);
        if (parallel) {
            std::vector<T> vec;
            vec.reserve(m_queue_.size());
            while (!m_queue_.empty()) {
                vec.push_back(std::move(m_queue_.front()));
                m_queue_.pop();
            }

#pragma omp parallel for
            for (size_t i = 0; i < vec.size(); ++i) {
                func(vec[i]);
            }

            for (auto& item : vec) {
                m_queue_.push(std::move(item));
            }
        } else {
            std::queue<T> tempQueue;
            while (!m_queue_.empty()) {
                T& item = m_queue_.front();
                func(item);
                tempQueue.push(std::move(item));
                m_queue_.pop();
            }
            m_queue_ = std::move(tempQueue);
        }
    }

    auto tryTake() -> std::optional<T> {
        std::lock_guard lock(m_mutex_);
        if (m_queue_.empty()) {
            return std::nullopt;
        }
        T ret = std::move(m_queue_.front());
        m_queue_.pop();
        return ret;
    }

    template <typename Rep, typename Period>
    auto takeFor(const std::chrono::duration<Rep, Period>& timeout)
        -> std::optional<T> {
        std::unique_lock lock(m_mutex_);
        if (m_conditionVariable_.wait_for(lock, timeout, [this] {
                return !m_queue_.empty() || m_mustReturnNullptr_;
            })) {
            if (m_mustReturnNullptr_) {
                return std::nullopt;
            }
            T ret = std::move(m_queue_.front());
            m_queue_.pop();
            return ret;
        }
        return std::nullopt;
    }

    template <typename Clock, typename Duration>
    auto takeUntil(const std::chrono::time_point<Clock, Duration>& timeout_time)
        -> std::optional<T> {
        std::unique_lock lock(m_mutex_);
        if (m_conditionVariable_.wait_until(lock, timeout_time, [this] {
                return !m_queue_.empty() || m_mustReturnNullptr_;
            })) {
            if (m_mustReturnNullptr_) {
                return std::nullopt;
            }
            T ret = std::move(m_queue_.front());
            m_queue_.pop();
            return ret;
        }
        return std::nullopt;
    }

private:
    std::queue<T> m_queue_;
    mutable std::mutex m_mutex_;
    std::condition_variable m_conditionVariable_;
    std::atomic<bool> m_mustReturnNullptr_{false};
};

}  // namespace atom::async

#endif  // ATOM_ASYNC_QUEUE_HPP
