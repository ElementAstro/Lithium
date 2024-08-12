/*!
 * \file global_ptr.cpp
 * \brief Global shared pointer manager
 * \author Max Qian <lightapt.com>
 * \date 2023-06-17
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "global_ptr.hpp"

#if ENABLE_DEBUG
#include <iostream>
#include <sstream>
#endif

auto GlobalSharedPtrManager::getInstance() -> GlobalSharedPtrManager & {
    static GlobalSharedPtrManager instance;
    return instance;
}

void GlobalSharedPtrManager::removeSharedPtr(const std::string &key) {
    std::unique_lock lock(mtx);
    sharedPtrMap.erase(key);
}

void GlobalSharedPtrManager::removeExpiredWeakPtrs() {
    std::unique_lock lock(mtx);
    auto it = sharedPtrMap.begin();
    while (it != sharedPtrMap.end()) {
        try {
            if (std::any_cast<std::weak_ptr<void>>(it->second).expired()) {
                it = sharedPtrMap.erase(it);
            } else {
                ++it;
            }
        } catch (const std::bad_any_cast &) {
            ++it;
        }
    }
}

void GlobalSharedPtrManager::clearAll() {
    std::unique_lock lock(mtx);
    sharedPtrMap.clear();
}

auto GlobalSharedPtrManager::size() const -> size_t {
    std::shared_lock lock(mtx);
    return sharedPtrMap.size();
}

void GlobalSharedPtrManager::printSharedPtrMap() const {
    std::shared_lock lock(mtx);
#if ENABLE_DEBUG
    std::cout << "GlobalSharedPtrManager:\n";
    for (const auto &pair : sharedPtrMap) {
        std::cout << "  " << pair.first << "\n";
    }
#endif
}
