/*
 * global_ptr.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Global shared pointer manager

**************************************************/

#ifndef ATOM_SERVER_GLOBAL_PTR_HPP
#define ATOM_SERVER_GLOBAL_PTR_HPP

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <type_traits>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/experiment/noncopyable.hpp"

#define GetPtr GlobalSharedPtrManager::getInstance().getSharedPtr
#define GetWeakPtr GlobalSharedPtrManager::getInstance().getWeakPtrFromSharedPtr
#define AddPtr GlobalSharedPtrManager::getInstance().addSharedPtr
#define RemovePtr GlobalSharedPtrManager::getInstance().removeSharedPtr
#define GetPtrOrCreate \
    GlobalSharedPtrManager::getInstance().getOrCreateSharedPtr

/**
 * @brief The GlobalSharedPtrManager class manages a collection of shared
 * pointers and weak pointers. It provides functions to add, remove, and
 * retrieve shared pointers and weak pointers by key.
 */
class GlobalSharedPtrManager : public NonCopyable {
public:
    /**
     * @brief getInstance returns the singleton instance of the
     * GlobalSharedPtrManager.
     *
     * @return the singleton instance of the GlobalSharedPtrManager.
     */
    static GlobalSharedPtrManager &getInstance();

    /**
     * @brief getSharedPtr retrieves a shared pointer from the shared pointer
     * map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @return the shared pointer if found, std::nullopt otherwise.
     */
    template <typename T>
    [[nodiscard]] std::optional<std::shared_ptr<T>> getSharedPtr(
        const std::string &key);

    /**
     * @brief getOrCreateSharedPtr retrieves a shared pointer from the shared
     * pointer map with the specified key. If the shared pointer does not exist,
     * it creates a new one using the provided creation function.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @param creator a function that creates a new shared pointer if it doesn't
     * exist.
     * @return the shared pointer.
     */
    template <typename T, typename CreatorFunc>
    std::shared_ptr<T> getOrCreateSharedPtr(const std::string &key,
                                            CreatorFunc creator);

    /**
     * @brief getWeakPtr retrieves a weak pointer from the shared pointer map
     * with the specified key.
     *
     * @tparam T the type of the weak pointer.
     * @param key the key associated with the weak pointer.
     * @return the weak pointer if found, an empty weak pointer otherwise.
     * @note The weak pointer is not guaranteed to be valid after the shared
     */
    template <typename T>
    [[nodiscard]] std::weak_ptr<T> getWeakPtr(const std::string &key);

    /**
     * @brief addSharedPtr adds a shared pointer to the shared pointer map with
     * the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @param sharedPtr the shared pointer to add.
     */
    template <typename T>
    void addSharedPtr(const std::string &key, std::shared_ptr<T> sharedPtr);

    /**
     * @brief removeSharedPtr removes a shared pointer from the shared pointer
     * map with the specified key.
     *
     * @param key the key associated with the shared pointer to remove.
     */
    void removeSharedPtr(const std::string &key);

    /**
     * @brief addWeakPtr adds a weak pointer to the shared pointer map with the
     * specified key.
     *
     * @tparam T the type of the weak pointer.
     * @param key the key associated with the weak pointer.
     * @param weakPtr the weak pointer to add.
     */
    template <typename T>
    void addWeakPtr(const std::string &key, const std::weak_ptr<T> &weakPtr);

    /**
     * @brief getSharedPtrFromWeakPtr retrieves a shared pointer from a weak
     * pointer in the shared pointer map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the weak pointer.
     * @return the shared pointer if the weak pointer is valid and the shared
     * object still exists, an empty shared pointer otherwise.
     */
    template <typename T>
    [[nodiscard]] std::shared_ptr<T> getSharedPtrFromWeakPtr(
        const std::string &key);

    /**
     * @brief getWeakPtrFromSharedPtr retrieves a weak pointer from a shared
     * pointer in the shared pointer map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @return the weak pointer if the shared pointer is valid, an empty weak
     * pointer otherwise.
     */
    template <typename T>
    [[nodiscard]] std::weak_ptr<T> getWeakPtrFromSharedPtr(
        const std::string &key);

    /**
     * @brief removeExpiredWeakPtrs removes all expired weak pointers from the
     * shared pointer map. Expired weak pointers are weak pointers whose shared
     * objects have been deleted.
     */
    void removeExpiredWeakPtrs();

    /**
     * @brief addDeleter adds a custom deleter function for the shared object
     * associated with the specified key.
     *
     * @tparam T the type of the shared object.
     * @param key the key associated with the shared object.
     * @param deleter the deleter function that takes a pointer to the shared
     * object as its parameter.
     */
    template <typename T>
    void addDeleter(const std::string &key,
                    const std::function<void(T *)> &deleter);

    /**
     * @brief deleteObject deletes the shared object associated with the
     * specified key using the custom deleter if available.
     *
     * @tparam T the type of the shared object.
     * @param key the key associated with the shared object to delete.
     * @param ptr a pointer to the shared object.
     */
    template <typename T>
    void deleteObject(const std::string &key, T *ptr);

    /**
     * @brief clearAll removes all shared pointers and weak pointers from the
     * shared pointer map.
     */
    void clearAll();

    /**
     * @brief size returns the number of elements in the shared pointer map.
     *
     * @return the number of elements in the shared pointer map.
     */
    size_t size() const;

    /**
     * @brief printSharedPtrMap prints the contents of the shared pointer map.
     */
    void printSharedPtrMap() const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> sharedPtrMap;
#else
    std::unordered_map<std::string, std::any>
        sharedPtrMap; /**< The map that stores the shared pointers and weak
                         pointers. */
#endif
    mutable std::shared_mutex mtx; /**< The mutex used for thread-safe access to
                                      the shared pointer map. */
};

template <typename T>
std::optional<std::shared_ptr<T>> GlobalSharedPtrManager::getSharedPtr(
    const std::string &key) {
    std::shared_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        } catch (const std::bad_any_cast &) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

template <typename T, typename CreatorFunc>
std::shared_ptr<T> GlobalSharedPtrManager::getOrCreateSharedPtr(
    const std::string &key, CreatorFunc creator) {
    std::unique_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        } catch (const std::bad_any_cast &) {
            // Key exists but the stored type does not match, replace it
            auto ptr = creator();
            sharedPtrMap[key] = ptr;
            return ptr;
        }
    } else {
        auto ptr = creator();
        sharedPtrMap[key] = ptr;
        return ptr;
    }
}

template <typename T>
std::weak_ptr<T> GlobalSharedPtrManager::getWeakPtr(const std::string &key) {
    std::shared_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            return std::any_cast<std::weak_ptr<T>>(it->second);
        } catch (const std::bad_any_cast &) {
            return std::weak_ptr<T>();
        }
    }
    return std::weak_ptr<T>();
}

template <typename T>
void GlobalSharedPtrManager::addSharedPtr(const std::string &key,
                                          std::shared_ptr<T> sharedPtr) {
    std::unique_lock lock(mtx);
    sharedPtrMap[key] = std::move(sharedPtr);
}

template <typename T>
void GlobalSharedPtrManager::addWeakPtr(const std::string &key,
                                        const std::weak_ptr<T> &weakPtr) {
    std::unique_lock lock(mtx);
    sharedPtrMap[key] = weakPtr;
}

template <typename T>
std::shared_ptr<T> GlobalSharedPtrManager::getSharedPtrFromWeakPtr(
    const std::string &key) {
    std::shared_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            return std::any_cast<std::weak_ptr<T>>(it->second).lock();
        } catch (const std::bad_any_cast &) {
            return std::shared_ptr<T>();
        }
    }
    return std::shared_ptr<T>();
}

template <typename T>
std::weak_ptr<T> GlobalSharedPtrManager::getWeakPtrFromSharedPtr(
    const std::string &key) {
    std::shared_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            return std::weak_ptr(std::any_cast<std::shared_ptr<T>>(it->second));
        } catch (const std::bad_any_cast &) {
            return std::weak_ptr<T>();
        }
    }
    return std::weak_ptr<T>();
}

template <typename T>
void GlobalSharedPtrManager::addDeleter(
    const std::string &key, const std::function<void(T *)> &deleter) {
    std::unique_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            auto ptr = std::any_cast<std::shared_ptr<T>>(it->second);
            ptr.reset(ptr.get(), deleter);
            sharedPtrMap[key] = ptr;
        } catch (const std::bad_any_cast &) {
            // Ignore if the stored type does not match
        }
    }
}

template <typename T>
void GlobalSharedPtrManager::deleteObject(const std::string &key, T *ptr) {
    std::unique_lock lock(mtx);
    auto it = sharedPtrMap.find(key);
    if (it != sharedPtrMap.end()) {
        try {
            auto deleter = std::any_cast<std::function<void(T *)>>(it->second);
            deleter(ptr);
        } catch (const std::bad_any_cast &) {
            delete ptr;  // Use default delete if no custom deleter is found
        }
        sharedPtrMap.erase(it);
    }
}

#endif  // ATOM_SERVER_GLOBAL_PTR_HPP
