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
#include <functional>
#include <future>
#include <thread>
#include <typeinfo>
#include <utility>
#include <vector>
#include "atom/async/async.hpp"
#include "atom/macro.hpp"
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
} ATOM_ALIGNAS(128);

template <typename T>
auto anyCastRef(std::any &operand) -> T && {
    return *std::any_cast<std::decay_t<T> *>(operand);
}

template <typename T>
auto anyCastRef(const std::any &operand) -> T && {
    return *std::any_cast<std::decay_t<T> *>(operand);
}

template <typename T>
auto anyCastVal(std::any &operand) -> T {
    return std::any_cast<T>(operand);
}

template <typename T>
auto anyCastVal(const std::any &operand) -> T {
    return std::any_cast<T>(operand);
}

template <typename T>
auto anyCastHelper(std::any &operand) -> decltype(auto) {
    if constexpr (std::is_reference_v<T>) {
        return anyCastRef<T>(operand);
    } else {
        return anyCastVal<T>(operand);
    }
}

template <typename T>
auto anyCastHelper(const std::any &operand) -> decltype(auto) {
    if constexpr (std::is_reference_v<T>) {
        return anyCastRef<T>(operand);
    } else {
        return anyCastVal<T>(operand);
    }
}

template <typename Func>
class ProxyFunction {
    std::decay_t<Func> func_;

    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

public:
    explicit ProxyFunction(Func &&func) : func_(std::move(func)) {
        collectFunctionInfo();
        calcFuncInfoHash();
    }

    auto operator()(const std::vector<std::any> &args) -> std::any {
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

    auto operator()(const FunctionParams &params) -> std::any {
        logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (params.size() != N + 1) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return callMemberFunction(params.toVector());
        } else {
            if (params.size() != N) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return callFunction(params.toVector());
        }
    }

    [[nodiscard]] auto getFunctionInfo() const -> FunctionInfo { return info_; }

private:
    void collectFunctionInfo() {
        // Collect return type information
        info_.returnType =
            DemangleHelper::demangleType<typename Traits::return_type>();

        // Collect argument types information
        collectArgumentTypes(std::make_index_sequence<N>{});
    }

    template <std::size_t... Is>
    void collectArgumentTypes(std::index_sequence<Is...>) {
        (info_.argumentTypes.push_back(
             DemangleHelper::demangleType<
                 typename Traits::template argument_t<Is>>()),
         ...);
    }

    void calcFuncInfoHash() {
        // 仅根据参数类型进行区分,返回值不支持,具体是因为在dispatch时不知道返回值的类型
        if (!info_.argumentTypes.empty()) {
            info_.hash = std::to_string(
                atom::algorithm::computeHash(info_.argumentTypes));
        }
    }

    FunctionInfo info_;

    void logArgumentTypes() const {
#if ENABLE_DEBUG
        std::cout << "Function Arity: " << N << "\n";
        info.logFunctionInfo();
#endif
    }

    template <std::size_t... Is>
    auto callFunction(const std::vector<std::any> &args,
                      std::index_sequence<Is...>) -> std::any {
        if constexpr (std::is_void_v<typename Traits::return_type>) {
            std::invoke(func_,
                        anyCastHelper<typename Traits::template argument_t<Is>>(
                            args[Is])...);
            return {};
        } else {
            return std::make_any<typename Traits::return_type>(std::invoke(
                func_, anyCastHelper<typename Traits::template argument_t<Is>>(
                           args[Is])...));
        }
    }

    auto callFunction(const FunctionParams &params) -> std::any {
        if constexpr (std::is_void_v<typename Traits::return_type>) {
            std::invoke(func_, params.toVector());
            return {};
        } else {
            return std::make_any<typename Traits::return_type>(
                std::invoke(func_, params.toVector()));
        }
    }

    template <std::size_t... Is>
    auto callMemberFunction(const std::vector<std::any> &args,
                            std::index_sequence<Is...>) -> std::any {
        auto invokeFunc = [this](auto &obj, auto &&...args) {
            if constexpr (Traits::is_const_member_function) {
                if constexpr (std::is_void_v<typename Traits::return_type>) {
                    (obj.*func_)(std::forward<decltype(args)>(args)...);
                    return std::any{};
                } else {
                    return std::make_any<typename Traits::return_type>(
                        (obj.*func_)(std::forward<decltype(args)>(args)...));
                }
            } else {
                if constexpr (std::is_void_v<typename Traits::return_type>) {
                    (obj.*func_)(std::forward<decltype(args)>(args)...);
                    return std::any{};
                } else {
                    return std::make_any<typename Traits::return_type>(
                        (obj.*func_)(std::forward<decltype(args)>(args)...));
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
                obj, anyCastHelper<typename Traits::template argument_t<Is>>(
                         args[Is + 1])...);
        }
        auto &obj = const_cast<typename Traits::class_type &>(
            std::any_cast<const typename Traits::class_type &>(args[0]));
        return invokeFunc(
            obj, anyCastHelper<typename Traits::template argument_t<Is>>(
                     args[Is + 1])...);
    }
};

template <typename Func>
class TimerProxyFunction {
    std::decay_t<Func> func_;
    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

public:
    explicit TimerProxyFunction(Func &&func) : func_(std::move(func)) {}

    auto operator()(const std::vector<std::any> &args,
                    std::chrono::milliseconds timeout) -> std::any {
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
    FunctionInfo info_;

    void collectFunctionInfo() {
        // Collect return type information
        info_.returnType =
            DemangleHelper::demangleType<typename Traits::return_type>();

        // Collect argument types information
        collectArgumentTypes(std::make_index_sequence<N>{});
    }

    template <std::size_t... Is>
    void collectArgumentTypes(std::index_sequence<Is...>) {
        (info_.argumentTypes.push_back(
             DemangleHelper::demangleType<
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
    auto callFunctionWithTimeout(const std::vector<std::any> &args,
                                 std::chrono::milliseconds timeout,
                                 std::index_sequence<Is...>) -> std::any {
        auto task = [this, &args]() -> std::any {
            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(
                    func_,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    func_,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is])...));
            }
        };

        return executeWithTimeout(task, timeout);
    }

    template <std::size_t... Is>
    auto callMemberFunctionWithTimeout(const std::vector<std::any> &args,
                                       std::chrono::milliseconds timeout,
                                       std::index_sequence<Is...>) -> std::any {
        auto task = [this, &args]() -> std::any {
            auto &obj =
                std::any_cast<
                    std::reference_wrapper<typename Traits::class_type>>(
                    args[0])
                    .get();
            auto boundFunc = std::bind_front(func_, obj, std::placeholders::_1);

            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(
                    boundFunc,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is + 1])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    boundFunc,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is + 1])...));
            }
        };

        return executeWithTimeout(task, timeout);
    }

    template <typename TaskFunc>
    auto executeWithTimeout(
        TaskFunc task, std::chrono::milliseconds timeout) const -> std::any {
        std::packaged_task<std::any()> packagedTask(std::move(task));
        std::future<std::any> future = packagedTask.get_future();
#if __cplusplus >= 201703L
        std::jthread taskThread(std::move(packagedTask));
#else
        std::thread taskThread(std::move(packaged_task));
#endif
        if (future.wait_for(timeout) == std::future_status::timeout) {
            taskThread.detach();  // Detach the thread on timeout
            THROW_TIMEOUT_EXCEPTION("Function execution timed out");
        }

        taskThread.join();  // Ensure the thread has finished
        return future.get();
    }
};
}  // namespace atom::meta

#endif
