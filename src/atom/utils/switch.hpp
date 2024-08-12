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
#include <string>
#include <type_traits>
#include <vector>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"
#include "macro.hpp"

namespace atom::utils {
/**
 * @brief A class for implementing a switch statement with string cases,
 * enhanced with C++17/20 features.
 *
 * @tparam Args The types of additional arguments to pass to the functions.
 */
template <typename... Args>
class StringSwitch : public NonCopyable {
public:
    using Func = std::function<void(Args...)>;
    using DefaultFunc = std::optional<Func>;

    StringSwitch() = default;

    // Register a case with the given string and function
    void registerCase(const std::string &str, Func func) {
        if (cases_.find(str) != cases_.end()) {
            THROW_OBJ_ALREADY_EXIST("Case already registered");
        }
        cases_[str] = std::move(func);  // Use move semantics for efficiency
    }

    // Unregister a case with the given string
    void unregisterCase(const std::string &str) { cases_.erase(str); }

    // Clear all registered cases
    void clearCases() { cases_.clear(); }

    // Match the given string against the registered cases
    auto match(const std::string &str, Args... args) -> bool {
        auto iter = cases_.find(str);
        if (iter != cases_.end()) {
            std::invoke(iter->second, args...);
            return true;
        }

        if (defaultFunc_) {
            std::invoke(*defaultFunc_,
                        args...);  // Use optional's value() for clarity
            return true;
        }

        return false;
    }

    // Set the default function to be called if no match is found
    void setDefault(DefaultFunc func) { defaultFunc_ = std::move(func); }

    // Get a vector of all registered cases
    ATOM_NODISCARD auto getCases() const -> std::vector<std::string> {
        std::vector<std::string> caseList;
        for (const auto &[key, value] :
             cases_) {  // Use structured bindings for clarity
            caseList.push_back(key);
        }
        return caseList;
    }

    // C++17 deduction guide for easier initialization
    template <typename T,
              typename = std::enable_if_t<std::is_invocable_v<T, Args...>>>
    void registerCase(const std::string &str, T &&func) {
        registerCase(str, Func(std::forward<T>(func)));
    }

    // C++20 designated initializers for easier case registration
    StringSwitch(std::initializer_list<std::pair<std::string, Func>> initList) {
        for (auto [str, func] : initList) {
            registerCase(str, std::move(func));
        }
    }

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, Func> cases_;
#else
    std::unordered_map<std::string, Func> cases_;
#endif
    DefaultFunc defaultFunc_;
};

}  // namespace atom::utils

#endif
