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

#include <algorithm>
#include <any>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "macro.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/trackable.hpp"

class VariableManager {
public:
    template <typename T>
        requires std::is_copy_constructible_v<T>
    void addVariable(const std::string& name, T initialValue,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "");

    template <typename T, typename C>
        requires std::is_copy_constructible_v<T>
    void addVariable(const std::string& name, T C::*memberPointer, C& instance,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "");

    template <typename T>
        requires std::is_arithmetic_v<T>
    void setRange(const std::string& name, T min, T max);

    void setStringOptions(const std::string& name,
                          std::span<const std::string> options);

    template <typename T>
    [[nodiscard]] auto getVariable(const std::string& name)
        -> std::shared_ptr<Trackable<T>>;

    void setValue(const std::string& name, const char* newValue);

    template <typename T>
    void setValue(const std::string& name, T newValue);

    [[nodiscard]] bool has(const std::string& name) const;

    [[nodiscard]] std::string getDescription(const std::string& name) const;

    [[nodiscard]] std::string getAlias(const std::string& name) const;

    [[nodiscard]] std::string getGroup(const std::string& name) const;

    // New functionalities
    void removeVariable(const std::string& name);
    [[nodiscard]] std::vector<std::string> getAllVariables() const;

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
    requires std::is_copy_constructible_v<T>
void VariableManager::addVariable(const std::string& name, T initialValue,
                                  const std::string& description,
                                  const std::string& alias,
                                  const std::string& group) {
    LOG_F(INFO, "Adding variable: {}", name);
    auto variable = std::make_shared<Trackable<T>>(std::move(initialValue));
    variables_[name] = {std::move(variable), description, alias, group};
}

template <typename T, typename C>
    requires std::is_copy_constructible_v<T>
void VariableManager::addVariable(const std::string& name, T C::*memberPointer,
                                  C& instance, const std::string& description,
                                  const std::string& alias,
                                  const std::string& group) {
    LOG_F(INFO, "Adding variable with member pointer: {}", name);
    auto variable = std::make_shared<Trackable<T>>(instance.*memberPointer);
    variable->setOnChangeCallback(
        [&instance, memberPointer](const T& newValue) {
            instance.*memberPointer = newValue;
        });
    variables_[name] = {std::move(variable), description, alias, group};
}

template <typename T>
    requires std::is_arithmetic_v<T>
void VariableManager::setRange(const std::string& name, T min, T max) {
    LOG_F(INFO, "Setting range for variable: {}", name);
    if (auto variable = getVariable<T>(name)) {
        ranges_[name] = std::make_pair(std::move(min), std::move(max));
    }
}

template <typename T>
[[nodiscard]] auto VariableManager::getVariable(const std::string& name)
    -> std::shared_ptr<Trackable<T>> {
    LOG_F(INFO, "Getting variable: {}", name);
    if (auto it = variables_.find(name); it != variables_.end()) {
        try {
            return std::any_cast<std::shared_ptr<Trackable<T>>>(
                it->second.variable);
        } catch (const std::bad_any_cast& e) {
            LOG_F(ERROR, "Type mismatch for variable: {}", name);
            THROW_INVALID_ARGUMENT("Type mismatch: ", name);
        }
    }
    return nullptr;
}

template <typename T>
void VariableManager::setValue(const std::string& name, T newValue) {
    LOG_F(INFO, "Setting value for variable: {}", name);
    if (auto variable = getVariable<T>(name)) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (ranges_.contains(name)) {
                auto [min, max] = std::any_cast<std::pair<T, T>>(ranges_[name]);
                if (newValue < min || newValue > max) {
                    LOG_F(ERROR, "Value out of range for variable: {}",
                          name);
                    THROW_OUT_OF_RANGE("Value out of range");
                }
            }
        } else if constexpr (std::is_same_v<T, std::string> ||
                             std::is_same_v<T, std::string_view>) {
            if (stringOptions_.contains(name)) {
                auto& options = stringOptions_[name];
                if (std::ranges::find(options.begin(), options.end(), newValue) ==
                    options.end()) {
                    LOG_F(ERROR, "Invalid string option for variable: {}",
                          name);
                    THROW_INVALID_ARGUMENT("Invalid string option");
                }
            }
        }
        *variable = std::move(newValue);
    } else {
        LOG_F(ERROR, "Variable not found: {}", name);
        THROW_OBJ_NOT_EXIST("Variable not found");
    }
}

#endif  // ATOM_COMPONENT_VAR_HPP
