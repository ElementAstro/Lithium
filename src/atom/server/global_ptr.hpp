/*
 * global_ptr.hpp
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

#pragma once

#include <shared_mutex>
#include <any>
#include <memory>
#include <mutex>
#include <string>
#include <functional>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#define GetPtr GlobalSharedPtrManager::getInstance().getSharedPtr
#define AddPtr GlobalSharedPtrManager::getInstance().addSharedPtr

/**
 * @brief The GlobalSharedPtrManager class manages a collection of shared pointers and weak pointers.
 *        It provides functions to add, remove, and retrieve shared pointers and weak pointers by key.
 */
class GlobalSharedPtrManager
{
public:
    /**
     * @brief getInstance returns the singleton instance of the GlobalSharedPtrManager.
     *
     * @return the singleton instance of the GlobalSharedPtrManager.
     */
    static GlobalSharedPtrManager &getInstance();

    /**
     * @brief getSharedPtr retrieves a shared pointer from the shared pointer map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @return the shared pointer if found, nullptr otherwise.
     */
    template <typename T>
    std::shared_ptr<T> getSharedPtr(const std::string &key);

    /**
     * @brief addSharedPtr adds a shared pointer to the shared pointer map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @param sharedPtr the shared pointer to add.
     */
    template <typename T>
    void addSharedPtr(const std::string &key, std::shared_ptr<T> sharedPtr);

    /**
     * @brief removeSharedPtr removes a shared pointer from the shared pointer map with the specified key.
     *
     * @param key the key associated with the shared pointer to remove.
     */
    void removeSharedPtr(const std::string &key);

    /**
     * @brief addWeakPtr adds a weak pointer to the shared pointer map with the specified key.
     *
     * @tparam T the type of the weak pointer.
     * @param key the key associated with the weak pointer.
     * @param weakPtr the weak pointer to add.
     */
    template <typename T>
    void addWeakPtr(const std::string &key, const std::weak_ptr<T> &weakPtr);

    /**
     * @brief getSharedPtrFromWeakPtr retrieves a shared pointer from a weak pointer in the shared pointer map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the weak pointer.
     * @return the shared pointer if the weak pointer is valid and the shared object still exists, nullptr otherwise.
     */
    template <typename T>
    std::shared_ptr<T> getSharedPtrFromWeakPtr(const std::string &key);

    /**
     * @brief removeExpiredWeakPtrs removes all expired weak pointers from the shared pointer map.
     *        Expired weak pointers are weak pointers whose shared objects have been deleted.
     */
    template <typename T>
    void removeExpiredWeakPtrs();

    /**
     * @brief addDeleter adds a custom deleter function for the shared object associated with the specified key.
     *
     * @tparam T the type of the shared object.
     * @param key the key associated with the shared object.
     * @param deleter the deleter function that takes a pointer to the shared object as its parameter.
     */
    template <typename T>
    void addDeleter(const std::string &key, const std::function<void(T *)> &deleter);

    /**
     * @brief deleteObject deletes the shared object associated with the specified key.
     *
     * @tparam T the type of the shared object.
     * @param key the key associated with the shared object to delete.
     * @param ptr a pointer to the shared object.
     */
    template <typename T>
    void deleteObject(const std::string &key, T *ptr);

    /**
     * @brief printSharedPtrMap prints the contents of the shared pointer map.
     */
    void printSharedPtrMap() const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> sharedPtrMap;
#else
    std::unordered_map<std::string, std::any> sharedPtrMap; /**< The map that stores the shared pointers and weak pointers. */
#endif
    mutable std::shared_mutex mtx;                          /**< The mutex used for thread-safe access to the shared pointer map. */

    /**
     * @brief GlobalSharedPtrManager is a singleton class.
     */
    GlobalSharedPtrManager() {}
};

template <typename T>
std::shared_ptr<T> GlobalSharedPtrManager::getSharedPtr(const std::string &key)
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
void GlobalSharedPtrManager::addSharedPtr(const std::string &key, std::shared_ptr<T> sharedPtr)
{
    std::unique_lock<std::shared_mutex> lock(mtx);
    sharedPtrMap[key] = sharedPtr;
}

template <typename T>
void GlobalSharedPtrManager::addWeakPtr(const std::string &key, const std::weak_ptr<T> &weakPtr)
{
    std::unique_lock<std::shared_mutex> lock(mtx);
    sharedPtrMap[key] = weakPtr;
}

template <typename T>
std::shared_ptr<T> GlobalSharedPtrManager::getSharedPtrFromWeakPtr(const std::string &key)
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
void GlobalSharedPtrManager::removeExpiredWeakPtrs()
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
void GlobalSharedPtrManager::addDeleter(const std::string &key, const std::function<void(T *)> &deleter)
{
    std::unique_lock<std::shared_mutex> lock(mtx);
    sharedPtrMap[key] = deleter;
}

template <typename T>
void GlobalSharedPtrManager::deleteObject(const std::string &key, T *ptr)
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