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
#include <atomic>
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
#include <vector>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace atom::search {

template <typename T>
concept Cacheable = std::copy_constructible<T> && std::is_copy_assignable_v<T>;

template <Cacheable T>
class ResourceCache {
public:
    explicit ResourceCache(int maxSize) : maxSize_(maxSize) {
        cleanupThread_ =
            std::jthread(&ResourceCache::cleanupExpiredEntries, this);
    }

    ~ResourceCache() {
        stopCleanupThread_.store(true);
        if (cleanupThread_.joinable()) {
            cleanupThread_.join();
        }
    }

    void insert(const std::string &key, const T &value,
                std::chrono::seconds expirationTime);

    auto contains(const std::string &key) const -> bool;

    auto get(const std::string &key) -> std::optional<T>;

    void remove(const std::string &key);

    auto asyncGet(const std::string &key) -> std::future<std::optional<T>>;

    auto asyncInsert(const std::string &key, const T &value,
                     std::chrono::seconds expirationTime) -> std::future<void>;

    void clear();

    auto size() const -> size_t;

    auto empty() const -> bool;

    void evictOldest();

    auto isExpired(const std::string &key) const -> bool;

    auto asyncLoad(const std::string &key,
                   std::function<T()> loadDataFunction) -> std::future<void>;

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

    // New methods for bulk operations
    void insertBatch(const std::vector<std::pair<std::string, T>> &items,
                     std::chrono::seconds expirationTime);

    void removeBatch(const std::vector<std::string> &keys);

private:
    void evict();

    std::unordered_map<std::string,
                       std::pair<T, std::chrono::steady_clock::time_point>>
        cache_;
    int maxSize_;
    std::unordered_map<std::string, std::chrono::seconds> expirationTimes_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point>
        lastAccessTimes_;
    std::list<std::string> lruList_;
    mutable std::shared_mutex cacheMutex_;
    std::jthread cleanupThread_;
    std::atomic<bool> stopCleanupThread_{false};
    void cleanupExpiredEntries();

    // Adaptive cleanup interval based on expired entry density
    std::chrono::seconds cleanupInterval_{1};
};

template <Cacheable T>
void ResourceCache<T>::insert(const std::string &key, const T &value,
                              std::chrono::seconds expirationTime) {
    std::unique_lock lock(cacheMutex_);
    if (cache_.size() >= maxSize_) {
        evictOldest();
    }
    cache_[key] = {value, std::chrono::steady_clock::now()};
    expirationTimes_[key] = expirationTime;
    lastAccessTimes_[key] = std::chrono::steady_clock::now();
    lruList_.push_front(key);
}

template <Cacheable T>
auto ResourceCache<T>::contains(const std::string &key) const -> bool {
    std::shared_lock lock(cacheMutex_);
    return cache_.contains(key);
}

template <Cacheable T>
auto ResourceCache<T>::get(const std::string &key) -> std::optional<T> {
    DLOG_F(INFO, "Get key: {}", key);
    std::shared_lock lock(cacheMutex_);
    if (!contains(key)) {
        return std::nullopt;
    }
    if (isExpired(key)) {
        lock.unlock();
        remove(key);
        return std::nullopt;
    }
    lock.unlock();

    std::unique_lock uniqueLock(cacheMutex_);
    lastAccessTimes_[key] = std::chrono::steady_clock::now();
    lruList_.remove(key);
    lruList_.push_front(key);
    return cache_[key].first;
}

template <Cacheable T>
void ResourceCache<T>::remove(const std::string &key) {
    std::unique_lock lock(cacheMutex_);
    cache_.erase(key);
    expirationTimes_.erase(key);
    lastAccessTimes_.erase(key);
    lruList_.remove(key);
}

template <Cacheable T>
auto ResourceCache<T>::asyncGet(const std::string &key)
    -> std::future<std::optional<T>> {
    return std::async(std::launch::async,
                      [this, key]() -> std::optional<T> { return get(key); });
}

template <Cacheable T>
auto ResourceCache<T>::asyncInsert(const std::string &key, const T &value,
                                   std::chrono::seconds expirationTime)
    -> std::future<void> {
    return std::async(std::launch::async, [this, key, value, expirationTime]() {
        insert(key, value, expirationTime);
    });
}

template <Cacheable T>
void ResourceCache<T>::clear() {
    std::unique_lock lock(cacheMutex_);
    cache_.clear();
    expirationTimes_.clear();
    lastAccessTimes_.clear();
    lruList_.clear();
}

template <Cacheable T>
auto ResourceCache<T>::size() const -> size_t {
    std::shared_lock lock(cacheMutex_);
    return cache_.size();
}

template <Cacheable T>
auto ResourceCache<T>::empty() const -> bool {
    std::shared_lock lock(cacheMutex_);
    return cache_.empty();
}

template <Cacheable T>
void ResourceCache<T>::evict() {
    if (lruList_.empty()) {
        return;
    }
    auto keyToEvict = lruList_.back();
    remove(keyToEvict);
    LOG_F(INFO, "Evicted key: {}", keyToEvict);
}

template <Cacheable T>
void ResourceCache<T>::evictOldest() {
    evict();
}

template <Cacheable T>
auto ResourceCache<T>::isExpired(const std::string &key) const -> bool {
    auto it = expirationTimes_.find(key);
    if (it == expirationTimes_.end()) {
        return false;
    }

    return (std::chrono::steady_clock::now() - cache_.at(key).second) >=
           it->second;
}

template <Cacheable T>
auto ResourceCache<T>::asyncLoad(const std::string &key,
                                 std::function<T()> loadDataFunction)
    -> std::future<void> {
    return std::async(std::launch::async, [this, key, loadDataFunction]() {
        try {
            T value = loadDataFunction();
            insert(key, value, std::chrono::seconds(60));
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Async load failed: {}", e.what());
        }
    });
}

template <Cacheable T>
void ResourceCache<T>::setMaxSize(int maxSize) {
    std::unique_lock lock(cacheMutex_);
    this->maxSize_ = maxSize;
}

template <Cacheable T>
void ResourceCache<T>::setExpirationTime(const std::string &key,
                                         std::chrono::seconds expirationTime) {
    std::unique_lock lock(cacheMutex_);
    if (contains(key)) {
        expirationTimes_[key] = expirationTime;
    }
}

template <Cacheable T>
void ResourceCache<T>::readFromFile(
    const std::string &filePath,
    const std::function<T(const std::string &)> &deserializer) {
    std::ifstream inputFile(filePath);
    if (inputFile.is_open()) {
        std::unique_lock lock(cacheMutex_);
        std::string line;
        while (std::getline(inputFile, line)) {
            auto separatorIndex = line.find(':');
            if (separatorIndex != std::string::npos) {
                std::string key = line.substr(0, separatorIndex);
                std::string valueString = line.substr(separatorIndex + 1);
                T value = deserializer(valueString);
                cache_[key] = {value, std::chrono::steady_clock::now()};
                lastAccessTimes_[key] = std::chrono::steady_clock::now();
                lruList_.push_front(key);
            }
        }
        inputFile.close();
    }
}

template <Cacheable T>
void ResourceCache<T>::writeToFile(
    const std::string &filePath,
    const std::function<std::string(const T &)> &serializer) {
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        std::shared_lock lock(cacheMutex_);
        for (const auto &pair : cache_) {
            std::string line =
                pair.first + ":" + serializer(pair.second.first) + "\n";
            outputFile << line;
        }
        outputFile.close();
    }
}

template <Cacheable T>
void ResourceCache<T>::removeExpired() {
    std::unique_lock lock(cacheMutex_);
    auto it = cache_.begin();
    while (it != cache_.end()) {
        if (isExpired(it->first)) {
            lruList_.remove(it->first);
            expirationTimes_.erase(it->first);
            lastAccessTimes_.erase(it->first);
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }
}

template <Cacheable T>
void ResourceCache<T>::readFromJsonFile(
    const std::string &filePath,
    const std::function<T(const json &)> &fromJson) {
    std::ifstream inputFile(filePath);
    if (inputFile.is_open()) {
        std::unique_lock lock(cacheMutex_);
        json jsonData;
        inputFile >> jsonData;
        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            T value = fromJson(it.value());
            cache_[it.key()] = {value, std::chrono::steady_clock::now()};
            lastAccessTimes_[it.key()] = std::chrono::steady_clock::now();
            lruList_.push_front(it.key());
        }
        inputFile.close();
    }
}

template <Cacheable T>
void ResourceCache<T>::writeToJsonFile(
    const std::string &filePath, const std::function<json(const T &)> &toJson) {
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        std::shared_lock lock(cacheMutex_);
        json jsonData;
        for (const auto &pair : cache_) {
            jsonData[pair.first] = toJson(pair.second.first);
        }
        outputFile << jsonData.dump(
            4);  // Pretty-print JSON with 4-space indentation
        outputFile.close();
    }
}

template <Cacheable T>
void ResourceCache<T>::cleanupExpiredEntries() {
    while (!stopCleanupThread_.load()) {
        std::this_thread::sleep_for(cleanupInterval_);
        removeExpired();

        // Adjust cleanup interval adaptively based on cache size and expired
        // entry density
        std::unique_lock lock(cacheMutex_);
        if (cache_.size() > 0) {
            auto expiredCount = std::count_if(
                cache_.begin(), cache_.end(),
                [this](const auto &entry) { return isExpired(entry.first); });
            auto density = static_cast<double>(expiredCount) / cache_.size();
            if (density >
                0.3) {  // If more than 30% are expired, increase frequency
                cleanupInterval_ = std::chrono::seconds(1);
            } else if (density <
                       0.1) {  // If less than 10% are expired, reduce frequency
                cleanupInterval_ = std::chrono::seconds(5);
            } else {
                cleanupInterval_ = std::chrono::seconds(3);
            }
        } else {
            cleanupInterval_ =
                std::chrono::seconds(5);  // Default when cache is empty
        }
    }
}

// New method: Batch insert
template <Cacheable T>
void ResourceCache<T>::insertBatch(
    const std::vector<std::pair<std::string, T>> &items,
    std::chrono::seconds expirationTime) {
    std::unique_lock lock(cacheMutex_);
    for (const auto &[key, value] : items) {
        if (cache_.size() >= maxSize_) {
            evict();
        }
        cache_[key] = {value, std::chrono::steady_clock::now()};
        expirationTimes_[key] = expirationTime;
        lastAccessTimes_[key] = std::chrono::steady_clock::now();
        lruList_.push_front(key);
    }
}

// New method: Batch remove
template <Cacheable T>
void ResourceCache<T>::removeBatch(const std::vector<std::string> &keys) {
    std::unique_lock lock(cacheMutex_);
    for (const auto &key : keys) {
        cache_.erase(key);
        expirationTimes_.erase(key);
        lastAccessTimes_.erase(key);
        lruList_.remove(key);
    }
}

}  // namespace atom::search

#endif  // ATOM_SEARCH_CACHE_TPP
