/*
 * argsview.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-28

Description: Argument View for C++20

**************************************************/

#ifndef ATOM_TYPE_ARGSVIEW_HPP
#define ATOM_TYPE_ARGSVIEW_HPP

#include <any>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename... Args>
class ArgsView {
public:
    constexpr ArgsView(Args&&... args) noexcept
        : args_(std::forward<Args>(args)...) {}

    template <typename... OtherArgs,
              std::enable_if_t<
                  std::conjunction_v<std::is_constructible<Args, OtherArgs>...>,
                  int> = 0>
    constexpr ArgsView(const std::tuple<OtherArgs...>& other_tuple)
        : args_(std::apply(
              [](const auto&... args) {
                  return std::tuple(std::forward<decltype(args)>(args)...);
              },
              other_tuple)) {}

    template <typename... OtherArgs,
              std::enable_if_t<
                  std::conjunction_v<std::is_constructible<Args, OtherArgs>...>,
                  int> = 0>
    constexpr ArgsView(ArgsView<OtherArgs...> other_args_view)
        : args_(std::apply(
              [](const auto&... args) {
                  return std::tuple(std::forward<decltype(args)>(args)...);
              },
              other_args_view.args_)) {}

    template <size_t I>
    constexpr decltype(auto) get() const noexcept {
        return std::get<I>(args_);
    }

    constexpr size_t size() const noexcept { return sizeof...(Args); }

    constexpr bool empty() const noexcept { return size() == 0; }

    template <typename Func>
    constexpr void for_each(Func&& func) const {
        apply([&func](auto&&... args) {
            (func(std::forward<decltype(args)>(args)), ...);
        });
    }

    template <typename Func>
    constexpr auto transform(Func&& func) const {
        return ArgsView(std::apply(
            [&func](auto&&... args) {
                return std::make_tuple(
                    func(std::forward<decltype(args)>(args))...);
            },
            args_));
    }

    template <typename Func, typename Init>
    constexpr auto accumulate(Func&& func, Init init) const {
        return apply([&func, init = std::move(init)](auto&&... args) mutable {
            ((init = func(std::move(init), std::forward<decltype(args)>(args))),
             ...);
            return init;
        });
    }

    template <typename Func>
    constexpr auto apply(Func&& func) const {
        return std::apply(std::forward<Func>(func), args_);
    }

    template <typename... OtherArgs,
              std::enable_if_t<
                  std::conjunction_v<std::is_constructible<Args, OtherArgs>...>,
                  int> = 0>
    constexpr ArgsView& operator=(const std::tuple<OtherArgs...>& other_tuple) {
        args_ = std::apply(
            [](const auto&... args) {
                return std::tuple(std::forward<decltype(args)>(args)...);
            },
            other_tuple);
        return *this;
    }

    template <typename... OtherArgs,
              std::enable_if_t<
                  std::conjunction_v<std::is_constructible<Args, OtherArgs>...>,
                  int> = 0>
    constexpr ArgsView& operator=(ArgsView<OtherArgs...> other_args_view) {
        args_ = std::apply(
            [](const auto&... args) {
                return std::tuple(std::forward<decltype(args)>(args)...);
            },
            other_args_view.args_);
        return *this;
    }

    constexpr decltype(auto) begin() const noexcept {
        return std::apply(
                   [](const auto&... args) {
                       return std::tuple<const Args&...>(args...);
                   },
                   args_)
            .cbegin();
    }

    constexpr decltype(auto) end() const noexcept {
        return std::apply(
                   [](const auto&... args) {
                       return std::tuple<const Args&...>(args...);
                   },
                   args_)
            .cend();
    }

private:
    std::tuple<Args...> args_;
};

template <typename... Args>
ArgsView(Args&&...) -> ArgsView<std::decay_t<Args>...>;

template <typename... Args>
using ArgsViewT = ArgsView<std::decay_t<Args>...>;

template <typename... Args>
int sum(Args&&... args) {
    return ArgsView{std::forward<Args>(args)...}.accumulate(
        [](int a, int b) { return a + b; }, 0);
}

template <typename... Args>
std::string concat(Args&&... args) {
    return transform(
               [](const auto& arg) {
                   if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                                const char*>) {
                       return std::string(arg);
                   } else {
                       return std::to_string(arg);
                   }
               },
               make_args_view(std::forward<Args>(args)...))
        .accumulate([](std::string a, std::string b) { return a + b; },
                    std::string{});
}

template <typename Func, typename... Args>
constexpr auto apply(Func&& func, ArgsViewT<Args...> args_view) {
    return args_view.apply(std::forward<Func>(func));
}

template <typename Func, typename... Args>
constexpr void for_each(Func&& func, ArgsView<Args...> args_view) {
    args_view.for_each(std::forward<Func>(func));
}

template <typename Func, typename Init, typename... Args>
constexpr auto accumulate(Func&& func, Init init,
                          ArgsViewT<Args...> args_view) {
    return args_view.accumulate(std::forward<Func>(func), std::move(init));
}

template <typename... Args>
constexpr ArgsViewT<Args...> make_args_view(Args&&... args) {
    return ArgsViewT<Args...>(std::forward<Args>(args)...);
}

template <std::size_t I, typename... Args>
constexpr decltype(auto) get(ArgsView<Args...> args_view) {
    return args_view.template get<I>();
}

template <typename... Args1, typename... Args2>
constexpr bool operator==(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) {
    return lhs.size() == rhs.size() &&
           lhs.apply([&rhs](const auto&... lhs_args) {
               return rhs.apply([&lhs_args...](const auto&... rhs_args) {
                   return ((lhs_args == rhs_args) && ...);
               });
           });
}

template <typename... Args1, typename... Args2>
constexpr bool operator!=(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) {
    return !(lhs == rhs);
}

template <typename... Args1, typename... Args2>
constexpr bool operator<(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) {
    return lhs.apply([&rhs](const auto&... lhs_args) {
        return rhs.apply([&lhs_args...](const auto&... rhs_args) {
            return std::tie(lhs_args...) < std::tie(rhs_args...);
        });
    });
}

template <typename... Args1, typename... Args2>
constexpr bool operator<=(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) {
    return !(rhs < lhs);
}

template <typename... Args1, typename... Args2>
constexpr bool operator>(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) {
    return rhs < lhs;
}

template <typename... Args1, typename... Args2>
constexpr bool operator>=(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) {
    return !(lhs < rhs);
}

template <typename... Args>
struct std::hash<ArgsView<Args...>> {
    std::size_t operator()(ArgsView<Args...> args_view) const {
        return args_view.apply([](const auto&... args) {
            std::size_t seed = 0;
            ((seed ^= std::hash<std::decay_t<decltype(args)>>{}(args) +
                      0x9e3779b9 + (seed << 6) + (seed >> 2)),
             ...);
            return seed;
        });
    }
};

#ifdef __DEBUG__
#include <iostream>
template <typename... Args>
void print(Args&&... args) {
    ArgsView{std::forward<Args>(args)...}.for_each(
        [](const auto& arg) { std::cout << arg << ' '; });
    std::cout << '\n';
}
#endif

#endif
