#include "var.hpp"

void VariableManager::setStringOptions(const std::string& name,
                                       std::vector<std::string> options) {
    LOG_F(INFO, "Setting string options for variable: %s", name.c_str());
    if (auto variable = getVariable<std::string>(name)) {
        stringOptions_[name] = std::move(options);
    }
}

void VariableManager::setValue(const std::string& name, const char* newValue) {
    LOG_F(INFO, "Setting value for variable: %s", name.c_str());
    setValue(name, std::string(newValue));
}

auto VariableManager::has(const std::string& name) const -> bool {
    LOG_F(INFO, "Checking if variable exists: %s", name.c_str());
    return variables_.find(name) != variables_.end();
}

auto VariableManager::getDescription(const std::string& name) const
    -> std::string {
    LOG_F(INFO, "Getting description for variable: %s", name.c_str());
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
    LOG_F(INFO, "Getting alias for variable: %s", name.c_str());
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
    LOG_F(INFO, "Getting group for variable: %s", name.c_str());
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
    LOG_F(INFO, "Removing variable: %s", name.c_str());
    variables_.erase(name);
    ranges_.erase(name);
    stringOptions_.erase(name);
}

auto VariableManager::getAllVariables() const -> std::vector<std::string> {
    LOG_F(INFO, "Getting all variables");
    std::vector<std::string> variableNames;
    for (const auto& [name, _] : variables_) {
        variableNames.push_back(name);
    }
    return variableNames;
}