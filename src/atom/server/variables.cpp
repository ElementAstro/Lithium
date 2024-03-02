/*
 * variables.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-14

Description: Variable Registry 类，用于注册、获取和观察变量值。

**************************************************/

#include "variables.hpp"

VariableRegistry::VariableRegistry(const std::string &name) { m_name = name; }

bool VariableRegistry::HasVariable(const std::string &name) const {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return m_variables.find(name) != m_variables.end();
}

std::string VariableRegistry::GetDescription(const std::string &name) const {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);

    if (auto it = m_descriptions.find(name); it != m_descriptions.end()) {
        return it->second;
    }
    return "";
}

void VariableRegistry::AddObserver(const std::string &name,
                                   const Observer &observer) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (m_observers.find(name) == m_observers.end()) {
        m_observers[name].push_back(observer);
    }
}

std::unordered_map<std::string, std::any> VariableRegistry::GetAll() const {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return m_variables;
}

bool VariableRegistry::RemoveAll() {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    m_variables.clear();
    m_observers.clear();
    return true;
}

bool VariableRegistry::RemoveObserver(const std::string &name,
                                      const std::string &observerName) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (auto it = m_observers.find(name); it != m_observers.end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (it2->name == observerName) {
                it->second.erase(it2);
                return true;
            }
        }
    }
    return false;
}
