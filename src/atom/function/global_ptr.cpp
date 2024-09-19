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
    std::unique_lock<std::shared_mutex> lock(mutex_);
    shared_ptr_map_.erase(key);
}

void GlobalSharedPtrManager::removeExpiredWeakPtrs() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto iter = shared_ptr_map_.begin();
    while (iter != shared_ptr_map_.end()) {
        try {
            if (std::any_cast<std::weak_ptr<void>>(iter->second).expired()) {
                iter = shared_ptr_map_.erase(iter);
            } else {
                ++iter;
            }
        } catch (const std::bad_any_cast &) {
            ++iter;
        }
    }
}

void GlobalSharedPtrManager::clearAll() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    shared_ptr_map_.clear();
}

auto GlobalSharedPtrManager::size() const -> size_t {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return shared_ptr_map_.size();
}

void GlobalSharedPtrManager::printSharedPtrMap() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
#if ENABLE_DEBUG
    std::cout << "GlobalSharedPtrManager:\n";
    for (const auto &pair : shared_ptr_map_) {
        std::cout << "  " << pair.first << "\n";
    }
#endif
}
