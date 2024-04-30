#ifndef ATOM_COMPONENT_VAR_INL
#define ATOM_COMPONENT_VAR_INL

#include "var.hpp"

template <typename T>
inline void VariableManager::addVariable(const std::string& name, T initialValue,
                                  const std::string& description,
                                  const std::string& alias,
                                  const std::string& group) {
    auto variable = std::make_shared<Trackable<T>>(std::move(initialValue));
    variables_[name] = {std::move(variable), description, alias, group};
}

template <typename T>
inline void VariableManager::setRange(const std::string& name, T min, T max) {
    if (auto variable = getVariable<T>(name)) {
        ranges_[name] = std::make_pair(std::move(min), std::move(max));
    }
}

inline void VariableManager::setStringOptions(
    const std::string& name, std::vector<std::string> options) {
    if (auto variable = getVariable<std::string>(name)) {
        stringOptions_[name] = std::move(options);
    }
}

template <typename T>
inline std::shared_ptr<Trackable<T>> VariableManager::getVariable(
    const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        try {
            return std::any_cast<std::shared_ptr<Trackable<T>>>(
                it->second.variable);
        } catch (const std::bad_any_cast& e) {
            //THROW_EXCEPTION(concat("Type mismatch: ", name));
        }
    }
    return nullptr;
}

inline void VariableManager::setValue(const std::string& name, const char* newValue) {
    setValue(name, std::string(newValue));
}

template <typename T>
inline void VariableManager::setValue(const std::string& name, T newValue) {
    if (auto variable = getVariable<T>(name)) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (ranges_.count(name)) {
                auto [min, max] = std::any_cast<std::pair<T, T>>(ranges_[name]);
                if (newValue < min || newValue > max) {
                    THROW_EXCEPTION("Value out of range");
                }
            }
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (stringOptions_.count(name)) {
                auto& options = stringOptions_[name];
                if (std::find(options.begin(), options.end(), newValue) ==
                    options.end()) {
                    THROW_EXCEPTION("Invalid string option");
                }
            }
        }
        *variable = std::move(newValue);
    } else {
        THROW_EXCEPTION("Variable not found");
    }
}

#endif