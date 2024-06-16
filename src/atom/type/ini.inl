/*
 * ini.inl
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: INI File Read/Write Library

**************************************************/

#ifndef ATOM_TYPE_INI_INL
#define ATOM_TYPE_INI_INL

#include "ini.hpp"

namespace atom::type {

template <typename T>
void INIFile::set(const std::string &section, const std::string &key,
                  const T &value) {
    std::unique_lock lock(m_sharedMutex);
    data[section][key] = value;
}

template <typename T>
std::optional<T> INIFile::get(const std::string &section,
                              const std::string &key) const {
    std::shared_lock lock(m_sharedMutex);
    if (auto it = data.find(section); it != data.end()) {
        if (auto entryIt = it->second.find(key); entryIt != it->second.end()) {
            if constexpr (std::is_same_v<T, std::any>) {
                return entryIt->second;
            } else {
                return std::any_cast<T>(entryIt->second);
            }
        }
    }
    return std::nullopt;
}

}  // namespace atom::type

#endif