/*
 * cache_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-6

Description: ResourceCache class implementation

**************************************************/

#ifndef ATOM_SEARCH_CACHE_TPP
#define ATOM_SEARCH_CACHE_TPP

#include "cache.hpp"

#include "atom/log/loguru.hpp"

namespace atom::search {
template <typename T>
ResourceCache<T>::ResourceCache(int maxSize) : maxSize(maxSize) {
    cleanupThread = std::thread(&ResourceCache::cleanupExpiredEntries, this);
}

template <typename T>
ResourceCache<T>::~ResourceCache() {
    stopCleanupThread.store(true);
    if (cleanupThread.joinable()) {
        cleanupThread.join();
    }
}

template <typename T>
void ResourceCache<T>::insert(const std::string &key, const T &value,
                              std::chrono::seconds expirationTime) {
    std::unique_lock lock(cacheMutex);
    if (cache.size() >= maxSize) {
        evictOldest();
    }
    cache[key] = {value, std::chrono::steady_clock::now()};
    expirationTimes[key] = expirationTime;
    lastAccessTimes[key] = std::chrono::steady_clock::now();
    lruList.push_front(key);
}

template <typename T>
bool ResourceCache<T>::contains(const std::string &key) const {
    std::shared_lock lock(cacheMutex);
    return cache.find(key) != cache.end();
}

template <typename T>
std::optional<T> ResourceCache<T>::get(const std::string &key) {
    DLOG_F(INFO, "Get key: {}", key);
    if (!contains(key)) {
        return std::nullopt;
    }
    if (isExpired(key)) {
        cache.erase(key);
        expirationTimes.erase(key);
        lastAccessTimes.erase(key);
        lruList.remove(key);
        return std::nullopt;
    }
    std::unique_lock lock(cacheMutex);
    lastAccessTimes[key] = std::chrono::steady_clock::now();
    lruList.remove(key);
    lruList.push_front(key);
    return cache[key].first;
}

template <typename T>
void ResourceCache<T>::remove(const std::string &key) {
    std::unique_lock lock(cacheMutex);
    cache.erase(key);
    expirationTimes.erase(key);
    lastAccessTimes.erase(key);
    lruList.remove(key);
}

template <typename T>
std::future<std::optional<T>> ResourceCache<T>::asyncGet(
    const std::string &key) {
    return std::async(std::launch::async, [this, key]() -> std::optional<T> {
        std::shared_lock lock(cacheMutex);
        if (contains(key)) {
            lastAccessTimes[key] = std::chrono::steady_clock::now();
            return std::optional<T>(cache[key].first);
        }
        return std::nullopt;
    });
}

template <typename T>
std::future<void> ResourceCache<T>::asyncInsert(
    const std::string &key, const T &value,
    std::chrono::seconds expirationTime) {
    return std::async(std::launch::async, [this, key, value, expirationTime]() {
        std::unique_lock lock(cacheMutex);
        if (cache.size() >= maxSize) {
            evict();
        }
        cache[key] = {value, std::chrono::steady_clock::now()};
        expirationTimes[key] = expirationTime;
        lastAccessTimes[key] = std::chrono::steady_clock::now();
        lruList.push_front(key);
    });
}

template <typename T>
void ResourceCache<T>::clear() {
    std::unique_lock lock(cacheMutex);
    cache.clear();
    expirationTimes.clear();
    lastAccessTimes.clear();
    lruList.clear();
}

template <typename T>
size_t ResourceCache<T>::size() const {
    std::shared_lock lock(cacheMutex);
    return cache.size();
}

template <typename T>
bool ResourceCache<T>::empty() const {
    std::shared_lock lock(cacheMutex);
    return cache.empty();
}

template <typename T>
void ResourceCache<T>::evict() {
    if (lruList.empty())
        return;
    auto keyToEvict = lruList.back();
    cache.erase(keyToEvict);
    expirationTimes.erase(keyToEvict);
    lastAccessTimes.erase(keyToEvict);
    lruList.pop_back();

    LOG_F(INFO, "Evicted key: {}", keyToEvict);
}

template <typename T>
void ResourceCache<T>::evictOldest() {
    if (lruList.empty())
        return;
    auto keyToEvict = lruList.back();
    lruList.pop_back();
    cache.erase(keyToEvict);
    expirationTimes.erase(keyToEvict);
    lastAccessTimes.erase(keyToEvict);
    LOG_F(ERROR, "Evicted key: {}", keyToEvict);
}

template <typename T>
bool ResourceCache<T>::isExpired(const std::string &key) const {
    if (expirationTimes.find(key) == expirationTimes.end()) {
        return false;
    }
    return std::chrono::steady_clock::now() - cache.at(key).second >=
           expirationTimes.at(key);
}

template <typename T>
std::future<void> ResourceCache<T>::asyncLoad(
    const std::string &key, std::function<T()> loadDataFunction) {
    return std::async(std::launch::async, [this, key, loadDataFunction]() {
        try {
            T value = loadDataFunction();
            std::unique_lock lock(cacheMutex);
            if (!contains(key)) {
                insert(key, value, std::chrono::seconds(60));
            }
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Async load failed: {}", e.what());
        }
    });
}

template <typename T>
void ResourceCache<T>::setMaxSize(int maxSize) {
    std::unique_lock lock(cacheMutex);
    this->maxSize = maxSize;
}

template <typename T>
void ResourceCache<T>::setExpirationTime(const std::string &key,
                                         std::chrono::seconds expirationTime) {
    std::unique_lock lock(cacheMutex);
    if (contains(key)) {
        expirationTimes[key] = expirationTime;
    }
}

template <typename T>
void ResourceCache<T>::readFromFile(
    const std::string &filePath,
    const std::function<T(const std::string &)> &deserializer) {
    std::ifstream inputFile(filePath);
    if (inputFile.is_open()) {
        std::unique_lock lock(cacheMutex);
        std::string line;
        while (std::getline(inputFile, line)) {
            auto separatorIndex = line.find(':');
            if (separatorIndex != line.length() - 1) {
                std::string key = line.substr(0, separatorIndex);
                std::string valueString = line.substr(separatorIndex + 1);
                T value = deserializer(valueString);
                cache[key] = {value, std::chrono::steady_clock::now()};
                lastAccessTimes[key] = std::chrono::steady_clock::now();
                lruList.push_front(key);
            }
        }
        inputFile.close();
    }
}

template <typename T>
void ResourceCache<T>::writeToFile(
    const std::string &filePath,
    const std::function<std::string(const T &)> &serializer) {
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        std::shared_lock lock(cacheMutex);
        for (const auto &pair : cache) {
            std::string line =
                pair.first + ":" + serializer(pair.second.first) + "\n";
            outputFile << line;
        }
        outputFile.close();
    }
}

template <typename T>
void ResourceCache<T>::removeExpired() {
    std::unique_lock lock(cacheMutex);
    auto it = cache.begin();
    while (it != cache.end()) {
        if (isExpired(it->first)) {
            lruList.remove(it->first);
            expirationTimes.erase(it->first);
            lastAccessTimes.erase(it->first);
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
}

template <typename T>
void ResourceCache<T>::readFromJsonFile(
    const std::string &filePath,
    const std::function<T(const json &)> &fromJson) {
    std::ifstream inputFile(filePath);
    if (inputFile.is_open()) {
        std::unique_lock lock(cacheMutex);
        json jsonData;
        inputFile >> jsonData;
        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            T value = fromJson(it.value());
            cache[it.key()] = {value, std::chrono::steady_clock::now()};
            lastAccessTimes[it.key()] = std::chrono::steady_clock::now();
            lruList.push_front(it.key());
        }
        inputFile.close();
    }
}

template <typename T>
void ResourceCache<T>::writeToJsonFile(
    const std::string &filePath, const std::function<json(const T &)> &toJson) {
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        std::shared_lock lock(cacheMutex);
        json jsonData;
        for (const auto &pair : cache) {
            jsonData[pair.first] = toJson(pair.second.first);
        }
        outputFile << jsonData.dump();
        outputFile.close();
    }
}

template <typename T>
void ResourceCache<T>::cleanupExpiredEntries() {
    while (!stopCleanupThread.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            std::unique_lock lock(cacheMutex);
            auto it = cache.begin();
            while (it != cache.end()) {
                if (isExpired(it->first)) {
                    lruList.remove(it->first);
                    expirationTimes.erase(it->first);
                    lastAccessTimes.erase(it->first);
                    it = cache.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}
}  // namespace atom::search

#endif  // ATOM_SEARCH_CACHE_TPP