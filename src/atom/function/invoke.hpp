/*
 * invoke.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: An implementation of invoke function. Support C++11 and C++17.

**************************************************/

#ifndef ATOM_FUNCTION_INVOKE_HPP
#define ATOM_FUNCTION_INVOKE_HPP

#include <exception>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <utility>

#if __cplusplus >= 201703L

#include <type_traits>
#include <variant>

template <typename F, typename... Args>
concept Invocable = std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>;

template <typename F, typename... Args>
    requires Invocable<F, Args...>
auto delay_invoke(F &&f, Args &&...args) {
    return [f = std::forward<F>(f),
            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(std::move(f), std::move(args));
    };
}

template <typename R, typename T, typename... Args>
auto delay_mem_invoke(R (T::*f)(Args...), T *obj) {
    return [f, obj](Args... args) {
        return (obj->*f)(std::forward<Args>(args)...);
    };
}

template <typename R, typename T, typename... Args>
auto delay_cmem_invoke(R (T::*f)(Args...) const, const T *obj) {
    return [f, obj](Args... args) {
        return (obj->*f)(std::forward<Args>(args)...);
    };
}

template <typename R, typename T, typename... Args>
auto delay_static_mem_invoke(R (*f)(Args...), T *obj) {
    return [f, obj](Args... args) {
        (void)obj;  // obj is not used in static member functions
        return f(std::forward<Args>(args)...);
    };
}

template <typename T, typename M>
auto delay_member_var_invoke(M T::*m, T *obj) {
    return [m, obj]() -> decltype(auto) { return (obj->*m); };
}

template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safe_call(Func &&func, Args &&...args) {
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

template <typename F, typename... Args>
    requires std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>
auto safe_try_catch(F &&func, Args &&...args) {
    using ReturnType =
        std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
    using ResultType = std::variant<ReturnType, std::exception_ptr>;

    try {
        if constexpr (std::is_same_v<ReturnType, void>) {
            std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
            return ResultType{};  // Empty variant for void functions
        } else {
            return ResultType{std::invoke(std::forward<F>(func),
                                          std::forward<Args>(args)...)};
        }
    } catch (...) {
        return ResultType{std::current_exception()};
    }
}

template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
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

template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safe_try_catch_with_custom_handler(
    Func &&func, std::function<void(std::exception_ptr)> handler,
    Args &&...args) {
    try {
        return std::invoke(std::forward<Func>(func),
                           std::forward<Args>(args)...);
    } catch (...) {
        handler(std::current_exception());
        using ReturnType =
            std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
        if constexpr (std::is_default_constructible_v<ReturnType>) {
            return ReturnType{};
        } else {
            throw;
        }
    }
}

#else  // C++11 and C++14 support

#include <type_traits>

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

template <typename Func, typename... Args>
auto safe_try_catch(Func &&func, Args &&...args) ->
    typename std::enable_if<
        !std::is_void<typename std::result_of<Func(Args...)>::type>::value,
        std::tuple<typename std::result_of<Func(Args...)>::type,
                   std::exception_ptr> >::type {
    using ReturnType = typename std::result_of<Func(Args...)>::type;
    try {
        return std::make_tuple(
            std::forward<Func>(func)(std::forward<Args>(args)...), nullptr);
    } catch (...) {
        return std::make_tuple(ReturnType(), std::current_exception());
    }
}

template <typename Func, typename... Args>
auto safe_try_catch(Func &&func, Args &&...args) ->
    typename std::enable_if<
        std::is_void<typename std::result_of<Func(Args...)>::type>::value,
        std::tuple<std::exception_ptr> >::type {
    try {
        std::forward<Func>(func)(std::forward<Args>(args)...);
        return std::make_tuple(nullptr);
    } catch (...) {
        return std::make_tuple(std::current_exception());
    }
}

template <typename Func, typename... Args>
auto safe_try_catch_or_default(
    Func &&func, typename std::result_of<Func(Args...)>::type default_value,
    Args &&...args) -> typename std::result_of<Func(Args...)>::type {
    try {
        return std::forward<Func>(func)(std::forward<Args>(args)...);
    } catch (...) {
        return default_value;
    }
}

template <typename Func, typename... Args>
auto safe_try_catch_with_custom_handler(
    Func &&func, std::function<void(std::exception_ptr)> handler,
    Args &&...args) -> typename std::result_of<Func(Args...)>::type {
    try {
        return std::forward<Func>(func)(std::forward<Args>(args)...);
    } catch (...) {
        handler(std::current_exception());
        using ReturnType = typename std::result_of<Func(Args...)>::type;
        if (std::is_default_constructible<ReturnType>::value) {
            return ReturnType{};
        } else {
            throw;
        }
    }
}

#endif  // __cplusplus

#endif  // ATOM_FUNCTION_INVOKE_HPP
