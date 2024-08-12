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
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <utility>
#include "type/noncopyable.hpp"

template <typename T>
class ThreadLocal : public NonCopyable {
public:
    using InitializerFn = std::function<T()>;

    ThreadLocal() = default;

    explicit ThreadLocal(InitializerFn initializer)
        : initializer_(std::move(initializer)) {}

    ThreadLocal(ThreadLocal&&) = default;
    auto operator=(ThreadLocal&&) -> ThreadLocal& = default;

    auto get() -> T& {
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

    auto operator->() -> T* { return &get(); }
    auto operator->() const -> const T* { return &get(); }

    auto operator*() -> T& { return get(); }
    auto operator*() const -> const T& { return get(); }

    void reset(T value = T()) {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        values_[tid] = std::make_optional(std::move(value));
    }

    auto hasValue() const -> bool {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        auto it = values_.find(tid);
        return it != values_.end() && it->second.has_value();
    }

    auto getPointer() -> T* {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        auto it = values_.find(tid);
        return it != values_.end() && it->second.has_value()
                   ? &it->second.value()
                   : nullptr;
    }

    auto getPointer() const -> const T* {
        auto tid = std::this_thread::get_id();
        std::lock_guard lock(mutex_);
        auto it = values_.find(tid);
        return it != values_.end() && it->second.has_value()
                   ? &it->second.value()
                   : nullptr;
    }

    template <typename Func>
    void forEach(Func&& func) {
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
