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

namespace atom::type
{

template <typename T>
void INIFile::set(const std::string &section, const std::string &key,
                  const T &value) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    data[section][key] = value;
}

template <typename T>
std::optional<T> INIFile::get(const std::string &section,
                              const std::string &key) const {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = data.find(section);
    if (it != data.end()) {
        auto entryIt = it->second.find(key);
        if (entryIt != it->second.end()) {
            try {
                return std::any_cast<T>(entryIt->second);
            } catch (const std::bad_any_cast &) {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

}

#endif
