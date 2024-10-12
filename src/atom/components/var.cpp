#include "var.hpp"

void VariableManager::setStringOptions(const std::string& name,
                                       std::span<const std::string> options) {
    LOG_F(INFO, "Setting string options for variable: {}", name);
    if (auto variable = getVariable<std::string>(name)) {
        stringOptions_[name] =
            std::vector<std::string>(options.begin(), options.end());
    }
}

void VariableManager::setValue(const std::string& name, const char* newValue) {
    LOG_F(INFO, "Setting value for variable: {}", name);
    setValue(name, std::string(newValue));
}

auto VariableManager::has(const std::string& name) const -> bool {
    LOG_F(INFO, "Checking if variable exists: {}", name);
    return variables_.contains(name);
}

auto VariableManager::getDescription(const std::string& name) const
    -> std::string {
    LOG_F(INFO, "Getting description for variable: {}", name);
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

auto VariableManager::getAlias(const std::string& name) const -> std::string {
    LOG_F(INFO, "Getting alias for variable: {}", name);
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

auto VariableManager::getGroup(const std::string& name) const -> std::string {
    LOG_F(INFO, "Getting group for variable: {}", name);
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

void VariableManager::removeVariable(const std::string& name) {
    LOG_F(INFO, "Removing variable: {}", name);
    variables_.erase(name);
    ranges_.erase(name);
    stringOptions_.erase(name);
}

auto VariableManager::getAllVariables() const -> std::vector<std::string> {
    LOG_F(INFO, "Getting all variables");
    std::vector<std::string> variableNames;
    variableNames.reserve(variables_.size());
    for (const auto& [name, _] : variables_) {
        variableNames.push_back(name);
    }
    return variableNames;
}
