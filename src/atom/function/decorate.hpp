/*!
 * \file decorate.hpp
 * \brief An implementation of decorate function. Just like Python's decorator.
 * \author Max Qian <lightapt.com>
 * \date 2023-03-29
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_DECORATE_HPP
#define ATOM_META_DECORATE_HPP

#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace atom::meta {
template <typename R, typename... Args>
class Switchable {
public:
    Switchable(std::function<R(Args...)> f) : f(f) {}

    template <typename F>
    void switch_to(F new_f) {
        f = new_f;
    }

    auto operator()(Args... args) const -> R { return f(args...); }

private:
    std::function<R(Args...)> f;
};

template <typename FuncType>
struct decorator;

template <typename R, typename... Args>
struct decorator<std::function<R(Args...)>> {
    using FuncType = std::function<R(Args...)>;
    using CallbackType =
        typename std::conditional<std::is_void<R>::value, std::function<void()>,
                                  std::function<void(R)>>::type;

    FuncType func;
    std::function<void()> before = nullptr;
    CallbackType callback = nullptr;
    std::function<void(long long)> after = nullptr;

    decorator(FuncType f) : func(f) {}

    template <typename Before, typename Callback = CallbackType,
              typename After = std::function<void(long long)>>
    decorator<FuncType> with_hooks(
        Before &&b, Callback &&c = CallbackType(),
        After &&a = [](long long) {}) const {
        decorator<FuncType> copy(std::move(func));
        copy.before = std::forward<Before>(b);
        copy.callback = std::forward<Callback>(c);
        copy.after = std::forward<After>(a);
        return copy;
    }

    template <typename... TArgs>
    auto operator()(TArgs &&...args) const {
        if (before)
            before();
        auto start = std::chrono::high_resolution_clock::now();
        if constexpr (std::is_void_v<R>) {
            std::invoke(func, std::forward<TArgs>(args)...);
            if (callback)
                callback();
        } else {
            auto result = std::invoke(func, std::forward<TArgs>(args)...);
            if (callback)
                callback(result);
            auto end = std::chrono::high_resolution_clock::now();
            if (after)
                after(std::chrono::duration_cast<std::chrono::microseconds>(
                          end - start)
                          .count());
            return result;
        }
        auto end = std::chrono::high_resolution_clock::now();
        if (after)
            after(std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                        start)
                      .count());
    }

    template <typename T, typename... TArgs>
    auto operator()(T &obj, TArgs &&...args) const {
        if (before)
            before();
        auto start = std::chrono::high_resolution_clock::now();
        if constexpr (std::is_void_v<R>) {
            std::invoke(func, obj, std::forward<TArgs>(args)...);
            if (callback)
                callback();
        } else {
            auto result = std::invoke(func, obj, std::forward<TArgs>(args)...);
            if (callback)
                callback(result);
            auto end = std::chrono::high_resolution_clock::now();
            if (after)
                after(std::chrono::duration_cast<std::chrono::microseconds>(
                          end - start)
                          .count());
            return result;
        }
        auto end = std::chrono::high_resolution_clock::now();
        if (after)
            after(std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                        start)
                      .count());
    }
};

template <typename F>
decorator<F> make_decorator(F f) {
    return decorator<F>(f);
}

template <typename FuncType>
struct LoopDecorator : public decorator<FuncType> {
    using Base = decorator<FuncType>;
    using Base::Base;

    template <typename... TArgs>
    auto operator()(int loopCount,
                    TArgs &&...args) const -> decltype(this->func(args...)) {
        decltype(this->func(args...)) result;
        for (int i = 0; i < loopCount; ++i) {
            result = Base::operator()(std::forward<TArgs>(args)...);
        }
        return result;
    }

    template <typename T, typename... TArgs>
    auto operator()(T &obj, int loopCount, TArgs &&...args) const {
        for (int i = 0; i < loopCount; ++i) {
            std::invoke(this->func, obj, std::forward<TArgs>(args)...);
        }
    }
};

template <typename FuncType>
LoopDecorator<FuncType> make_loop_decorator(FuncType f) {
    return LoopDecorator<FuncType>(f);
}

template <typename FuncType>
struct ConditionCheckDecorator : public decorator<FuncType> {
    using Base = decorator<FuncType>;
    using Base::Base;  // Inherit constructor

    template <typename ConditionFunc, typename... TArgs>
    auto operator()(ConditionFunc condition,
                    TArgs &&...args) const -> decltype(this->func(args...)) {
        if (condition()) {
            return Base::operator()(std::forward<TArgs>(args)...);
        } else {
            // 处理不满足条件时的逻辑
            return decltype(this->func(args...))();
        }
    }
};

template <typename FuncType>
ConditionCheckDecorator<FuncType> make_condition_check_decorator(FuncType f) {
    return ConditionCheckDecorator<FuncType>(f);
}

struct DecoratorError : public std::exception {
    std::string message;
    explicit DecoratorError(const std::string &msg) : message(msg) {}
    const char *what() const noexcept override { return message.c_str(); }
};

template <typename R, typename... Args>
class BaseDecorator {
public:
    using FuncType = std::function<R(Args...)>;
    virtual R operator()(FuncType func, Args &&...args) = 0;
};

template <typename R, typename... Args>
class DecorateStepper {
    using FuncType = std::function<R(Args...)>;
    using DecoratorPtr = std::unique_ptr<BaseDecorator<R, Args...>>;

    std::vector<DecoratorPtr> decorators;
    FuncType baseFunction;

public:
    explicit DecorateStepper(FuncType func) : baseFunction(std::move(func)) {}

    template <typename Decorator, typename... DArgs>
    void addDecorator(DArgs &&...args) {
        decorators.emplace_back(
            std::make_unique<Decorator>(std::forward<DArgs>(args)...));
    }

    R execute(Args... args) {
        try {
            FuncType currentFunction = baseFunction;

            for (auto &decorator : decorators) {
                currentFunction = [&,
                                   nextFunction = std::move(currentFunction)](
                                      Args... innerArgs) -> R {
                    return (*decorator)(nextFunction,
                                        std::forward<Args>(innerArgs)...);
                };
            }

            return currentFunction(std::forward<Args>(args)...);
        } catch (const DecoratorError &e) {
            return R();
        }
    }
};
}  // namespace atom::meta

#endif
