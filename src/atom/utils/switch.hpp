/*
 * switch.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: Smart Switch just like javascript

**************************************************/

#ifndef ATOM_UTILS_SWITCH_HPP
#define ATOM_UTILS_SWITCH_HPP

#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"
#include "macro.hpp"

namespace atom::utils {

/**
 * @brief A class for implementing a switch statement with string cases,
 * enhanced with C++20 features.
 *
 * This class allows you to register functions associated with string keys,
 * similar to a switch statement in JavaScript. It supports multiple return
 * types using std::variant and provides a default function if no match is
 * found.
 *
 * @tparam Args The types of additional arguments to pass to the functions.
 */
template <typename... Args>
class StringSwitch : public NonCopyable {
public:
    /**
     * @brief Type alias for the function to be registered.
     *
     * The function can return a std::variant containing either std::monostate,
     * int, or std::string.
     */
    using Func =
        std::function<std::variant<std::monostate, int, std::string>(Args...)>;

    /**
     * @brief Type alias for the default function.
     *
     * The default function is optional and can be set to handle cases where no
     * match is found.
     */
    using DefaultFunc = std::optional<Func>;

    /**
     * @brief Default constructor.
     */
    StringSwitch() = default;

    /**
     * @brief Register a case with the given string and function.
     *
     * @param str The string key for the case.
     * @param func The function to be associated with the string key.
     * @throws std::runtime_error if the case is already registered.
     */
    void registerCase(const std::string &str, Func func) {
        if (cases_.contains(str)) {
            THROW_OBJ_ALREADY_EXIST("Case already registered");
        }
        cases_[str] = std::move(func);  // Use move semantics for efficiency
    }

    /**
     * @brief Unregister a case with the given string.
     *
     * @param str The string key for the case to be unregistered.
     */
    void unregisterCase(const std::string &str) { cases_.erase(str); }

    /**
     * @brief Clear all registered cases.
     */
    void clearCases() { cases_.clear(); }

    /**
     * @brief Match the given string against the registered cases.
     *
     * @param str The string key to match.
     * @param args Additional arguments to pass to the function.
     * @return std::optional<std::variant<std::monostate, int, std::string>> The
     * result of the function call, or std::nullopt if no match is found.
     */
    auto match(const std::string &str, Args... args)
        -> std::optional<std::variant<std::monostate, int, std::string>> {
        if (auto iter = cases_.find(str); iter != cases_.end()) {
            return std::invoke(iter->second, args...);
        }

        if (defaultFunc_) {
            return std::invoke(*defaultFunc_, args...);
        }

        return std::nullopt;
    }

    /**
     * @brief Set the default function to be called if no match is found.
     *
     * @param func The default function.
     */
    void setDefault(DefaultFunc func) { defaultFunc_ = std::move(func); }

    /**
     * @brief Get a vector of all registered cases.
     *
     * @return std::vector<std::string> A vector containing all registered
     * string keys.
     */
    ATOM_NODISCARD auto getCases() const -> std::vector<std::string> {
        std::vector<std::string> caseList;
        for (const auto &[key, value] :
             cases_) {  // Use structured bindings for clarity
            caseList.push_back(key);
        }
        return caseList;
    }

    /**
     * @brief C++17 deduction guide for easier initialization.
     *
     * @tparam T The type of the function to be registered.
     * @param str The string key for the case.
     * @param func The function to be associated with the string key.
     */
    template <typename T,
              typename = std::enable_if_t<std::is_invocable_v<T, Args...>>>
    void registerCase(const std::string &str, T &&func) {
        registerCase(str, Func(std::forward<T>(func)));
    }

    /**
     * @brief C++20 designated initializers for easier case registration.
     *
     * @param initList An initializer list of pairs containing string keys and
     * functions.
     */
    StringSwitch(std::initializer_list<std::pair<std::string, Func>> initList) {
        for (auto [str, func] : initList) {
            registerCase(str, std::move(func));
        }
    }

    /**
     * @brief Match the given string against the registered cases with a span of
     * arguments.
     *
     * @param str The string key to match.
     * @param args A span of additional arguments to pass to the function.
     * @return std::optional<std::variant<std::monostate, int, std::string>> The
     * result of the function call, or std::nullopt if no match is found.
     */
    auto matchWithSpan(const std::string &str, std::span<Args...> args)
        -> std::optional<std::variant<std::monostate, int, std::string>> {
        if (auto iter = cases_.find(str); iter != cases_.end()) {
            return std::apply(iter->second, args);
        }

        if (defaultFunc_) {
            return std::apply(*defaultFunc_, args);
        }

        return std::nullopt;
    }

    /**
     * @brief Get a vector of all registered cases using ranges.
     *
     * @return std::vector<std::string> A vector containing all registered
     * string keys.
     */
    ATOM_NODISCARD auto getCasesWithRanges() const -> std::vector<std::string> {
        return cases_ | std::views::keys | std::ranges::to<std::vector>();
    }

private:
    std::unordered_map<std::string, Func>
        cases_;                ///< A map of string keys to functions.
    DefaultFunc defaultFunc_;  ///< The default function to be called if no
                               ///< match is found.
};

}  // namespace atom::utils

#endif