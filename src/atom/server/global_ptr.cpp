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

#include <sstream>
#include <iostream>

GlobalSharedPtrManager &GlobalSharedPtrManager::getInstance()
{
    static GlobalSharedPtrManager instance;
    return instance;
}

void GlobalSharedPtrManager::removeSharedPtr(const std::string &key)
{
    std::unique_lock<std::shared_mutex> lock(mtx);
    sharedPtrMap.erase(key);
}

void GlobalSharedPtrManager::printSharedPtrMap() const
{
    std::shared_lock<std::shared_mutex> lock(mtx);
    std::cout << "Shared pointer map:" << std::endl;
    for (const auto &it : sharedPtrMap)
    {
        std::ostringstream oss;
        oss << it.second.type().name();
        std::cout << "- Key: " << it.first << ", Type: " << oss.str() << std::endl;
    }
}