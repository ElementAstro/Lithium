/*
 * object.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-04-05

Description: An enhanced implementation of object pool with
automatic object release, better exception handling, and additional
functionalities.

**************************************************/

#ifndef ATOM_MEMORY_OBJECT_POOL_HPP
#define ATOM_MEMORY_OBJECT_POOL_HPP

#include <algorithm>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "atom/error/exception.hpp"

template <typename T>
concept Resettable = requires(T& obj) { obj.reset(); };

namespace atom::memory {
/**
 * @brief A thread-safe object pool for managing reusable objects.
 *
 * @tparam T The type of objects managed by the pool. Must satisfy the
 * Resettable concept.
 */
template <Resettable T>
class ObjectPool {
public:
    using CreateFunc = std::function<std::shared_ptr<T>()>;

    /**
     * @brief Constructs an ObjectPool with a specified maximum size and an
     * optional custom object creator.
     *
     * @param max_size The maximum number of objects the pool can hold.
     * @param initial_size The initial number of objects to prefill the pool
     * with.
     * @param creator A function to create new objects. Defaults to
     * std::make_shared<T>().
     */
    explicit ObjectPool(
        size_t max_size, size_t initial_size = 0,
        CreateFunc creator = []() { return std::make_shared<T>(); })
        : max_size_(max_size),
          available_(max_size),
          creator_(std::move(creator)) {
        assert(max_size_ > 0 && "ObjectPool size must be greater than zero.");
        prefill(initial_size);
    }

    // 禁用拷贝和赋值
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    /**
     * @brief Acquires an object from the pool. Blocks if no objects are
     * available.
     *
     * @return A shared pointer to the acquired object with a custom deleter.
     * @throw std::runtime_error If the pool is full and no object is available.
     */
    [[nodiscard]] std::shared_ptr<T> acquire() {
        std::unique_lock lock(mutex_);

        if (available_ == 0 && pool_.empty()) {
            THROW_RUNTIME_ERROR("ObjectPool is full.");
        }

        cv_.wait(lock, [this] { return !pool_.empty() || available_ > 0; });

        return acquireImpl();
    }

    /**
     * @brief Acquires an object from the pool with a timeout.
     *
     * @param timeout_duration The maximum duration to wait for an available
     * object.
     * @return A shared pointer to the acquired object or nullptr if the timeout
     * expires.
     * @throw std::runtime_error If the pool is full and no object is available.
     */
    template <typename Rep, typename Period>
    [[nodiscard]] std::optional<std::shared_ptr<T>> tryAcquireFor(
        const std::chrono::duration<Rep, Period>& timeout_duration) {
        std::unique_lock lock(mutex_);

        if (available_ == 0 && pool_.empty()) {
            THROW_RUNTIME_ERROR("ObjectPool is full.");
        }

        if (!cv_.wait_for(lock, timeout_duration, [this] {
                return !pool_.empty() || available_ > 0;
            })) {
            return std::nullopt;
        }

        return acquireImpl();
    }

    /**
     * @brief Releases an object back to the pool.
     *
     * Note: This method is now private and managed automatically via the custom
     * deleter.
     *
     * @param obj The shared pointer to the object to release.
     */
    void release(std::shared_ptr<T> obj) {
        std::unique_lock lock(mutex_);
        if (pool_.size() < max_size_) {
            obj->reset();
            pool_.push_back(std::move(obj));
        } else {
            ++available_;
        }
        cv_.notify_one();
    }

    /**
     * @brief Returns the number of available objects in the pool.
     *
     * @return The number of available objects.
     */
    [[nodiscard]] size_t available() const {
        std::lock_guard lock(mutex_);
        return available_ + pool_.size();
    }

    /**
     * @brief Returns the current size of the pool.
     *
     * @return The current number of objects in the pool.
     */
    [[nodiscard]] size_t size() const {
        std::lock_guard lock(mutex_);
        return max_size_ - available_ + pool_.size();
    }

    /**
     * @brief Prefills the pool with a specified number of objects.
     *
     * @param count The number of objects to prefill the pool with.
     * @throw std::runtime_error If prefill exceeds the maximum pool size.
     */
    void prefill(size_t count) {
        std::unique_lock lock(mutex_);
        if (count > max_size_) {
            THROW_RUNTIME_ERROR("Prefill count exceeds maximum pool size.");
        }
        for (size_t i = pool_.size(); i < count; ++i) {
            pool_.emplace_back(creator_());
            --available_;
        }
    }

    /**
     * @brief Clears all objects from the pool.
     */
    void clear() {
        std::unique_lock lock(mutex_);
        pool_.clear();
        available_ = max_size_;
    }

    /**
     * @brief Resizes the pool to a new maximum size.
     *
     * @param new_max_size The new maximum size for the pool.
     * @throw std::runtime_error If the new size is smaller than the number of
     * prefilled objects.
     */
    void resize(size_t new_max_size) {
        std::unique_lock lock(mutex_);
        if (new_max_size < (max_size_ - available_)) {
            THROW_RUNTIME_ERROR(
                "New maximum size is smaller than the number of in-use "
                "objects.");
        }
        max_size_ = new_max_size;
        available_ = std::max(available_, max_size_ - pool_.size());
        pool_.reserve(max_size_);
        cv_.notify_all();
    }

    /**
     * @brief Applies a function to all objects in the pool.
     *
     * @param func The function to apply to each object.
     */
    void applyToAll(const std::function<void(T&)>& func) {
        std::unique_lock lock(mutex_);
        for (auto& objPtr : pool_) {
            func(*objPtr);
        }
    }

    /**
     * @brief Gets the current number of in-use objects.
     *
     * @return The number of in-use objects.
     */
    [[nodiscard]] size_t inUseCount() const {
        std::lock_guard lock(mutex_);
        return max_size_ - available_;
    }

private:
    /**
     * @brief Acquires an object from the pool and wraps it with a custom
     * deleter.
     *
     * @return A shared pointer to the acquired object with a custom deleter.
     */
    std::shared_ptr<T> acquireImpl() {
        std::shared_ptr<T> obj;
        if (!pool_.empty()) {
            obj = std::move(pool_.back());
            pool_.pop_back();
        } else {
            --available_;
            obj = creator_();
        }

        // 创建自定义删除器，确保对象在shared_ptr销毁时返回到对象池
        auto deleter = [this](T* ptr) {
            std::shared_ptr<T> sharedPtrObj(ptr, [](T*) {
                // 自定义删除器为空，防止shared_ptr尝试删除对象
            });
            release(sharedPtrObj);
        };

        // 返回带有自定义删除器的shared_ptr
        return std::shared_ptr<T>(obj.get(), deleter);
    }

    size_t max_size_;
    size_t available_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::shared_ptr<T>> pool_;
    CreateFunc creator_;
};

}  // namespace atom::memory

#endif  // ATOM_MEMORY_OBJECT_POOL_HPP