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

#include "atom/error/exception.hpp"
#include "atom/function/func_traits.hpp"

template <typename Func>
struct ProxyFunction {
    Func func;

    using Traits = FunctionTraits<Func>;
    static constexpr std::size_t N = Traits::arity;

    ProxyFunction(Func func) : func(std::move(func)) {}

    std::any operator()(const std::vector<std::any>& args) {
        if (args.size() != N) {
            throw std::runtime_error("Incorrect number of arguments");
        }

        return call(args, std::make_index_sequence<N>());
    }

private:
    template <std::size_t... Is>
    std::any call(const std::vector<std::any>& args,
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
};

template <typename Ret, typename Class, typename... Args>
struct ProxyFunction<Ret (Class::*)(Args...)> {
    using MemFuncPtr = Ret (Class::*)(Args...);
    MemFuncPtr memFunc;
    static constexpr std::size_t N = sizeof...(Args) + 1;

    ProxyFunction(MemFuncPtr func) : memFunc(func) {}

    std::any operator()(const std::vector<std::any>& args) {
        if (args.size() != N)
            THROW_EXCEPTION("Number of arguments does not match");
        return call(args, std::make_index_sequence<N>());
    }

private:
    template <std::size_t... Is>
    std::any call(const std::vector<std::any>& args,
                  std::index_sequence<Is...>) {
        if constexpr (std::is_void_v<Ret>) {
            (std::any_cast<Class&>(args[0]).*memFunc)(
                std::any_cast<std::remove_cvref_t<Args>>(args[Is + 1])...);
            return {};
        } else {
            return std::make_any<Ret>((std::any_cast<Class&>(args[0]).*memFunc)(
                std::any_cast<std::remove_cvref_t<Args>>(args[Is + 1])...));
        }
    }
};

template <typename Ret, typename Class, typename... Args>
struct ProxyFunction<Ret (Class::*)(Args...) const> {
    using ConstMemFuncPtr = Ret (Class::*)(Args...) const;
    ConstMemFuncPtr memFunc;
    static constexpr std::size_t N = sizeof...(Args) + 1;

    ProxyFunction(ConstMemFuncPtr func) : memFunc(func) {}

    std::any operator()(const std::vector<std::any>& args) {
        if (args.size() != N)
            THROW_EXCEPTION("Number of arguments does not match");
        return call(args, std::make_index_sequence<N>());
    }

private:
    template <std::size_t... Is>
    std::any call(const std::vector<std::any>& args,
                  std::index_sequence<Is...>) {
        if constexpr (std::is_void_v<Ret>) {
            (std::any_cast<const Class&>(args[0]).*memFunc)(
                std::any_cast<std::remove_cvref_t<Args>>(args[Is + 1])...);
            return {};
        } else {
            return std::make_any<Ret>(
                (std::any_cast<const Class&>(args[0]).*memFunc)(
                    std::any_cast<std::remove_cvref_t<Args>>(args[Is + 1])...));
        }
    }
};

template <typename Func>
struct TimerProxyFunction;

template <typename Ret, typename... Args>
struct TimerProxyFunction<Ret(Args...)> {
    std::function<Ret(Args...)> func;
    static constexpr std::size_t N = sizeof...(Args);

    template <typename F>
    explicit TimerProxyFunction(F&& f) : func(std::forward<F>(f)) {}

    template <typename C>
    explicit TimerProxyFunction(Ret (C::*f)(Args...)) {
        func = [f](Args... args) {
            C* obj = nullptr;
            return (obj->*f)(std::forward<Args>(args)...);
        };
    }

    template <typename C>
    explicit TimerProxyFunction(Ret (C::*f)(Args...) const) {
        func = [f](Args... args) {
            const C* obj = nullptr;
            return (obj->*f)(std::forward<Args>(args)...);
        };
    }

    std::any operator()(const std::vector<std::any>& args,
                        std::chrono::milliseconds timeout) {
        if (args.size() != N)
            THROW_EXCEPTION("Number of arguments does not match");

        std::packaged_task<std::any()> task([this, &args]() {
            return call(args, std::make_index_sequence<N>());
        });
        std::future<std::any> future = task.get_future();
        std::jthread thread(std::move(task));

        if (future.wait_for(timeout) == std::future_status::timeout) {
            thread.request_stop();
            THROW_EXCEPTION("Function execution timed out");
        }

        return future.get();
    }

private:
    template <std::size_t... Is>
    std::any call(const std::vector<std::any>& args,
                  std::index_sequence<Is...>) {
        if constexpr (std::is_void_v<Ret>) {
            std::invoke(func,
                        std::any_cast<std::remove_cvref_t<Args>>(args[Is])...);
            return {};
        } else {
            return std::make_any<Ret>(std::invoke(
                func, std::any_cast<std::remove_cvref_t<Args>>(args[Is])...));
        }
    }
};

#endif
