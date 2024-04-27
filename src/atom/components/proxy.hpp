/*
 * proxy.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Proxy Function Implementation

**************************************************/

#ifndef ATOM_COMPONENT_PROXY_HPP
#define ATOM_COMPONENT_PROXY_HPP

#include <any>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include <utility>

template <typename Ret, typename... Args>
struct ProxyFunction {
    std::function<Ret(Args...)> func;
    static constexpr std::size_t N = sizeof...(Args);

    ProxyFunction(std::function<Ret(Args...)> func) : func(std::move(func)) {}

    std::any operator()(const std::vector<std::any>& args) {
        if (args.size() != N)
            throw std::runtime_error("Number of arguments does not match");
        return call(args, std::make_index_sequence<N>());
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

template <typename Ret, typename Class, typename... Args>
struct ProxyFunction<Ret (Class::*)(Args...)> {
    using MemFuncPtr = Ret (Class::*)(Args...);
    MemFuncPtr memFunc;
    static constexpr std::size_t N = sizeof...(Args) + 1;

    ProxyFunction(MemFuncPtr func) : memFunc(func) {}

    std::any operator()(const std::vector<std::any>& args) {
        if (args.size() != N)
            throw std::runtime_error("Number of arguments does not match");
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
            throw std::runtime_error("Number of arguments does not match");
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

#endif
