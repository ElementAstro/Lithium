/*!
 * \file proxy.hpp
 * \brief Proxy Function Implementation
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_PROXY_HPP
#define ATOM_META_PROXY_HPP

#include "config.h"

#include <any>
#include <chrono>
#include <functional>
#include <future>
#include <thread>
#include <typeinfo>
#include <utility>
#include <vector>
#include "atom/async/async.hpp"

#include "macro.hpp"

#if ENABLE_DEBUG
#include <iostream>
#endif

#include "atom/algorithm/hash.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"
#include "atom/function/proxy_params.hpp"

namespace atom::meta {

struct ATOM_ALIGNAS(128) FunctionInfo {
private:
    std::string returnType_;
    std::vector<std::string> argumentTypes_;
    std::string hash_;

public:
    FunctionInfo() = default;

    void logFunctionInfo() const {
#if ENABLE_DEBUG
        std::cout << "Function return type: " << returnType_ << "\n";
        for (size_t i = 0; i < argumentTypes_.size(); ++i) {
            std::cout << "Argument " << i + 1
                      << ": Type = " << argumentTypes_[i] << std::endl;
        }
#endif
    }

    [[nodiscard]] auto getReturnType() const -> const std::string & {
        return returnType_;
    }

    [[nodiscard]] auto getArgumentTypes() const
        -> const std::vector<std::string> & {
        return argumentTypes_;
    }

    [[nodiscard]] auto getHash() const -> const std::string & { return hash_; }

    void setReturnType(const std::string &returnType) {
        returnType_ = returnType;
    }

    void addArgumentType(const std::string &argumentType) {
        argumentTypes_.push_back(argumentType);
    }

    void setHash(const std::string &hash) { hash_ = hash; }
};

// TODO: FIX ME - any cast can not cover all cases
template <typename T>
auto anyCastRef(std::any &operand) -> T && {
    return *std::any_cast<std::decay_t<T> *>(operand);
}

template <typename T>
auto anyCastRef(const std::any &operand) -> T & {
#if ENABLE_DEBUG
    std::cout << "type: " << TypeInfo::fromType<T>().name() << "\n";
#endif
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
auto anyCastConstRef(const std::any &operand) -> const T & {
    // Max: 有的时候大道至简
    // return *std::any_cast<std::remove_reference_t<std::remove_const_t<T>> *>(
    //    &const_cast<std::any &>(operand));
    return std::any_cast<T>(operand);
}

template <typename T>
auto anyCastHelper(std::any &operand) -> decltype(auto) {
    if constexpr (std::is_reference_v<T> &&
                  std::is_const_v<std::remove_reference_t<T>>) {
        return anyCastConstRef<T>(operand);
    } else if constexpr (std::is_reference_v<T>) {
        return anyCastRef<T>(operand);
    } else {
        return anyCastVal<T>(operand);
    }
}

template <typename T>
auto anyCastHelper(const std::any &operand) -> decltype(auto) {
    if constexpr (std::is_reference_v<T> &&
                  std::is_const_v<std::remove_reference_t<T>>) {
        return anyCastConstRef<T>(operand);
    } else if constexpr (std::is_reference_v<T>) {
        return anyCastRef<T>(operand);
    } else {
        return anyCastVal<T>(operand);
    }
}

template <typename Func>
class BaseProxyFunction {
protected:
    std::decay_t<Func> func_;

    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t ARITY = Traits::arity;
    FunctionInfo info_;

public:
    explicit BaseProxyFunction(Func &&func) : func_(std::move(func)) {
        collectFunctionInfo();
        calcFuncInfoHash();
    }

    [[nodiscard]] auto getFunctionInfo() const -> FunctionInfo { return info_; }

protected:
    void collectFunctionInfo() {
        // Collect return type information
        info_.setReturnType(
            DemangleHelper::demangleType<typename Traits::return_type>());

        // Collect argument types information
        collectArgumentTypes(std::make_index_sequence<ARITY>{});
    }

    template <std::size_t... Is>
    void collectArgumentTypes(std::index_sequence<Is...> /*unused*/) {
        (info_.addArgumentType(DemangleHelper::demangleType<
                               typename Traits::template argument_t<Is>>()),
         ...);
    }

    void calcFuncInfoHash() {
        // 仅根据参数类型进行区分,返回值不支持,具体是因为在dispatch时不知道返回值的类型
        if (!info_.getArgumentTypes().empty()) {
            info_.setHash(std::to_string(
                atom::algorithm::computeHash(info_.getArgumentTypes())));
        }
    }

    void logArgumentTypes() const {
#if ENABLE_DEBUG
        std::cout << "Function Arity: " << arity << "\n";
        info_.logFunctionInfo();
#endif
    }

    template <std::size_t... Is>
    auto callFunction(const std::vector<std::any> &args,
                      std::index_sequence<Is...> /*unused*/) -> std::any {
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
                            std::index_sequence<Is...> /*unused*/) -> std::any {
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
class ProxyFunction : public BaseProxyFunction<Func> {
    using Base = BaseProxyFunction<Func>;
    using Traits = typename Base::Traits;
    static constexpr std::size_t arity = Base::arity;

public:
    explicit ProxyFunction(Func &&func) : Base(std::move(func)) {}

    auto operator()(const std::vector<std::any> &args) -> std::any {
        this->logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (args.size() != arity + 1) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return this->callMemberFunction(args,
                                            std::make_index_sequence<arity>());
        } else {
#if ENABLE_DEBUG
            std::cout << "Function Arity: " << arity << "\n";
            std::cout << "Arguments size: " << args.size() << "\n";
#endif
            if (args.size() != arity) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return this->callFunction(args, std::make_index_sequence<arity>());
        }
    }

    auto operator()(const FunctionParams &params) -> std::any {
        this->logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (params.size() != arity + 1) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return this->callMemberFunction(params.toVector());
        } else {
            if (params.size() != arity) {
                THROW_EXCEPTION("Incorrect number of arguments");
            }
            return this->callFunction(params.toVector());
        }
    }
};

template <typename Func>
class TimerProxyFunction : public BaseProxyFunction<Func> {
    using Base = BaseProxyFunction<Func>;
    using Traits = typename Base::Traits;
    static constexpr std::size_t arity = Base::arity;

public:
    explicit TimerProxyFunction(Func &&func) : Base(std::move(func)) {}

    auto operator()(const std::vector<std::any> &args,
                    std::chrono::milliseconds timeout) -> std::any {
        this->logArgumentTypes();
        if constexpr (Traits::is_member_function) {
            if (args.size() != arity + 1) {
                THROW_EXCEPTION(
                    "Incorrect number of arguments for member function");
            }
            return this->callMemberFunctionWithTimeout(
                args, timeout, std::make_index_sequence<arity>());
        } else {
            if (args.size() != arity) {
                THROW_EXCEPTION(
                    "Incorrect number of arguments for non-member function");
            }
            return this->callFunctionWithTimeout(
                args, timeout, std::make_index_sequence<arity>());
        }
    }

private:
    template <std::size_t... Is>
    auto callFunctionWithTimeout(
        const std::vector<std::any> &args, std::chrono::milliseconds timeout,
        std::index_sequence<Is...> /*unused*/) -> std::any {
        auto task = [this, &args]() -> std::any {
            if constexpr (std::is_void_v<typename Traits::return_type>) {
                std::invoke(
                    this->func_,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(std::invoke(
                    this->func_,
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is])...));
            }
        };

        return executeWithTimeout(task, timeout);
    }

    template <std::size_t... Is>
    auto callMemberFunctionWithTimeout(
        const std::vector<std::any> &args, std::chrono::milliseconds timeout,
        std::index_sequence<Is...> /*unused*/) -> std::any {
        auto task = [this, &args]() -> std::any {
            auto &obj =
                std::any_cast<
                    std::reference_wrapper<typename Traits::class_type>>(
                    args[0])
                    .get();

            if constexpr (std::is_void_v<typename Traits::return_type>) {
                (obj.*(this->func_))(
                    std::any_cast<typename Traits::template argument_t<Is>>(
                        args[Is + 1])...);
                return {};
            } else {
                return std::make_any<typename Traits::return_type>(
                    (obj.*(this->func_))(
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
        std::thread taskThread(std::move(packagedTask));
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