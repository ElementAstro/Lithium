/*
 * args.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-28

Description: Argument Container Library for C++

**************************************************/

#include "args.hpp"

bool ArgumentContainer::remove(const std::string &name) {
    if (m_arguments.find(name) == m_arguments.end()) {
        return false;
    }
    return m_arguments.erase(name) != 0;
}

bool ArgumentContainer::contains(const std::string &name) const {
    return m_arguments.count(name) != 0;
}

std::size_t ArgumentContainer::size() const { return m_arguments.size(); }

std::vector<std::string> ArgumentContainer::getNames() const {
    std::vector<std::string> names;
    names.reserve(m_arguments.size());
    for (const auto &pair : m_arguments) {
        names.push_back(pair.first);
    }
    return names;
}

std::string ArgumentContainer::toJson() const {
    std::string json;
    json += "{";
    for (const auto &pair : m_arguments) {
        json += "\"" + pair.first + "\":";
        if (pair.second.type() == typeid(std::string)) {
            json += "\"" + std::any_cast<std::string>(pair.second) + "\"";
        } else if (pair.second.type() == typeid(int)) {
            json += std::to_string(std::any_cast<int>(pair.second));
        } else if (pair.second.type() == typeid(double)) {
            json += std::to_string(std::any_cast<double>(pair.second));
        } else if (pair.second.type() == typeid(bool)) {
            json += std::to_string(std::any_cast<bool>(pair.second));
        } else {
            json += "null";
        }
        json += ",";
    }
    json += "}";
    return json;
}