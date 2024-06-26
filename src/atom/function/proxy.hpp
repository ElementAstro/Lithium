/*!
 * \file proxy.hpp
 * \brief Proxy Function Implementation
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_PROXY_HPP
#define ATOM_META_PROXY_HPP

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

#include "atom/algorithm/hash.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"
#include "atom/function/proxy_params.hpp"

namespace atom::meta {
struct FunctionInfo {
    std::string returnType;
    std::vector<std::string> argumentTypes;
    std::string hash;

    FunctionInfo() = default;

    void logFunctionInfo() const {
#if ENABLE_DEBUG
        std::cout << "Function return type: " << returnType << "\n";
        for (size_t i = 0; i < argumentTypes.size(); ++i) {
            std::cout << "Argument " << i + 1 << ": Type = " << argumentTypes[i]
                      << std::endl;
        }
#endif
    }
};

template <typename T>
T &&any_cast_ref(std::any &operand) {
    return *std::any_cast<std::decay_t<T> *>(operand);
}

template <typename T>
T &&any_cast_ref(const std::any &operand) {
    return *std::any_cast<std::decay_t<T> *>(operand);
}

template <typename T>
T any_cast_val(std::any &operand) {
    return std::any_cast<T>(operand);
}

template <typename T>
T any_cast_val(const std::any &operand) {
    return std::any_cast<T>(operand);
}

template <typename T>
decltype(auto) any_cast_helper(std::any &operand) {
    if constexpr (std::is_reference_v<T>) {
        return any_cast_ref<T>(operand);
    } else {
        return any_cast_val<T>(operand);
    }
}

template <typename T>
decltype(auto) any_cast_helper(const std::any &operand) {
    if constexpr (std::is_reference_v<T>) {
        return any_cast_ref<T>(operand);
    } else {
        return any_cast_val<T>(operand);
    }
}

template <typename Func>
struct ProxyFunction {
    std::decay_t<Func> func;

    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

    explicit ProxyFunction(Func &&func) : func(std::move(func)) {
        collectFunctionInfo();
        calcFuncInfoHash();
    }

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

    std::any operator()(const FunctionParams &params) {
        logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (params.size() != N + 1) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return callMemberFunction(params.to_vector());
        } else {
            if (params.size() != N) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return callFunction(params.to_vector());
        }
    }

    FunctionInfo getFunctionInfo() const { return info; }

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
             DemangleHelper::DemangleType<
                 typename Traits::template argument_t<Is>>()),
         ...);
    }

    void calcFuncInfoHash() {
        // 仅根据参数类型进行区分,返回值不支持,具体是因为在dispatch时不知道返回值的类型
        if (!info.argumentTypes.empty()) {
            info.hash = atom::algorithm::computeHash(info.argumentTypes);
        }
    }

    FunctionInfo info;

    void logArgumentTypes() const {
#if ENABLE_DEBUG
        std::cout << "Function Arity: " << N << "\n";
        info.logFunctionInfo();
#endif
    }

    template <std::size_t... Is>
    std::any callFunction(const std::vector<std::any> &args,
                          std::index_sequence<Is...>) {
        if constexpr (std::is_void_v<typename Traits::return_type>) {
            std::invoke(
                func, any_cast_helper<typename Traits::template argument_t<Is>>(
                          args[Is])...);
            return {};
        } else {
            return std::make_any<typename Traits::return_type>(std::invoke(
                func, any_cast_helper<typename Traits::template argument_t<Is>>(
                          args[Is])...));
        }
    }

    std::any callFunction(const FunctionParams &params) {
        if constexpr (std::is_void_v<typename Traits::return_type>) {
            std::invoke(func, params.to_vector());
            return {};
        } else {
            return std::make_any<typename Traits::return_type>(
                std::invoke(func, params.to_vector()));
        }
    }

    template <std::size_t... Is>
    std::any callMemberFunction(const std::vector<std::any> &args,
                                std::index_sequence<Is...>) {
        auto invokeFunc = [this](auto &obj, auto &&...args) {
            if constexpr (Traits::is_const_member_function) {
                if constexpr (std::is_void_v<typename Traits::return_type>) {
                    (obj.*func)(std::forward<decltype(args)>(args)...);
                    return std::any{};
                } else {
                    return std::make_any<typename Traits::return_type>(
                        (obj.*func)(std::forward<decltype(args)>(args)...));
                }
            } else {
                if constexpr (std::is_void_v<typename Traits::return_type>) {
                    (obj.*func)(std::forward<decltype(args)>(args)...);
                    return std::any{};
                } else {
                    return std::make_any<typename Traits::return_type>(
                        (obj.*func)(std::forward<decltype(args)>(args)...));
                }
            }
        };

        if (args[0].type() ==
            typeid(std::reference_wrapper<typename Traits::class_type>)) {
            auto &obj =
                std::any_cast<
                    std::reference_wrapper<typename Traits::class_type>>(
                    args[0])
                    .get();
            return invokeFunc(
                obj, any_cast_helper<typename Traits::template argument_t<Is>>(
                         args[Is + 1])...);
        } else {
            auto &obj = const_cast<typename Traits::class_type &>(
                std::any_cast<const typename Traits::class_type &>(args[0]));
            return invokeFunc(
                obj, any_cast_helper<typename Traits::template argument_t<Is>>(
                         args[Is + 1])...);
        }
    }
};

template <typename Func>
struct TimerProxyFunction {
    std::decay_t<Func> func;
    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

    explicit TimerProxyFunction(Func &&func) : func(std::move(func)) {}

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
             DemangleHelper::DemangleType<
                 typename Traits::template argument_t<Is>>()),
         ...);
    }

    void logArgumentTypes() const {
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
                std::invoke(
                    func,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    func,
                    std::any_cast<typename Traits::template argument_t<Is>>(
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
            auto bound_func = std::bind_front(func, obj, std::placeholders::_1);

            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(
                    bound_func,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is + 1])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    bound_func,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is + 1])...));
            }
        };

        return executeWithTimeout(task, timeout);
    }

    template <typename TaskFunc>
    std::any executeWithTimeout(TaskFunc task,
                                std::chrono::milliseconds timeout) const {
        std::packaged_task<std::any()> packaged_task(std::move(task));
        std::future<std::any> future = packaged_task.get_future();
#if __cplusplus >= 201703L
        std::jthread task_thread(std::move(packaged_task));
#else
        std::thread task_thread(std::move(packaged_task));
#endif
        if (future.wait_for(timeout) == std::future_status::timeout) {
            task_thread.detach();  // Detach the thread on timeout
            THROW_EXCEPTION("Function execution timed out");
        }

        task_thread.join();  // Ensure the thread has finished
        return future.get();
    }
};
}  // namespace atom::meta

#endif
