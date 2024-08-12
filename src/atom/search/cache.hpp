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
#include <list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace atom::search {
template <typename T>
class ResourceCache {
    static_assert(std::is_copy_constructible_v<T>,
                  "T must be copy constructible");
    static_assert(std::is_copy_assignable_v<T>, "T must be copy assignable");

public:
    explicit ResourceCache(int maxSize);

    ~ResourceCache();

    void insert(const std::string &key, const T &value,
                std::chrono::seconds expirationTime);

    bool contains(const std::string &key) const;

    std::optional<T> get(const std::string &key);

    void remove(const std::string &key);

    std::future<std::optional<T>> asyncGet(const std::string &key);

    std::future<void> asyncInsert(const std::string &key, const T &value,
                                  std::chrono::seconds expirationTime);

    void clear();

    size_t size() const;

    bool empty() const;

    void evictOldest();

    bool isExpired(const std::string &key) const;

    std::future<void> asyncLoad(const std::string &key,
                                std::function<T()> loadDataFunction);

    void setMaxSize(int maxSize);

    void setExpirationTime(const std::string &key,
                           std::chrono::seconds expirationTime);

    void readFromFile(
        const std::string &filePath,
        const std::function<T(const std::string &)> &deserializer);

    void writeToFile(const std::string &filePath,
                     const std::function<std::string(const T &)> &serializer);

    void removeExpired();

    void readFromJsonFile(const std::string &filePath,
                          const std::function<T(const json &)> &fromJson);

    void writeToJsonFile(const std::string &filePath,
                         const std::function<json(const T &)> &toJson);

private:
    void evict();

    std::unordered_map<std::string,
                       std::pair<T, std::chrono::steady_clock::time_point>>
        cache;
    int maxSize;
    std::unordered_map<std::string, std::chrono::seconds> expirationTimes;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point>
        lastAccessTimes;
    std::list<std::string> lruList;
    mutable std::shared_mutex cacheMutex;
    std::thread cleanupThread;
    std::atomic<bool> stopCleanupThread{false};
    void cleanupExpiredEntries();
};
}  // namespace atom::search

#include "cache.tpp"

#endif
