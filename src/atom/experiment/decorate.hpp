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

template <typename F>
struct decorator
{
    decorator(F f) : func(f) {}

    template <typename... Args>
    auto operator()(Args &&...args) const
    {
        if (before)
            before();
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func(std::forward<Args>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        if (callback)
            callback(result);
        if (after)
            after(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        return result;
    }

    template <typename Before, typename Callback, typename After>
    decorator<F> with_hooks(Before b, Callback c, After a) const
    {
        decorator<F> copy(func);
        copy.before = b;
        copy.callback = c;
        copy.after = a;
        return copy;
    }

    std::function<void()> before = nullptr;
    std::function<void(int)> callback = nullptr;
    std::function<void(long long)> after = nullptr;
    F func;
};

template <typename F>
decorator<F> make_decorator(F f)
{
    return decorator<F>(f);
}

#endif
