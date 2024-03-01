/*
 * decorate.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: An implementation of decorate function. Just like Python's decorator.

**************************************************/

#ifndef ATOM_EXPERIMENT_DECORATE_HPP
#define ATOM_EXPERIMENT_DECORATE_HPP

#include <functional>
#include <chrono>
#include <utility>

template <typename FuncType>
struct decorator;

template <typename R, typename... Args>
struct decorator<std::function<R(Args...)>>
{
    using FuncType = std::function<R(Args...)>;

    decorator(FuncType f) : func(f) {}

    template <typename Before, typename Callback = std::function<void(R)>, typename After = std::function<void(long long)>>
    decorator<FuncType> with_hooks(
        Before b, Callback c = [](R) {}, After a = [](long long) {}) const
    {
        decorator<FuncType> copy(func);
        copy.before = b;
        copy.callback = c;
        copy.after = a;
        return copy;
    }

    template <typename T, typename... TArgs>
    auto operator()(T &obj, TArgs &&...args) const
    {
        if (before)
            before();
        auto start = std::chrono::high_resolution_clock::now();
        auto result = std::invoke(func, obj, std::forward<TArgs>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        if (callback)
            callback(result);
        if (after)
            after(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        return result;
    }

    template <typename... TArgs>
    auto operator()(TArgs &&...args) const
    {
        if (before)
            before();
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func(std::forward<TArgs>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        if (callback)
            callback(result);
        if (after)
            after(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        return result;
    }

    std::function<void()> before = nullptr;
    std::function<void(R)> callback = nullptr;
    std::function<void(long long)> after = nullptr;
    FuncType func;
};

template <typename F>
decorator<F> make_decorator(F f)
{
    return decorator<F>(f);
}

#endif
