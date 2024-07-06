/*
 * var.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Variable Manager

**************************************************/

#ifndef ATOM_COMPONENT_VAR_HPP
#define ATOM_COMPONENT_VAR_HPP

#include <any>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/type/trackable.hpp"
#include "macro.hpp"

class VariableManager {
public:
    template <typename T>
    void addVariable(const std::string& name, T initialValue,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "");

    template <typename T>
    void setRange(const std::string& name, T min, T max);

    void setStringOptions(const std::string& name,
                          std::vector<std::string> options);

    template <typename T>
    auto getVariable(const std::string& name) -> std::shared_ptr<Trackable<T>>;

    void setValue(const std::string& name, const char* newValue);

    template <typename T>
    void setValue(const std::string& name, T newValue);

    auto has(const std::string& name) const -> bool;

    auto getDescription(const std::string& name) const -> std::string;

    auto getAlias(const std::string& name) const -> std::string;

    auto getGroup(const std::string& name) const -> std::string;

private:
    struct VariableInfo {
        std::any variable;
        std::string description;
        std::string alias;
        std::string group;
    } ATOM_ALIGNAS(128);

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, VariableInfo> variables_;
    emhash8::HashMap<std::string, std::any> ranges_;
    emhash8::HashMap<std::string, std::vector<std::string>> stringOptions_;
#else
    std::unordered_map<std::string, VariableInfo> variables_;
    std::unordered_map<std::string, std::any> ranges_;
    std::unordered_map<std::string, std::vector<std::string>> stringOptions_;
#endif
};

template <typename T>
ATOM_INLINE void VariableManager::addVariable(const std::string& name,
                                              T initialValue,
                                              const std::string& description,
                                              const std::string& alias,
                                              const std::string& group) {
    auto variable = std::make_shared<Trackable<T>>(std::move(initialValue));
    variables_[name] = {std::move(variable), description, alias, group};
}

template <typename T>
ATOM_INLINE void VariableManager::setRange(const std::string& name, T min,
                                           T max) {
    if (auto variable = getVariable<T>(name)) {
        ranges_[name] = std::make_pair(std::move(min), std::move(max));
    }
}

ATOM_INLINE void VariableManager::setStringOptions(
    const std::string& name, std::vector<std::string> options) {
    if (auto variable = getVariable<std::string>(name)) {
        stringOptions_[name] = std::move(options);
    }
}

template <typename T>
ATOM_INLINE auto VariableManager::getVariable(const std::string& name)
    -> std::shared_ptr<Trackable<T>> {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        try {
            return std::any_cast<std::shared_ptr<Trackable<T>>>(
                it->second.variable);
        } catch (const std::bad_any_cast& e) {
            THROW_INVALID_ARGUMENT("Type mismatch: ", name);
        }
    }
    return nullptr;
}

ATOM_INLINE auto VariableManager::has(const std::string& name) const -> bool {
    return variables_.find(name) != variables_.end();
}

ATOM_INLINE void VariableManager::setValue(const std::string& name,
                                           const char* newValue) {
    setValue(name, std::string(newValue));
}

template <typename T>
ATOM_INLINE void VariableManager::setValue(const std::string& name,
                                           T newValue) {
    if (auto variable = getVariable<T>(name)) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (ranges_.contains(name)) {
                auto [min, max] = std::any_cast<std::pair<T, T>>(ranges_[name]);
                if (newValue < min || newValue > max) {
                    THROW_OUT_OF_RANGE("Value out of range");
                }
            }
        } else if constexpr (std::is_same_v<T, std::string> ||
                             std::is_same_v<T, std::string_view>) {
            if (stringOptions_.contains(name)) {
                auto& options = stringOptions_[name];
                if (std::find(options.begin(), options.end(), newValue) ==
                    options.end()) {
                    THROW_INVALID_ARGUMENT("Invalid string option");
                }
            }
        }
        *variable = std::move(newValue);
    } else {
        THROW_OBJ_NOT_EXIST("Variable not found");
    }
}

ATOM_INLINE auto VariableManager::getDescription(const std::string& name) const
    -> std::string {
    if (auto it = variables_.find(name); it != variables_.end()) {
        return it->second.description;
    }
    for (const auto& [key, value] : variables_) {
        if (value.alias == name) {
            return value.description;
        }
    }
    return "";
}

ATOM_INLINE auto VariableManager::getAlias(const std::string& name) const
    -> std::string {
    if (auto it = variables_.find(name); it != variables_.end()) {
        return it->second.alias;
    }
    for (const auto& [key, value] : variables_) {
        if (value.alias == name) {
            return key;
        }
    }
    return "";
}

ATOM_INLINE auto VariableManager::getGroup(const std::string& name) const
    -> std::string {
    if (auto it = variables_.find(name); it != variables_.end()) {
        return it->second.group;
    }
    for (const auto& [key, value] : variables_) {
        if (value.alias == name) {
            return value.group;
        }
    }
    return "";
}

#endif
