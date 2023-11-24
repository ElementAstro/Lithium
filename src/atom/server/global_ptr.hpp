/*
 * global_ptr.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-6-17

Description: Global shared pointer manager

**************************************************/

#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <any>
#include <memory>
#include <mutex>
#include <string>
#include <functional>

#define GetPtr GlobalSharedPtrManager::getInstance().getSharedPtr
#define AddPtr GlobalSharedPtrManager::getInstance().addSharedPtr

class GlobalSharedPtrManager
{
private:
    std::unordered_map<std::string, std::any> sharedPtrMap;
    mutable std::shared_mutex mtx;

    GlobalSharedPtrManager() {}

public:
    static GlobalSharedPtrManager &getInstance()
    {
        static GlobalSharedPtrManager instance;
        return instance;
    }

    template <typename T>
    std::shared_ptr<T> getSharedPtr(const std::string &key)
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto it = sharedPtrMap.find(key);
        if (it != sharedPtrMap.end())
        {
            try
            {
                return std::any_cast<std::shared_ptr<T>>(it->second);
            }
            catch (const std::bad_any_cast &)
            {
                return nullptr; // 类型转换失败，返回空指针
            }
        }

        return nullptr;
    }

    template <typename T>
    void addSharedPtr(const std::string &key, std::shared_ptr<T> sharedPtr)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        sharedPtrMap[key] = sharedPtr;
    }

    void removeSharedPtr(const std::string &key);

    template <typename T>
    void addWeakPtr(const std::string &key, const std::weak_ptr<T> &weakPtr)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        sharedPtrMap[key] = weakPtr;
    }

    template <typename T>
    std::shared_ptr<T> getSharedPtrFromWeakPtr(const std::string &key)
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto it = sharedPtrMap.find(key);
        if (it != sharedPtrMap.end())
        {
            try
            {
                return std::any_cast<std::weak_ptr<T>>(it->second).lock();
            }
            catch (const std::bad_any_cast &)
            {
                return nullptr; // 类型转换失败，返回空指针
            }
        }

        return nullptr;
    }

    template <typename T>
    void removeExpiredWeakPtrs()
    {
        std::vector<std::string> keysToRemove;
        {
            std::shared_lock<std::shared_mutex> lock(mtx);
            for (auto &it : sharedPtrMap)
            {
                if (auto weakPtr = std::any_cast<std::weak_ptr<T>>(it.second); weakPtr.expired())
                {
                    keysToRemove.push_back(it.first);
                }
            }
        }

        if (!keysToRemove.empty())
        {
            std::unique_lock<std::shared_mutex> lock(mtx);
            for (auto &key : keysToRemove)
            {
                sharedPtrMap.erase(key);
            }
        }
    }

    template <typename T>
    void addDeleter(const std::string &key, const std::function<void(T *)> &deleter)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        sharedPtrMap[key] = deleter;
    }

    template <typename T>
    void deleteObject(const std::string &key, T *ptr)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        auto it = sharedPtrMap.find(key);
        if (it != sharedPtrMap.end())
        {
            try
            {
                std::function<void(T *)> deleter = std::any_cast<std::function<void(T *)>>(it->second);
                deleter(ptr);
            }
            catch (const std::bad_any_cast &)
            {
                // 类型转换失败，不做处理
            }
        }
    }

    void printSharedPtrMap() const;
};