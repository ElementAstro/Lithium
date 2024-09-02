/*!
 * \file invoke.hpp
 * \brief An implementation of invoke function. Supports C++11 and C++17.
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_INVOKE_HPP
#define ATOM_META_INVOKE_HPP

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "atom/error/exception.hpp"

template <typename F, typename... Args>
concept Invocable = std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>;

template <typename F, typename... Args>
    requires Invocable<F, Args...>
auto delayInvoke(F &&func, Args &&...args) {
    return [func = std::forward<F>(func),
            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(std::move(func), std::move(args));
    };
}

template <typename R, typename T, typename... Args>
auto delayMemInvoke(R (T::*func)(Args...), T *obj) {
    return [func, obj](Args... args) {
        return (obj->*func)(std::forward<Args>(args)...);
    };
}

template <typename R, typename T, typename... Args>
auto delayMemInvoke(R (T::*func)(Args...) const, const T *obj) {
    return [func, obj](Args... args) {
        return (obj->*func)(std::forward<Args>(args)...);
    };
}

template <typename R, typename T, typename... Args>
auto delayStaticMemInvoke(R (*func)(Args...), T *obj) {
    return [func, obj](Args... args) {
        (void)obj;  // obj is not used in static member functions
        return func(std::forward<Args>(args)...);
    };
}

template <typename T, typename M>
auto delayMemberVarInvoke(M T::*m, T *obj) {
    return [m, obj]() -> decltype(auto) { return (obj->*m); };
}

template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safeCall(Func &&func, Args &&...args) {
    try {
        return std::invoke(std::forward<Func>(func),
                           std::forward<Args>(args)...);
    } catch (...) {
        using ReturnType =
            std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
        if constexpr (std::is_default_constructible_v<ReturnType>) {
            return ReturnType{};
        } else {
            THROW_RUNTIME_ERROR("An exception occurred in safe_call");
        }
    }
}

template <typename F, typename... Args>
    requires std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>
auto safeTryCatch(F &&func, Args &&...args) {
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
auto safeTryCatchOrDefault(
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
auto safeTryCatchWithCustomHandler(
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

#endif  // ATOM_META_INVOKE_HPP
