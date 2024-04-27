#ifndef ATOM_ASYNC_QUEUE_INL
#define ATOM_ASYNC_QUEUE_INL

#include "queue.hpp"

#include <unordered_map>

namespace Atom::Async {
template <typename T>
void ThreadSafeQueue<T>::put(T element) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(element));
    }
    m_conditionVariable.notify_one();
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::take() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_conditionVariable.wait(
        lock, [this] { return m_mustReturnNullptr || !m_queue.empty(); });

    if (m_mustReturnNullptr)
        return std::nullopt;

    T ret = std::move(m_queue.front());
    m_queue.pop();

    return ret;
}

template <typename T>
std::queue<T> ThreadSafeQueue<T>::destroy() {
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

template <typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template <typename T>
void ThreadSafeQueue<T>::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<T> empty;
    std::swap(m_queue, empty);
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return std::nullopt;
    }
    return m_queue.front();
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::back() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return std::nullopt;
    }
    return m_queue.back();
}

template <typename T>
template <typename... Args>
void ThreadSafeQueue<T>::emplace(Args&&... args) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace(std::forward<Args>(args)...);
    }
    m_conditionVariable.notify_one();
}

template <typename T>
template <typename Predicate>
std::optional<T> ThreadSafeQueue<T>::waitFor(Predicate predicate) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_conditionVariable.wait(lock, [this, &predicate] {
        return m_mustReturnNullptr || predicate(m_queue);
    });

    if (m_mustReturnNullptr)
        return std::nullopt;

    T ret = std::move(m_queue.front());
    m_queue.pop();

    return ret;
}

template <typename T>
void ThreadSafeQueue<T>::waitUntilEmpty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_conditionVariable.wait(
        lock, [this] { return m_mustReturnNullptr || m_queue.empty(); });
}

template <typename T>
template <typename UnaryPredicate>
std::vector<T> ThreadSafeQueue<T>::extractIf(UnaryPredicate pred) {
    std::vector<T> result;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it =
            std::remove_if(m_queue.front(), m_queue.back(), [&](const T& item) {
                if (pred(item)) {
                    result.push_back(std::move(const_cast<T&>(item)));
                    return true;
                }
                return false;
            });
        m_queue.pop();
    }
    return result;
}

template <typename T>
template <typename Compare>
void ThreadSafeQueue<T>::sort(Compare comp) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 移动元素到临时向量并排序
    std::vector<T> temp;
    while (!m_queue.empty()) {
        temp.push_back(std::move(m_queue.front()));
        m_queue.pop();
    }
    std::sort(temp.begin(), temp.end(), comp);

    // 将排序后的元素移动到新的队列中
    std::queue<T> newQueue;
    for (auto& elem : temp) {
        newQueue.push(std::move(elem));
    }

    // 交换新旧队列
    std::swap(m_queue, newQueue);
}

/*
template <typename T>
template <typename ResultType>
ThreadSafeQueue<ResultType> ThreadSafeQueue<T>::transform(
    std::function<ResultType(T)> func) {
    ThreadSafeQueue<ResultType> resultQueue;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::transform(std::make_move_iterator(m_queue.front()),
                       std::make_move_iterator(m_queue.back()),
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
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            T item = std::move(m_queue.front());
            m_queue.pop();
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

}  // namespace Atom::Async

#endif