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

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <utility>

template <typename T>
class ThreadLocal {
public:
    using InitializerFn = std::function<T()>;

    ThreadLocal() = default;

    explicit ThreadLocal(InitializerFn initializer)
        : initializer_(std::move(initializer)) {}

    ThreadLocal(const ThreadLocal&) = delete;
    ThreadLocal& operator=(const ThreadLocal&) = delete;

    ThreadLocal(ThreadLocal&&) = default;
    ThreadLocal& operator=(ThreadLocal&&) = default;

    T& get() {
        auto tid = std::this_thread::get_id();
        std::unique_lock<std::mutex> lock(mutex_);
        auto [it, inserted] = values_.try_emplace(tid);
        if (inserted) {
            it->second = initializer_ ? std::make_optional(initializer_())
                                      : std::nullopt;
        }
        lock.unlock();
        return it->second.value();
    }

    T* operator->() { return &get(); }
    const T* operator->() const { return &get(); }

    T& operator*() { return get(); }
    const T& operator*() const { return get(); }

    void reset(T value = T()) {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        values_[tid] = std::make_optional(std::move(value));
    }

    bool has_value() const {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        auto it = values_.find(tid);
        return it != values_.end() && it->second.has_value();
    }

    T* get_pointer() {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        auto it = values_.find(tid);
        return it != values_.end() && it->second.has_value()
                   ? &it->second.value()
                   : nullptr;
    }

    const T* get_pointer() const {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        auto it = values_.find(tid);
        return it != values_.end() && it->second.has_value()
                   ? &it->second.value()
                   : nullptr;
    }

    template <typename Func>
    void for_each(Func&& func) {
        std::lock_guard lock(mutex_);
        for (auto& [tid, value_opt] : values_) {
            if (value_opt.has_value()) {
                func(value_opt.value());
            }
        }
    }

    void clear() {
        std::lock_guard lock(mutex_);
        values_.clear();
    }

private:
    InitializerFn initializer_;
    mutable std::mutex mutex_;
    std::unordered_map<std::thread::id, std::optional<T>> values_;
};

#endif