/*
 * object.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-5

Description: A simple implementation of object pool

**************************************************/

#ifndef ATOM_MEMORY_OBJECT_HPP
#define ATOM_MEMORY_OBJECT_HPP

#include <cassert>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

template <typename T>
concept Resettable = requires(T& obj) { obj.reset(); };

template <Resettable T>
class ObjectPool {
public:
    using CreateFunc = std::function<std::shared_ptr<T>()>;

    explicit ObjectPool(
        size_t max_size,
        CreateFunc creator = []() { return std::make_shared<T>(); })
        : max_size_(max_size),
          available_(max_size),
          creator_(std::move(creator)) {
        assert(max_size_ > 0 && "ObjectPool size must be greater than zero.");
        pool_.reserve(max_size_);
    }

    std::shared_ptr<T> acquire() {
        std::unique_lock lock(mutex_);

        if (available_ == 0) {
            throw std::runtime_error("ObjectPool is full.");
        }
        cv_.wait(lock, [this] { return !pool_.empty() || available_ > 0; });

        if (!pool_.empty()) {
            auto obj = pool_.back();
            pool_.pop_back();
            return obj;
        }

        assert(available_ > 0);
        --available_;
        return creator_();
    }

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

    size_t available() const {
        std::lock_guard lock(mutex_);
        return available_ + pool_.size();
    }

    size_t size() const {
        std::lock_guard lock(mutex_);
        return max_size_ - available_ + pool_.size();
    }

    void prefill(size_t count) {
        std::unique_lock lock(mutex_);
        for (size_t i = pool_.size(); i < count && i < max_size_; ++i) {
            pool_.push_back(creator_());
        }
    }

private:
    size_t max_size_;
    size_t available_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::shared_ptr<T>> pool_;
    CreateFunc creator_;
};

#endif  // ATOM_MEMORY_OBJECT_HPP
