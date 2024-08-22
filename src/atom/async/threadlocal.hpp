/*
 * threadlocal.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-16

Description: ThreadLocal

**************************************************/

#ifndef ATOM_ASYNC_THREADLOCAL_HPP
#define ATOM_ASYNC_THREADLOCAL_HPP

#include <concepts>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <utility>

#include "atom/type/noncopyable.hpp"

namespace atom::async {
/**
 * @brief A thread-local storage class template that provides thread-specific
 * storage for objects of type T.
 *
 * This class allows each thread to maintain its own independent instance of T,
 * with optional initialization and a variety of access methods. It is not
 * copyable to ensure each instance is unique per thread.
 *
 * @tparam T The type of the value that will be stored in thread-local storage.
 */
template <typename T>
class ThreadLocal : public NonCopyable {
public:
    using InitializerFn =
        std::function<T()>;  ///< Type definition for the initializer function.

    /**
     * @brief Default constructor for ThreadLocal.
     *
     * Initializes an instance of ThreadLocal without an initializer.
     */
    ThreadLocal() = default;

    /**
     * @brief Constructs a ThreadLocal instance with an initializer function.
     *
     * @param initializer A function that is called to initialize the value the
     * first time it is accessed.
     */
    explicit ThreadLocal(InitializerFn initializer);

    // Move constructor
    ThreadLocal(ThreadLocal&&) noexcept = default;

    /**
     * @brief Move assignment operator.
     *
     * @param other The ThreadLocal instance to move from.
     * @return A reference to this instance after the move.
     */
    auto operator=(ThreadLocal&&) noexcept -> ThreadLocal& = default;

    /**
     * @brief Retrieves the thread-local value.
     *
     * If the value has not been initialized for the current thread, the
     * initializer function is called to create it.
     *
     * @return A reference to the thread-local value of type T.
     */
    auto get() -> T&;

    /**
     * @brief Access the thread-local value using the arrow operator.
     *
     * @return A pointer to the thread-local value of type T.
     */
    auto operator->() -> T*;

    /**
     * @brief Access the thread-local value using the arrow operator (const
     * version).
     *
     * @return A pointer to the thread-local value of type T (const version).
     */
    auto operator->() const -> const T*;

    /**
     * @brief Dereference the thread-local value.
     *
     * @return A reference to the thread-local value of type T.
     */
    auto operator*() -> T&;

    /**
     * @brief Dereference the thread-local value (const version).
     *
     * @return A const reference to the thread-local value of type T.
     */
    auto operator*() const -> const T&;

    /**
     * @brief Resets the value in thread-local storage.
     *
     * If a value is provided, it will be set to the thread-local value. If no
     * value is provided, the thread-local value will be reset to its default
     * constructed value.
     *
     * @param value The value to set; the default is T(), which is the default
     * constructed value of T.
     */
    void reset(T value = T());

    /**
     * @brief Checks if the current thread has a value.
     *
     * @return true if the current thread has an initialized value, otherwise
     * false.
     */
    auto hasValue() const -> bool;

    /**
     * @brief Retrieves a pointer to the thread-local value.
     *
     * If the value has not been initialized, this will return a nullptr.
     *
     * @return A pointer to the thread-local value of type T.
     */
    auto getPointer() -> T*;

    /**
     * @brief Retrieves a pointer to the thread-local value (const version).
     *
     * @return A const pointer to the thread-local value of type T.
     */
    auto getPointer() const -> const T*;

    /**
     * @brief Executes a function for each thread-local value.
     *
     * This allows the caller to provide a function that will be called with the
     * value of type T for each thread that has an initialized value.
     *
     * @tparam Func A callable type (e.g., a lambda or a function pointer) that
     * takes a reference to T.
     * @param func The function to execute for each thread-local value.
     */
    template <std::invocable<T&> Func>
    void forEach(Func&& func);

    /**
     * @brief Clears the thread-local storage for the current thread.
     *
     * This will remove the value associated with the current thread.
     */
    void clear();

private:
    InitializerFn initializer_;        ///< The function used to initialize T.
    mutable std::shared_mutex mutex_;  ///< Mutex for thread-safe access.
    std::unordered_map<std::thread::id, std::optional<T>>
        values_;  ///< Store values by thread ID.
};

template <typename T>
ThreadLocal<T>::ThreadLocal(InitializerFn initializer)
    : initializer_(std::move(initializer)) {}

template <typename T>
auto ThreadLocal<T>::get() -> T& {
    auto tid = std::this_thread::get_id();
    std::unique_lock lock(mutex_);
    auto [it, inserted] = values_.try_emplace(tid);
    if (inserted && initializer_) {
        it->second = std::make_optional(initializer_());
    }
    lock.unlock();
    return it->second.value();
}

template <typename T>
auto ThreadLocal<T>::operator->() -> T* {
    return &get();
}

template <typename T>
auto ThreadLocal<T>::operator->() const -> const T* {
    return &get();
}

template <typename T>
auto ThreadLocal<T>::operator*() -> T& {
    return get();
}

template <typename T>
auto ThreadLocal<T>::operator*() const -> const T& {
    return get();
}

template <typename T>
void ThreadLocal<T>::reset(T value) {
    auto tid = std::this_thread::get_id();
    std::unique_lock lock(mutex_);
    values_[tid] = std::make_optional(std::move(value));
}

template <typename T>
auto ThreadLocal<T>::hasValue() const -> bool {
    auto tid = std::this_thread::get_id();
    std::shared_lock lock(mutex_);
    auto it = values_.find(tid);
    return it != values_.end() && it->second.has_value();
}

template <typename T>
auto ThreadLocal<T>::getPointer() -> T* {
    auto tid = std::this_thread::get_id();
    std::shared_lock lock(mutex_);
    auto it = values_.find(tid);
    return it != values_.end() && it->second.has_value() ? &it->second.value()
                                                         : nullptr;
}

template <typename T>
auto ThreadLocal<T>::getPointer() const -> const T* {
    auto tid = std::this_thread::get_id();
    std::shared_lock lock(mutex_);
    auto it = values_.find(tid);
    return it != values_.end() && it->second.has_value() ? &it->second.value()
                                                         : nullptr;
}

template <typename T>
template <std::invocable<T&> Func>
void ThreadLocal<T>::forEach(Func&& func) {
    std::unique_lock lock(mutex_);
    for (auto& [tid, value_opt] : values_) {
        if (value_opt.has_value()) {
            func(value_opt.value());
        }
    }
}

template <typename T>
void ThreadLocal<T>::clear() {
    std::unique_lock lock(mutex_);
    values_.clear();
}

}  // namespace atom::async

#endif  // ATOM_ASYNC_THREADLOCAL_HPP
