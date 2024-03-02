/*
 * cache.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-6

Description: ResourceCache class for Atom Search

**************************************************/

#ifndef ATOM_SEARCH_CACHE_HPP
#define ATOM_SEARCH_CACHE_HPP

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <thread>

#include "atom/type/json.hpp"
using json = nlohmann::json;

/**
 * @brief A cache for storing and managing resources of type T.
 *
 * The ResourceCache class provides functionalities to insert, retrieve, remove,
 * and manage resources with expiration times. It supports asynchronous
 * operations for getting and inserting resources.
 *
 * @tparam T The type of resource to be stored in the cache.
 */
template <typename T>
class ResourceCache {
    static_assert(std::is_copy_constructible_v<T>,
                  "T must be copy constructible");
    static_assert(std::is_copy_assignable_v<T>, "T must be copy assignable");

public:
    /**
     * @brief Constructs a ResourceCache with a maximum size.
     *
     * @param maxSize The maximum number of elements the cache can hold.
     */
    ResourceCache(int maxSize);

    /**
     * @brief Inserts a resource into the cache with an expiration time.
     *
     * @param key The key associated with the resource.
     * @param value The value of the resource to insert.
     * @param expirationTime The expiration time of the resource.
     */
    void insert(const std::string &key, const T &value,
                std::chrono::seconds expirationTime);

    /**
     * @brief Checks if the cache contains a resource with the given key.
     *
     * @param key The key to check for.
     * @return true if the key exists in the cache, false otherwise.
     */
    bool contains(const std::string &key) const;

    /**
     * @brief Retrieves a resource from the cache by key.
     *
     * @param key The key of the resource to retrieve.
     * @return A const reference to the retrieved resource.
     */
    const T &get(const std::string &key);

    /**
     * @brief Removes a resource from the cache by key.
     *
     * @param key The key of the resource to remove.
     */
    void remove(const std::string &key);

    /**
     * @brief Retrieves a resource from the cache by key asynchronously.
     *
     * @param key The key of the resource to retrieve.
     * @return A future to the retrieved resource.
     */
    std::future<T> asyncGet(const std::string &key);

    /**
     * @brief Inserts a resource into the cache with an expiration time
     * asynchronously.
     *
     * @param key The key associated with the resource.
     * @param value The value of the resource to insert.
     * @param expirationTime The expiration time of the resource.
     * @return A future to the inserted resource.
     */
    std::future<void> asyncInsert(const std::string &key, const T &value,
                                  const std::chrono::seconds &expirationTime);

    /**
     * @brief Clears the cache.
     */
    void clear();

    /**
     * @brief Returns the number of elements in the cache.
     *
     * @return The number of elements in the cache.
     */
    size_t size() const;

    /**
     * @brief Checks if the cache is empty.
     *
     * @return true if the cache is empty, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Evicts the oldest resource from the cache.
     */
    void evictOldest();

    /**
     * @brief Checks if a resource with the given key has expired.
     *
     * @param key The key of the resource to check.
     * @return true if the resource has expired, false otherwise.
     */
    bool isExpired(const std::string &key) const;

    /**
     * @brief Loads a resource asynchronously.
     *
     * @param key The key associated with the resource.
     * @param loadDataFunction The function to load the resource.
     * @return A future to the loaded resource.
     */
    std::future<void> asyncLoad(const std::string &key,
                                std::function<T()> loadDataFunction);

    /**
     * @brief Sets the maximum size of the cache.
     *
     * @param maxSize The maximum size of the cache.
     */
    void setMaxSize(int maxSize);

    /**
     * @brief Sets the expiration time of a resource.
     *
     * @param key The key of the resource.
     * @param expirationTime The expiration time of the resource.
     */
    void setExpirationTime(const std::string &key,
                           std::chrono::seconds expirationTime);

    /**
     * @brief Reads a resource from a file asynchronously.
     *
     * @param filePath The path to the file.
     * @param loadDataFunction The function to load the resource.
     * @return A future to the loaded resource.
     */
    void readFromFile(
        const std::string &filePath,
        const std::function<T(const std::string &)> &deserializer);

    /**
     * @brief Writes a resource to a file asynchronously.
     *
     * @param filePath The path to the file.
     * @param serializer The function to serialize the resource.
     * @return A future to the written resource.
     */
    void writeToFile(const std::string &filePath,
                     const std::function<std::string(const T &)> &serializer);

    /**
     * @brief Removes expired resources from the cache.
     */
    void removeExpired();

    /**
     * @brief Reads a resource from a JSON file asynchronously.
     *
     * @param filePath The path to the file.
     * @param loadDataFunction The function to load the resource.
     * @return A future to the loaded resource.
     */
    void readFromJsonFile(const std::string &filePath,
                          const std::function<T(const json &)> &fromJson);

    /**
     * @brief Writes a resource to a JSON file asynchronously.
     *
     * @param filePath The path to the file.
     * @param serializer The function to serialize the resource.
     * @return A future to the written resource.
     */
    void writeToJsonFile(const std::string &filePath,
                         const std::function<json(const T &)> &toJson);

private:
    std::unordered_map<std::string,
                       std::pair<T, std::chrono::steady_clock::time_point>>
        cache;
    int maxSize;
    std::unordered_map<std::string, std::chrono::seconds> expirationTimes;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point>
        lastAccessTimes;
    mutable std::recursive_mutex cacheMutex;

    /**
     * @brief Evicts the oldest resource from the cache.
     */
    void evict();
};

#include "cache_impl.hpp"

#endif
