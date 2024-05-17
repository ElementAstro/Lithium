/*
 * global_ptr.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Global shared pointer manager

**************************************************/

#include "global_ptr.hpp"

#include <iostream>
#include <sstream>

GlobalSharedPtrManager &GlobalSharedPtrManager::getInstance() {
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

size_t GlobalSharedPtrManager::size() const {
    std::shared_lock lock(mtx);
    return sharedPtrMap.size();
}

void GlobalSharedPtrManager::printSharedPtrMap() const {
    std::shared_lock lock(mtx);
    std::cout << "GlobalSharedPtrManager:\n";
    for (const auto &pair : sharedPtrMap) {
        std::cout << "  " << pair.first << "\n";
    }
}
