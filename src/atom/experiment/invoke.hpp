/*
 * invoke.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: An implementation of invoke function. Support C++11 and C++17.

**************************************************/

#ifndef ATOM_EXPERIMENTAL_INVOKE_HPP
#define ATOM_EXPERIMENTAL_INVOKE_HPP

#include <exception>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <utility>


#if __cplusplus >= 201703L

#include <type_traits>

template <typename F, typename... Args>
concept Invocable = std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>;

template <typename F, typename... Args>
auto delay_invoke(F &&f, Args &&...args) {
    static_assert(Invocable<F, Args...>, "F must be callable with Args...");

    return [f = std::forward<F>(f),
            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(std::move(f), std::move(args));
    };
}

template <typename Func, typename... Args>
std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...> safe_call(
    Func &&func, Args &&...args) {
    try {
        return std::invoke(std::forward<Func>(func),
                           std::forward<Args>(args)...);
    } catch (...) {
        using ReturnType =
            std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
        if constexpr (std::is_default_constructible_v<ReturnType>) {
            return ReturnType{};
        } else {
            throw std::runtime_error("An exception occurred in safe_call");
        }
    }
}

template <typename Func, typename... Args>
auto safe_try_catch(Func &&func, Args &&...args) {
    try {
        return std::invoke(std::forward<Func>(func),
                           std::forward<Args>(args)...);
    } catch (...) {
        if constexpr (std::is_void_v<std::invoke_result_t<
                          std::decay_t<Func>, std::decay_t<Args>...>>) {
            std::rethrow_exception(std::current_exception());
        } else
            return std::make_tuple(std::current_exception());
    }
}

template <typename Func, typename... Args>
auto safe_try_catch_or_default(
    Func &&func,
    std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>
        default_value,
    Args &&...args) {
    try {
        return std::invoke(std::forward<Func>(func),
                           std::forward<Args>(args)...);
    } catch (...) {
        return default_value;
    }
}

#else
template <std::size_t... I>
struct index_sequence {};

template <std::size_t N, std::size_t... I>
struct make_index_sequence_impl : make_index_sequence_impl<N - 1, N - 1, I...> {
};

template <std::size_t... I>
struct make_index_sequence_impl<0, I...> {
    using type = index_sequence<I...>;
};

template <std::size_t N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;

template <typename T, class F, class... Args>
struct DelayInvoke {
    F f;
    std::tuple<Args...> args;

    DelayInvoke(F &&f, Args &&...args)
        : f(std::forward<F>(f)),
          args(std::make_tuple(std::forward<Args>(args)...)) {}

    template <std::size_t... I>
    T invoke(index_sequence<I...>) {
        return (f)(std::get<I>(args)...);
    }

    T operator()() { return invoke(make_index_sequence<sizeof...(Args)>()); }
};

template <typename T, class F, class... Args>
T delay_invoke(F &&f, Args &&...args) {
    return DelayInvoke<T, F, Args...>(std::forward<F>(f),
                                      std::forward<Args>(args)...)();
}
#endif

#endif