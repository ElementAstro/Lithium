/*
 * object.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-5

Description: A simple implementation of object pool

**************************************************/

#ifndef ATOM_MEMORY_OBJECT_POOL_HPP
#define ATOM_MEMORY_OBJECT_POOL_HPP

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
     * @param creator A function to create new objects. Defaults to
     * std::make_shared<T>().
     */
    explicit ObjectPool(
        size_t max_size,
        CreateFunc creator = []() { return std::make_shared<T>(); })
        : max_size_(max_size),
          available_(max_size),
          creator_(std::move(creator)) {
        assert(max_size_ > 0 && "ObjectPool size must be greater than zero.");
        pool_.reserve(max_size_);
    }

    /**
     * @brief Acquires an object from the pool. Blocks if no objects are
     * available.
     *
     * @return A shared pointer to the acquired object.
     * @throw std::runtime_error If the pool is full and no object is available.
     */
    [[nodiscard]] auto acquire() -> std::shared_ptr<T> {
        std::unique_lock lock(mutex_);

        if (available_ == 0 && pool_.empty()) {
            THROW_INVALID_ARGUMENT("ObjectPool is full.");
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
     */
    template <typename Rep, typename Period>
    [[nodiscard]] auto acquireFor(
        const std::chrono::duration<Rep, Period>& timeout_duration)
        -> std::optional<std::shared_ptr<T>> {
        std::unique_lock lock(mutex_);

        if (available_ == 0 && pool_.empty()) {
            THROW_INVALID_ARGUMENT("ObjectPool is full.");
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
    [[nodiscard]] auto available() const -> size_t {
        std::lock_guard lock(mutex_);
        return available_ + pool_.size();
    }

    /**
     * @brief Returns the current size of the pool.
     *
     * @return The current number of objects in the pool.
     */
    [[nodiscard]] auto size() const -> size_t {
        std::lock_guard lock(mutex_);
        return max_size_ - available_ + pool_.size();
    }

    /**
     * @brief Prefills the pool with a specified number of objects.
     *
     * @param count The number of objects to prefill the pool with.
     */
    void prefill(size_t count) {
        std::unique_lock lock(mutex_);
        for (size_t i = pool_.size(); i < count && i < max_size_; ++i) {
            pool_.push_back(creator_());
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
     */
    void resize(size_t new_max_size) {
        std::unique_lock lock(mutex_);
        if (new_max_size < max_size_) {
            pool_.erase(pool_.begin() + new_max_size, pool_.end());
        }
        max_size_ = new_max_size;
        available_ = std::max(available_, max_size_ - pool_.size());
        pool_.reserve(max_size_);
    }

    /**
     * @brief Applies a function to all objects in the pool.
     *
     * @param func The function to apply to each object.
     */
    void applyToAll(const std::function<void(T&)>& func) {
        std::unique_lock lock(mutex_);
        std::for_each(pool_, [&func](const auto& obj) { func(*obj); });
    }

private:
    [[nodiscard]] auto acquireImpl() -> std::shared_ptr<T> {
        if (!pool_.empty()) {
            auto obj = std::move(pool_.back());
            pool_.pop_back();
            return obj;
        }

        assert(available_ > 0);
        --available_;
        return creator_();
    }

    size_t max_size_;
    size_t available_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::shared_ptr<T>> pool_;
    CreateFunc creator_;
};

#endif  // ATOM_MEMORY_OBJECT_POOL_HPP
