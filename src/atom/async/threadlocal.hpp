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
        auto& value_ptr = values_[std::this_thread::get_id()];
        if (!value_ptr) {
            value_ptr =
                std::make_unique<T>(initializer_ ? initializer_() : T());
        }
        return *value_ptr;
    }

    T* operator->() { return &get(); }
    const T* operator->() const { return &get(); }

    T& operator*() { return get(); }
    const T& operator*() const { return get(); }

    // 重置当前线程的值
    void reset(T value = T()) {
        values_[std::this_thread::get_id()] =
            std::make_unique<T>(std::move(value));
    }

    // 判断当前线程是否有值
    bool has_value() const {
        return values_.count(std::this_thread::get_id()) > 0;
    }

    // 获取当前线程的值,如果没有值则返回nullptr
    T* get_pointer() {
        auto it = values_.find(std::this_thread::get_id());
        return it != values_.end() ? it->second.get() : nullptr;
    }

    const T* get_pointer() const {
        auto it = values_.find(std::this_thread::get_id());
        return it != values_.end() ? it->second.get() : nullptr;
    }

    // 遍历所有线程的值并执行指定操作
    template <typename Func>
    void for_each(Func&& func) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [tid, value_ptr] : values_) {
            func(*value_ptr);
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        values_.clear();
    }

private:
    InitializerFn initializer_;
    mutable std::mutex mutex_;
    std::unordered_map<std::thread::id, std::unique_ptr<T>> values_;
};

#endif