/*
 * proxy.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Proxy Function Implementation

**************************************************/

#ifndef ATOM_FUNCTION_PROXY_HPP
#define ATOM_FUNCTION_PROXY_HPP

#include <any>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <stdexcept>
#include <thread>
#include <typeinfo>
#include <utility>
#include <vector>
#if ENABLE_DEBUG
#include <iostream>
#endif

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"

struct FunctionInfo {
    std::string returnType;
    std::vector<std::string> argumentTypes;
    std::vector<std::string> argumentNames;

    FunctionInfo() = default;

    void logFunctionInfo() {
#if ENABLE_DEBUG
        std::cout << "Function return type: " << returnType << "\n";
        for (size_t i = 0; i < argumentTypes.size(); ++i) {
            std::cout << "Argument " << i + 1
                      << ": Type = " << argumentTypes[i];
            if (i < argumentNames.size()) {
                std::cout << ", Name = " << argumentNames[i];
            }
            std::cout << "\n";
        }
#endif
    }
};

template <typename Func>
struct ProxyFunction {
    std::decay_t<Func> func;

    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

    ProxyFunction(Func func) : func(func) { collectFunctionInfo(); }

    std::any operator()(const std::vector<std::any> &args) {
        logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (args.size() != N + 1) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return callMemberFunction(args, std::make_index_sequence<N>());
        } else {
            if (args.size() != N) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return callFunction(args, std::make_index_sequence<N>());
        }
    }

private:
    void collectFunctionInfo() {
        // Collect return type information
        info.returnType =
            DemangleHelper::DemangleType<typename Traits::return_type>();

        // Collect argument types information
        collectArgumentTypes(std::make_index_sequence<N>{});
    }

    template <std::size_t... Is>
    void collectArgumentTypes(std::index_sequence<Is...>) {
        (info.argumentTypes.push_back(
             DemangleHelper::DemangleType<typename Traits::argument_t<Is>>()),
         ...);
    }

    FunctionInfo info;

    void logArgumentTypes() {
#if ENABLE_DEBUG
        std::cout << "Function Arity: " << N << "\n";
        info.logFunctionInfo();
#endif
    }

    template <std::size_t... Is>
    std::any callFunction(const std::vector<std::any> &args,
                          std::index_sequence<Is...>) {
        if constexpr (std::is_void_v<typename Traits::return_type>) {
            std::invoke(func, std::any_cast<typename Traits::argument_t<Is>>(
                                  args[Is])...);
            return {};
        } else {
            return std::make_any<typename Traits::return_type>(std::invoke(
                func,
                std::any_cast<typename Traits::argument_t<Is>>(args[Is])...));
        }
    }

    template <std::size_t... Is>
    std::any callMemberFunction(const std::vector<std::any> &args,
                                std::index_sequence<Is...>) {
        if (args[0].type() ==
            typeid(std::reference_wrapper<typename Traits::class_type>)) {
            auto &obj =
                std::any_cast<
                    std::reference_wrapper<typename Traits::class_type>>(
                    args[0])
                    .get();
            auto bound_func = bind_first(func, obj);
            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(bound_func,
                            std::any_cast<typename Traits::argument_t<Is>>(
                                args[Is + 1])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    bound_func, std::any_cast<typename Traits::argument_t<Is>>(
                                    args[Is + 1])...));
            }
        } else {
            auto &obj = const_cast<typename Traits::class_type &>(
                std::any_cast<const typename Traits::class_type &>(args[0]));
            auto bound_func = bind_first(func, obj);
            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(bound_func,
                            std::any_cast<typename Traits::argument_t<Is>>(
                                args[Is + 1])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    bound_func, std::any_cast<typename Traits::argument_t<Is>>(
                                    args[Is + 1])...));
            }
        }
    }
};

template <typename Func>
struct TimerProxyFunction {
    std::decay_t<Func> func;
    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

    explicit TimerProxyFunction(Func &&f) : func(std::forward<Func>(f)) {}

    std::any operator()(const std::vector<std::any> &args,
                        std::chrono::milliseconds timeout) {
        logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (args.size() != N + 1) {
                THROW_EXCEPTION(
                    "Incorrect number of arguments for member function");
            }
            return callMemberFunctionWithTimeout(args, timeout,
                                                 std::make_index_sequence<N>());
        } else {
            if (args.size() != N) {
                THROW_EXCEPTION(
                    "Incorrect number of arguments for non-member function");
            }
            return callFunctionWithTimeout(args, timeout,
                                           std::make_index_sequence<N>());
        }
    }

private:
    FunctionInfo info;

    void collectFunctionInfo() {
        // Collect return type information
        info.returnType =
            DemangleHelper::DemangleType<typename Traits::return_type>();

        // Collect argument types information
        collectArgumentTypes(std::make_index_sequence<N>{});
    }

    template <std::size_t... Is>
    void collectArgumentTypes(std::index_sequence<Is...>) {
        (info.argumentTypes.push_back(
             DemangleHelper::DemangleType<typename Traits::argument_t<Is>>()),
         ...);
    }

    void logArgumentTypes() {
#if ENABLE_DEBUG
        std::cout << "Function Arity: " << N << "\n";
        info.logFunctionInfo();
#endif
    }

    template <std::size_t... Is>
    std::any callFunctionWithTimeout(const std::vector<std::any> &args,
                                     std::chrono::milliseconds timeout,
                                     std::index_sequence<Is...>) {
        auto task = [this, &args]() -> std::any {
            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(func,
                            std::any_cast<typename Traits::argument_t<Is>>(
                                args[Is])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    func, std::any_cast<typename Traits::argument_t<Is>>(
                              args[Is])...));
            }
        };

        return executeWithTimeout(task, timeout);
    }

    template <std::size_t... Is>
    std::any callMemberFunctionWithTimeout(const std::vector<std::any> &args,
                                           std::chrono::milliseconds timeout,
                                           std::index_sequence<Is...>) {
        auto task = [this, &args]() -> std::any {
            auto &obj =
                std::any_cast<
                    std::reference_wrapper<typename Traits::class_type>>(
                    args[0])
                    .get();
            auto bound_func = std::bind(func, obj, std::placeholders::_1);

            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(bound_func,
                            std::any_cast<typename Traits::argument_t<Is>>(
                                args[Is + 1])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    bound_func, std::any_cast<typename Traits::argument_t<Is>>(
                                    args[Is + 1])...));
            }
        };

        return executeWithTimeout(task, timeout);
    }

    template <typename TaskFunc>
    std::any executeWithTimeout(TaskFunc task,
                                std::chrono::milliseconds timeout) {
        std::packaged_task<std::any()> packaged_task(std::move(task));
        std::future<std::any> future = packaged_task.get_future();
        std::thread task_thread(std::move(packaged_task));

        if (future.wait_for(timeout) == std::future_status::timeout) {
            task_thread.detach();  // Detach the thread on timeout
            THROW_EXCEPTION("Function execution timed out");
        }

        task_thread.join();  // Ensure the thread has finished
        return future.get();
    }
};

#endif
