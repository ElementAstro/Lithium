/*!
 * \file decorate.hpp
 * \brief An implementation of decorate function. Just like Python's decorator.
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef ATOM_META_DECORATE_HPP
#define ATOM_META_DECORATE_HPP

#include <chrono>
#include <concepts>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "func_traits.hpp"

namespace atom::meta {

template <typename F, typename... Args>
concept Callable = requires(F&& func, Args&&... args) {
    {
        std::invoke(std::forward<F>(func), std::forward<Args>(args)...)
    } -> std::same_as<typename FunctionTraits<std::decay_t<F>>::return_type>;
};

template <typename Func>
class Switchable {
public:
    using Traits = FunctionTraits<Func>;
    using R = typename Traits::return_type;
    using TupleArgs = typename Traits::argument_types;

    explicit Switchable(Func func) : f_(std::move(func)) {}

    template <typename F>
        requires Callable<F, typename std::tuple_element_t<0, TupleArgs>,
                          typename std::tuple_element_t<1, TupleArgs>>
    void switchTo(F&& new_f) {
        f_ = std::forward<F>(new_f);
    }

    template <std::size_t... Is>
    auto callImpl(const TupleArgs& args,
                  std::index_sequence<Is...> /*unused*/) const -> R {
        return std::invoke(f_, std::get<Is>(args)...);
    }

    template <typename... ArgsT>
    auto operator()(ArgsT&&... args) const -> R {
        static_assert(
            std::is_same_v<std::tuple<std::decay_t<ArgsT>...>, TupleArgs>,
            "Argument types do not match the function signature");
        auto tupleArgs = std::forward_as_tuple(std::forward<ArgsT>(args)...);
        return callImpl(tupleArgs,
                        std::make_index_sequence<sizeof...(ArgsT)>{});
    }

private:
    std::function<Func> f_;
};

template <typename FuncType>
struct decorator;

template <typename R, typename... Args>
struct decorator<R(Args...)> {
    using FuncType = std::function<R(Args...)>;
    using CallbackType =
        std::conditional_t<std::is_void_v<R>, std::function<void()>,
                           std::function<void(R)>>;

private:
    FuncType func_;
    std::function<void()> before_;
    CallbackType callback_;
    std::function<void(std::chrono::microseconds)> after_;

public:
    explicit decorator(FuncType func) : func_(std::move(func)) {}

    template <typename Before, typename Callback = CallbackType,
              typename After = std::function<void(std::chrono::microseconds)>>
    auto withHooks(
        Before&& before, Callback&& callback = CallbackType(),
        After&& after = [](auto) {}) const -> decorator<R(Args...)> {
        decorator<R(Args...)> copy(func_);
        copy.before_ = std::forward<Before>(before);
        copy.callback_ = std::forward<Callback>(callback);
        copy.after_ = std::forward<After>(after);
        return copy;
    }

    template <std::size_t... Is>
    auto callImpl(std::tuple<Args...>& args,
                  std::index_sequence<Is...> /*unused*/) const {
        if (before_) {
            before_();
        }
        auto start = std::chrono::high_resolution_clock::now();

        if constexpr (std::is_void_v<R>) {
            std::invoke(func_, std::get<Is>(args)...);
            if (callback_) {
                callback_();
            }
        } else {
            auto result = std::invoke(func_, std::get<Is>(args)...);
            if (callback_) {
                callback_(result);
            }
            auto end = std::chrono::high_resolution_clock::now();
            if (after_) {
                after_(std::chrono::duration_cast<std::chrono::microseconds>(
                    end - start));
            }
            return result;
        }

        auto end = std::chrono::high_resolution_clock::now();
        if (after_) {
            after_(std::chrono::duration_cast<std::chrono::microseconds>(
                end - start));
        }
    }

    auto operator()(Args... args) const {
        auto tupleArgs = std::make_tuple(std::forward<Args>(args)...);
        return callImpl(tupleArgs, std::make_index_sequence<sizeof...(Args)>{});
    }
};

template <typename F>
auto makeDecorator(F&& func) {
    return decorator<std::remove_reference_t<F>>(std::forward<F>(func));
}

template <typename FuncType>
struct LoopDecorator : public decorator<FuncType> {
    using Base = decorator<FuncType>;
    using Base::Base;

    template <typename... TArgs>
    auto operator()(int loopCount, TArgs&&... args) const {
        using ReturnType = decltype(this->func(args...));
        std::optional<ReturnType> result;

#pragma unroll
        for (int i = 0; i < loopCount; ++i) {
            if constexpr (std::is_void_v<ReturnType>) {
                Base::operator()(std::forward<TArgs>(args)...);
            } else {
                result = Base::operator()(std::forward<TArgs>(args)...);
            }
        }

        if constexpr (!std::is_void_v<ReturnType>) {
            return *result;
        }
    }
};

template <typename FuncType>
auto makeLoopDecorator(FuncType&& func) {
    return LoopDecorator<std::remove_reference_t<FuncType>>(
        std::forward<FuncType>(func));
}

template <typename FuncType>
struct ConditionCheckDecorator : public decorator<FuncType> {
    using Base = decorator<FuncType>;
    using Base::Base;

    template <typename ConditionFunc, typename... TArgs>
    auto operator()(ConditionFunc&& condition, TArgs&&... args) const {
        using ReturnType = decltype(this->func(args...));

        if (condition()) {
            return Base::operator()(std::forward<TArgs>(args)...);
        }
        if constexpr (!std::is_void_v<ReturnType>) {
            return ReturnType{};
        }
    }
};

template <typename FuncType>
auto makeConditionCheckDecorator(FuncType&& func) {
    return ConditionCheckDecorator<std::remove_reference_t<FuncType>>(
        std::forward<FuncType>(func));
}

class DecoratorError : public std::exception {
    std::string message_;

public:
    explicit DecoratorError(std::string msg) : message_(std::move(msg)) {}
    [[nodiscard]] auto what() const noexcept -> const char* override {
        return message_.c_str();
    }
};

template <typename R, typename... Args>
class BaseDecorator {
public:
    using FuncType = std::function<R(Args...)>;
    virtual auto operator()(FuncType func,
                            Args... args) -> R = 0;  // Changed from Args&&...
    virtual ~BaseDecorator() = default;

    // Define copy constructor
    BaseDecorator(const BaseDecorator& other) = default;

    // Define copy assignment operator
    auto operator=(const BaseDecorator& other) -> BaseDecorator& = default;

    // Define move constructor
    BaseDecorator(BaseDecorator&& other) noexcept = default;

    // Define move assignment operator
    auto operator=(BaseDecorator&& other) noexcept -> BaseDecorator& = default;
};

template <typename R, typename... Args>
class DecorateStepper {
    using FuncType = std::function<R(Args...)>;
    using DecoratorPtr = std::unique_ptr<BaseDecorator<R, Args...>>;

    std::vector<DecoratorPtr> decorators_;
    FuncType baseFunction_;

public:
    explicit DecorateStepper(FuncType func) : baseFunction_(std::move(func)) {}

    template <typename Decorator, typename... DArgs>
    void addDecorator(DArgs&&... args) {
        decorators_.emplace_back(
            std::make_unique<Decorator>(std::forward<DArgs>(args)...));
    }

    void addDecorator(DecoratorPtr decorator) {
        decorators_.push_back(std::move(decorator));
    }

    auto execute(Args... args) -> R {
        try {
            FuncType currentFunction = baseFunction_;

#pragma unroll
            for (auto it = decorators_.rbegin(); it != decorators_.rend();
                 ++it) {
                auto& decorator = *it;
                currentFunction = [&,
                                   nextFunction = std::move(currentFunction)](
                                      Args... innerArgs) -> R {
                    return (*decorator)(nextFunction,
                                        std::forward<Args>(innerArgs)...);
                };
            }

            return currentFunction(std::forward<Args>(args)...);
        } catch (const DecoratorError& e) {
            // Handle decorator error
            throw;  // Rethrow for now or handle as needed
        }
    }
};

// Helper function: Create DecorateStepper
template <typename Func>
auto makeDecorateStepper(Func&& func) {
    using Traits = FunctionTraits<std::remove_reference_t<Func>>;
    using ReturnType = typename Traits::return_type;
    using ArgumentTuple = typename Traits::argument_types;

    return DecorateStepper<ReturnType, std::tuple_element_t<0, ArgumentTuple>,
                           std::tuple_element_t<1, ArgumentTuple>>(
        std::forward<Func>(func));
}

// 新增功能：缓存装饰器
template <typename R, typename... Args>
class CacheDecorator : public BaseDecorator<R, Args...> {
    using FuncType = std::function<R(Args...)>;
    mutable std::unordered_map<std::tuple<Args...>, R> cache_;
    mutable std::mutex cacheMutex_;

public:
    auto operator()(FuncType func, Args... args) -> R override {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto argsTuple = std::make_tuple(args...);
        if (cache_.find(argsTuple) != cache_.end()) {
            return cache_[argsTuple];
        }
        auto result = func(std::forward<Args>(args)...);
        cache_[argsTuple] = result;
        return result;
    }
};

// 新增功能：重试装饰器
template <typename R, typename... Args>
class RetryDecorator : public BaseDecorator<R, Args...> {
    using FuncType = std::function<R(Args...)>;
    int retryCount_;

public:
    explicit RetryDecorator(int retryCount) : retryCount_(retryCount) {}

    auto operator()(FuncType func, Args... args) -> R override {
        for (int i = 0; i < retryCount_; ++i) {
            try {
                return func(std::forward<Args>(args)...);
            } catch (...) {
                if (i == retryCount_ - 1) {
                    throw;
                }
            }
        }
        throw DecoratorError("Retry limit reached");
    }
};

// 新增功能：异步装饰器
template <typename R, typename... Args>
class AsyncDecorator : public BaseDecorator<R, Args...> {
    using FuncType = std::function<R(Args...)>;

public:
    auto operator()(FuncType func, Args... args) -> R override {
        auto future =
            std::async(std::launch::async, func, std::forward<Args>(args)...);
        return future.get();
    }
};

// Helper function: Create CacheDecorator
template <typename Func>
auto makeCacheDecorator(Func&& func) {
    using Traits = FunctionTraits<std::remove_reference_t<Func>>;
    using ReturnType = typename Traits::return_type;
    using ArgumentTuple = typename Traits::argument_types;

    return CacheDecorator<ReturnType, std::tuple_element_t<0, ArgumentTuple>,
                          std::tuple_element_t<1, ArgumentTuple>>(
        std::forward<Func>(func));
}

// Helper function: Create RetryDecorator
template <typename Func>
auto makeRetryDecorator(Func&& func, int retryCount) {
    using Traits = FunctionTraits<std::remove_reference_t<Func>>;
    using ReturnType = typename Traits::return_type;
    using ArgumentTuple = typename Traits::argument_types;

    return RetryDecorator<ReturnType, std::tuple_element_t<0, ArgumentTuple>,
                          std::tuple_element_t<1, ArgumentTuple>>(retryCount);
}

// Helper function: Create AsyncDecorator
template <typename Func>
auto makeAsyncDecorator(Func&& func) {
    using Traits = FunctionTraits<std::remove_reference_t<Func>>;
    using ReturnType = typename Traits::return_type;
    using ArgumentTuple = typename Traits::argument_types;

    return AsyncDecorator<ReturnType, std::tuple_element_t<0, ArgumentTuple>,
                          std::tuple_element_t<1, ArgumentTuple>>(
        std::forward<Func>(func));
}

}  // namespace atom::meta

#endif  // ATOM_META_DECORATE_HPP