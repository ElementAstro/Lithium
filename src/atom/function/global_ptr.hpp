/*!
 * \file global_ptr.hpp
 * \brief Global shared pointer manager
 * \author Max Qian <lightapt.com>
 * \date 2023-06-17
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_GLOBAL_PTR_HPP
#define ATOM_META_GLOBAL_PTR_HPP

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/type/noncopyable.hpp"

#define GetPtr GlobalSharedPtrManager::getInstance().getSharedPtr
#define GetWeakPtr GlobalSharedPtrManager::getInstance().getWeakPtrFromSharedPtr
#define AddPtr GlobalSharedPtrManager::getInstance().addSharedPtr
#define RemovePtr GlobalSharedPtrManager::getInstance().removeSharedPtr
#define GetPtrOrCreate \
    GlobalSharedPtrManager::getInstance().getOrCreateSharedPtr

#define GET_OR_CREATE_PTR(variable, type, constant, ...)                     \
    if (auto ptr = GetPtrOrCreate<type>(                                     \
            constant, [] { return std::make_shared<type>(__VA_ARGS__); })) { \
        variable = ptr;                                                      \
    } else {                                                                 \
        THROW_UNLAWFUL_OPERATION("Failed to create " #type ".");             \
    }

#define GET_OR_CREATE_PTR_THIS(variable, type, constant, ...)    \
    if (auto ptr = GetPtrOrCreate<type>(constant, [this] {       \
            return std::make_shared<type>(__VA_ARGS__);          \
        })) {                                                    \
        variable = ptr;                                          \
    } else {                                                     \
        THROW_UNLAWFUL_OPERATION("Failed to create " #type "."); \
    }

#define GET_OR_CREATE_WEAK_PTR(variable, type, constant, ...)                \
    if (auto ptr = GetPtrOrCreate<type>(                                     \
            constant, [] { return std::make_shared<type>(__VA_ARGS__); })) { \
        variable = std::weak_ptr(ptr);                                       \
    } else {                                                                 \
        THROW_UNLAWFUL_OPERATION("Failed to create " #type ".");             \
    }

#define GET_OR_CREATE_PTR_WITH_DELETER(variable, type, constant, deleter) \
    if (auto ptr = GetPtrOrCreate<type>(constant, [deleter] {             \
            return std::shared_ptr<type>(new type, deleter);              \
        })) {                                                             \
        variable = ptr;                                                   \
    } else {                                                              \
        THROW_UNLAWFUL_OPERATION("Failed to create " #type ".");          \
    }

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
    static auto getInstance() -> GlobalSharedPtrManager &;

    /**
     * @brief getSharedPtr retrieves a shared pointer from the shared pointer
     * map with the specified key.
     *
     * @tparam T the type of the shared pointer.
     * @param key the key associated with the shared pointer.
     * @return the shared pointer if found, std::nullopt otherwise.
     */
    template <typename T>
    [[nodiscard]] auto getSharedPtr(const std::string &key)
        -> std::optional<std::shared_ptr<T>>;

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
    auto getOrCreateSharedPtr(const std::string &key,
                              CreatorFunc creator) -> std::shared_ptr<T>;

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
    [[nodiscard]] auto getWeakPtr(const std::string &key) -> std::weak_ptr<T>;

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
    [[nodiscard]] auto getSharedPtrFromWeakPtr(const std::string &key)
        -> std::shared_ptr<T>;

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
    [[nodiscard]] auto getWeakPtrFromSharedPtr(const std::string &key)
        -> std::weak_ptr<T>;

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
    auto size() const -> size_t;

    /**
     * @brief printSharedPtrMap prints the contents of the shared pointer map.
     */
    void printSharedPtrMap() const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> shared_ptr_map_;
#else
    std::unordered_map<std::string, std::any>
        shared_ptr_map_; /**< The map that stores the shared pointers and weak
                         pointers. */
#endif
    mutable std::shared_mutex mutex_; /**< The mutex used for thread-safe access
                                      to the shared pointer map. */
};

template <typename T>
auto GlobalSharedPtrManager::getSharedPtr(const std::string &key)
    -> std::optional<std::shared_ptr<T>> {
    std::shared_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            return std::any_cast<std::shared_ptr<T>>(iter->second);
        } catch (const std::bad_any_cast &) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

template <typename T, typename CreatorFunc>
auto GlobalSharedPtrManager::getOrCreateSharedPtr(
    const std::string &key, CreatorFunc creator) -> std::shared_ptr<T> {
    std::unique_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            return std::any_cast<std::shared_ptr<T>>(iter->second);
        } catch (const std::bad_any_cast &) {
            // Key exists but the stored type does not match, replace it
            auto ptr = creator();
            shared_ptr_map_[key] = ptr;
            return ptr;
        }
    } else {
        auto ptr = creator();
        shared_ptr_map_[key] = ptr;
        return ptr;
    }
}

template <typename T>
auto GlobalSharedPtrManager::getWeakPtr(const std::string &key)
    -> std::weak_ptr<T> {
    std::shared_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            return std::any_cast<std::weak_ptr<T>>(iter->second);
        } catch (const std::bad_any_cast &) {
            return std::weak_ptr<T>();
        }
    }
    return std::weak_ptr<T>();
}

template <typename T>
void GlobalSharedPtrManager::addSharedPtr(const std::string &key,
                                          std::shared_ptr<T> sharedPtr) {
    std::unique_lock lock(mutex_);
    shared_ptr_map_[key] = std::move(sharedPtr);
}

template <typename T>
void GlobalSharedPtrManager::addWeakPtr(const std::string &key,
                                        const std::weak_ptr<T> &weakPtr) {
    std::unique_lock lock(mutex_);
    shared_ptr_map_[key] = weakPtr;
}

template <typename T>
auto GlobalSharedPtrManager::getSharedPtrFromWeakPtr(const std::string &key)
    -> std::shared_ptr<T> {
    std::shared_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            return std::any_cast<std::weak_ptr<T>>(iter->second).lock();
        } catch (const std::bad_any_cast &) {
            return std::shared_ptr<T>();
        }
    }
    return std::shared_ptr<T>();
}

template <typename T>
auto GlobalSharedPtrManager::getWeakPtrFromSharedPtr(const std::string &key)
    -> std::weak_ptr<T> {
    std::shared_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            return std::weak_ptr(
                std::any_cast<std::shared_ptr<T>>(iter->second));
        } catch (const std::bad_any_cast &) {
            return std::weak_ptr<T>();
        }
    }
    return std::weak_ptr<T>();
}

template <typename T>
void GlobalSharedPtrManager::addDeleter(
    const std::string &key, const std::function<void(T *)> &deleter) {
    std::unique_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            auto ptr = std::any_cast<std::shared_ptr<T>>(iter->second);
            ptr.reset(ptr.get(), deleter);
            shared_ptr_map_[key] = ptr;
        } catch (const std::bad_any_cast &) {
            // Ignore if the stored type does not match
        }
    }
}

template <typename T>
void GlobalSharedPtrManager::deleteObject(const std::string &key, T *ptr) {
    std::unique_lock lock(mutex_);
    auto iter = shared_ptr_map_.find(key);
    if (iter != shared_ptr_map_.end()) {
        try {
            auto deleter =
                std::any_cast<std::function<void(T *)>>(iter->second);
            deleter(ptr);
        } catch (const std::bad_any_cast &) {
            delete ptr;  // Use default delete if no custom deleter is found
        }
        shared_ptr_map_.erase(iter);
    }
}

#endif  // ATOM_META_GLOBAL_PTR_HPP
