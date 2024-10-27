/*!
 * \file invoke.hpp
 * \brief An implementation of invoke function. Supports C++11 and C++17.
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29, Updated 2024-10-14
 */

#ifndef ATOM_META_INVOKE_HPP
#define ATOM_META_INVOKE_HPP

#include <chrono>
#include <functional>
#include <future>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

#include "atom/error/exception.hpp"

/*!
 * \brief Concept to check if a function is invocable with given arguments.
 * \tparam F The function type.
 * \tparam Args The argument types.
 */
template <typename F, typename... Args>
concept Invocable = std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>;

/*!
 * \brief Delays the invocation of a function with given arguments.
 * \tparam F The function type.
 * \tparam Args The argument types.
 * \param func The function to be invoked.
 * \param args The arguments to be passed to the function.
 * \return A lambda that, when called, invokes the function with the given
 * arguments.
 */
template <typename F, typename... Args>
    requires Invocable<F, Args...>
auto delayInvoke(F &&func, Args &&...args) {
    return [func = std::forward<F>(func),
            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(std::move(func), std::move(args));
    };
}

/*!
 * \brief Delays the invocation of a member function with given arguments.
 * \tparam R The return type of the member function.
 * \tparam T The class type of the member function.
 * \tparam Args The argument types.
 * \param func The member function to be invoked.
 * \param obj The object on which the member function will be invoked.
 * \return A lambda that, when called, invokes the member function with the
 * given arguments.
 */
template <typename R, typename T, typename... Args>
auto delayMemInvoke(R (T::*func)(Args...), T *obj) {
    return [func, obj](Args... args) {
        return (obj->*func)(std::forward<Args>(args)...);
    };
}

/*!
 * \brief Delays the invocation of a const member function with given arguments.
 * \tparam R The return type of the member function.
 * \tparam T The class type of the member function.
 * \tparam Args The argument types.
 * \param func The const member function to be invoked.
 * \param obj The object on which the member function will be invoked.
 * \return A lambda that, when called, invokes the const member function with
 * the given arguments.
 */
template <typename R, typename T, typename... Args>
auto delayMemInvoke(R (T::*func)(Args...) const, const T *obj) {
    return [func, obj](Args... args) {
        return (obj->*func)(std::forward<Args>(args)...);
    };
}

/*!
 * \brief Delays the invocation of a static member function with given
 * arguments. \tparam R The return type of the static member function. \tparam T
 * The class type of the static member function. \tparam Args The argument
 * types. \param func The static member function to be invoked. \param obj The
 * object (not used in static member functions). \return A lambda that, when
 * called, invokes the static member function with the given arguments.
 */
template <typename R, typename T, typename... Args>
auto delayStaticMemInvoke(R (*func)(Args...), T *obj) {
    return [func, obj](Args... args) {
        (void)obj;  // obj is not used in static member functions
        return func(std::forward<Args>(args)...);
    };
}

/*!
 * \brief Delays the invocation of a member variable.
 * \tparam T The class type of the member variable.
 * \tparam M The type of the member variable.
 * \param m The member variable to be accessed.
 * \param obj The object on which the member variable will be accessed.
 * \return A lambda that, when called, returns the member variable.
 */
template <typename T, typename M>
auto delayMemberVarInvoke(M T::*memberVar, T *obj) {
    return [memberVar, obj]() -> decltype(auto) { return (obj->*memberVar); };
}

/*!
 * \brief Safely calls a function with given arguments, catching any exceptions.
 * \tparam Func The function type.
 * \tparam Args The argument types.
 * \param func The function to be called.
 * \param args The arguments to be passed to the function.
 * \return The result of the function call, or a default-constructed value if an
 * exception occurs.
 */
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

/*!
 * \brief Safely tries to call a function with given arguments, catching any
 * exceptions. \tparam F The function type. \tparam Args The argument types.
 * \param func The function to be called.
 * \param args The arguments to be passed to the function.
 * \return A variant containing either the result of the function call or an
 * exception pointer.
 */
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

/*!
 * \brief Safely tries to call a function with given arguments, returning a
 * default value if an exception occurs. \tparam Func The function type. \tparam
 * Args The argument types. \param func The function to be called. \param
 * default_value The default value to return if an exception occurs. \param args
 * The arguments to be passed to the function. \return The result of the
 * function call, or the default value if an exception occurs.
 */
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

/*!
 * \brief Safely tries to call a function with given arguments, using a custom
 * handler if an exception occurs. \tparam Func The function type. \tparam Args
 * The argument types. \param func The function to be called. \param handler The
 * custom handler to be called if an exception occurs. \param args The arguments
 * to be passed to the function. \return The result of the function call, or a
 * default-constructed value if an exception occurs and the handler does not
 * rethrow.
 */
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safeTryCatchWithCustomHandler(
    Func &&func, const std::function<void(std::exception_ptr)> &handler,
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

/*!
 * \brief Asynchronously calls a function with given arguments.
 * \tparam Func The function type.
 * \tparam Args The argument types.
 * \param func The function to be called.
 * \param args The arguments to be passed to the function.
 * \return A future containing the result of the function call.
 */
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto asyncCall(Func &&func, Args &&...args) {
    return std::async(std::launch::async, std::forward<Func>(func),
                      std::forward<Args>(args)...);
}

/*!
 * \brief Calls a function with given arguments, retrying if an exception
 * occurs. \tparam Func The function type. \tparam Args The argument types.
 * \param func The function to be called. \param retries The number of retries.
 * \param args The arguments to be passed to the function. \return The result of
 * the function call.
 */
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto retryCall(Func &&func, int retries, Args &&...args) {
    while (retries-- > 0) {
        try {
            return std::invoke(std::forward<Func>(func),
                               std::forward<Args>(args)...);
        } catch (...) {
            if (retries == 0) {
                throw;
            }
        }
    }
}

/*!
 * \brief Calls a function with given arguments, timing out if the function
 * takes too long. \tparam Func The function type. \tparam Args The argument
 * types. \param func The function to be called. \param timeout The timeout
 * duration. \param args The arguments to be passed to the function. \return The
 * result of the function call, or a default-constructed value if the function
 * times out.
 */
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto timeoutCall(Func &&func, std::chrono::milliseconds timeout,
                 Args &&...args) {
    auto future = std::async(std::launch::async, std::forward<Func>(func),
                             std::forward<Args>(args)...);
    if (future.wait_for(timeout) == std::future_status::timeout) {
        THROW_RUNTIME_ERROR("Function call timed out");
    }
    return future.get();
}

/*!
 * \brief Caches the result of a function call with given arguments.
 * \tparam Func The function type.
 * \tparam Args The argument types.
 * \param func The function to be called.
 * \param args The arguments to be passed to the function.
 * \return The cached result of the function call.
 */
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto cacheCall(Func &&func, Args &&...args) {
    using ReturnType =
        std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
    static std::unordered_map<std::tuple<std::decay_t<Args>...>, ReturnType>
        cache;
    static std::shared_mutex mutex;

    auto key = std::make_tuple(std::forward<Args>(args)...);
    {
        std::shared_lock lock(mutex);
        if (auto it = cache.find(key); it != cache.end()) {
            return it->second;
        }
    }

    auto result =
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

    {
        std::unique_lock lock(mutex);
        cache[key] = result;
    }

    return result;
}

/*!
 * \brief Processes a batch of function calls with given arguments.
 * \tparam Func The function type.
 * \tparam Args The argument types.
 * \param func The function to be called.
 * \param argsList The list of argument tuples to be passed to the function.
 * \return A vector containing the results of the function calls.
 */
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto batchCall(Func &&func, const std::vector<std::tuple<Args...>> &argsList) {
    std::vector<std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>>
        results;
    results.reserve(argsList.size());

    for (const auto &args : argsList) {
        results.push_back(std::apply(std::forward<Func>(func), args));
    }

    return results;
}

#endif  // ATOM_META_INVOKE_HPP