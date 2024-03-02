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

#include "atom/experiment/noncopyable.hpp"

namespace Atom::Async {
template <typename T>
struct ThreadSafeQueue : public NonCopyable {
    ThreadSafeQueue() = default;

    void put(T element) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(element));
        }
        m_conditionVariable.notify_one();
    }

    std::optional<T> take() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_conditionVariable.wait(
            lock, [this] { return m_mustReturnNullptr || !m_queue.empty(); });

        if (m_mustReturnNullptr)
            return {};

        T ret = std::move(m_queue.front());
        m_queue.pop();

        return ret;
    }

    std::queue<T> destroy() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_mustReturnNullptr = true;
        }
        m_conditionVariable.notify_all();

        std::queue<T> result;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(result, m_queue);
        }
        return result;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            m_queue.pop();
        }
    }

    std::optional<T> front() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return {};
        }
        return m_queue.front();
    }

    std::optional<T> back() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return {};
        }
        return m_queue.back();
    }

    template <typename... Args>
    void emplace(Args &&...args) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.emplace(std::forward<Args>(args)...);
        }
        m_conditionVariable.notify_one();
    }

    template <typename Predicate>
    std::optional<T> waitFor(Predicate predicate) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_conditionVariable.wait(lock, [this, &predicate] {
            return m_mustReturnNullptr || predicate(m_queue);
        });

        if (m_mustReturnNullptr)
            return {};

        T ret = std::move(m_queue.front());
        m_queue.pop();

        return ret;
    }

    void waitUntilEmpty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_conditionVariable.wait(
            lock, [this] { return m_mustReturnNullptr || m_queue.empty(); });
    }

    template <typename UnaryPredicate>
    std::vector<T> extractIf(UnaryPredicate pred) {
        std::vector<T> result;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_queue.begin();
            while (it != m_queue.end()) {
                if (pred(*it)) {
                    result.push_back(std::move(*it));
                    it = m_queue.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return result;
    }

    template <typename Compare>
    void sort(Compare comp) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<T> temp(std::make_move_iterator(m_queue.front()),
                            std::make_move_iterator(m_queue.back()));
        std::sort(temp.begin(), temp.end(), comp);
        std::queue<T> newQueue;
        for (auto &item : temp) {
            newQueue.push(std::move(item));
        }
        std::swap(m_queue, newQueue);
    }

    template <typename ResultType>
    ThreadSafeQueue<ResultType> transform(std::function<ResultType(T)> func) {
        ThreadSafeQueue<ResultType> resultQueue;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            while (!m_queue.empty()) {
                T item = std::move(m_queue.front());
                m_queue.pop();
                resultQueue.put(func(std::move(item)));
            }
        }
        return resultQueue;
    }

    template <typename GroupKey>
    std::vector<ThreadSafeQueue<T>> groupBy(std::function<GroupKey(T)> func) {
        std::unordered_map<GroupKey, ThreadSafeQueue<T>> resultMap;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            while (!m_queue.empty()) {
                T item = std::move(m_queue.front());
                m_queue.pop();
                GroupKey key = func(item);
                resultMap[key].put(std::move(item));
            }
        }

        std::vector<ThreadSafeQueue<T>> resultQueues;
        for (auto &pair : resultMap) {
            resultQueues.push_back(std::move(pair.second));
        }

        return resultQueues;
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_conditionVariable;

    std::atomic<bool> m_mustReturnNullptr{false};
};
}  // namespace Atom::Async

#endif
