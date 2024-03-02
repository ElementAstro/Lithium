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

#include <functional>
#include <tuple>
#include <utility>

#if __cplusplus >= 201703L

#include <type_traits>

template <typename F, typename... Args>
using is_invocable_with_args = std::is_invocable<F, Args...>;

template <class F, class... Args>
auto delay_invoke(F &&f, Args &&...args) {
    static_assert(is_invocable_with_args<F, Args...>::value,
                  "F must be callable with Args...");

    return [f = std::forward<F>(f),
            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(f, args);
    };
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