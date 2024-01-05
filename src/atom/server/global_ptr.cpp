/*
 * global_ptr.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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