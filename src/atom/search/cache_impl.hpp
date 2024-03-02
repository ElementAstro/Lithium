/*
 * cache_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-6

Description: ResourceCache class implementation

**************************************************/

#ifndef ATOM_SEARCH_CACHE_IMPL_HPP
#define ATOM_SEARCH_CACHE_IMPL_HPP

#include "atom/log/loguru.hpp"

template <typename T>
void ResourceCache<T>::evict() {
    std::string keyToEvict;
    std::chrono::steady_clock::time_point oldestAccessTime =
        std::chrono::steady_clock::now();

    // 查找最久未访问且未过期的条目
    for (const auto &entry : lastAccessTimes) {
        if (std::chrono::steady_clock::now() - entry.second >
                expirationTimes[entry.first] &&
            entry.second < oldestAccessTime) {
            keyToEvict = entry.first;
            oldestAccessTime = entry.second;
        }
    }

    if (!keyToEvict.empty()) {
        cache.erase(keyToEvict);
        expirationTimes.erase(keyToEvict);
        lastAccessTimes.erase(keyToEvict);
        LOG_F(ERROR, "Evicted key: {}", keyToEvict);
    }
}

template <typename T>
ResourceCache<T>::ResourceCache(int maxSize) : maxSize(maxSize) {}

template <typename T>
void ResourceCache<T>::insert(const std::string &key, const T &value,
                              std::chrono::seconds expirationTime) {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    if (cache.size() >= maxSize) {
        evictOldest();
    }
    cache[key] = {value, std::chrono::steady_clock::now()};
    expirationTimes[key] = expirationTime;
}

template <typename T>
bool ResourceCache<T>::contains(const std::string &key) const {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    return cache.find(key) != cache.end();
}

template <typename T>
const T &ResourceCache<T>::get(const std::string &key) {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    DLOG_F(INFO, "Get key: {}", key);
    if (!contains(key)) {
        throw std::out_of_range("Key not found in cache");
    }
    if (isExpired(key)) {
        cache.erase(key);
        throw std::out_of_range("Key expired");
    }
    lastAccessTimes[key] = std::chrono::steady_clock::now();
    return cache[key].first;
}

template <typename T>
void ResourceCache<T>::remove(const std::string &key) {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    cache.erase(key);
    expirationTimes.erase(key);
}

template <typename T>
std::future<T> ResourceCache<T>::asyncGet(const std::string &key) {
    return std::async(std::launch::async, [this, key]() {
        std::lock_guard<std::recursive_mutex> lock(cacheMutex);
        if (cache.find(key) != cache.end()) {
            lastAccessTimes[key] = std::chrono::steady_clock::now();
            return cache[key].first;
        }
        throw std::out_of_range("Key not found in cache");
    });
}

template <typename T>
std::future<void> ResourceCache<T>::asyncInsert(
    const std::string &key, const T &value,
    const std::chrono::seconds &expirationTime) {
    return std::async(std::launch::async, [this, key, value, expirationTime]() {
        std::lock_guard<std::recursive_mutex> lock(cacheMutex);
        if (cache.size() >= maxSize) {
            evict();
        }
        cache[key] = {value, std::chrono::steady_clock::now() + expirationTime};
        expirationTimes[key] = expirationTime;
        lastAccessTimes[key] = std::chrono::steady_clock::now();
    });
}

template <typename T>
void ResourceCache<T>::clear() {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    cache.clear();
    expirationTimes.clear();
    lastAccessTimes.clear();
}

template <typename T>
size_t ResourceCache<T>::size() const {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    return cache.size();
}

template <typename T>
bool ResourceCache<T>::empty() const {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    return cache.empty();
}

template <typename T>
void ResourceCache<T>::evictOldest() {
    auto oldest = std::min_element(
        lastAccessTimes.begin(), lastAccessTimes.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });

    if (oldest != lastAccessTimes.end()) {
        cache.erase(oldest->first);
        expirationTimes.erase(oldest->first);
        lastAccessTimes.erase(oldest);
    }
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
            std::lock_guard<std::recursive_mutex> lock(cacheMutex);
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
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    this->maxSize = maxSize;
}

template <typename T>
void ResourceCache<T>::setExpirationTime(const std::string &key,
                                         std::chrono::seconds expirationTime) {
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
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
        std::lock_guard<std::recursive_mutex> lock(cacheMutex);
        std::string line;
        while (std::getline(inputFile, line)) {
            std::size_t separatorIndex = line.find(':');
            if (separatorIndex != std::string::npos &&
                separatorIndex != line.length() - 1) {
                std::string key = line.substr(0, separatorIndex);
                std::string valueString = line.substr(separatorIndex + 1);
                T value = deserializer(valueString);
                cache[key] = {value, std::chrono::steady_clock::now()};
                lastAccessTimes[key] = std::chrono::steady_clock::now();
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
        std::lock_guard<std::recursive_mutex> lock(cacheMutex);
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
    std::lock_guard<std::recursive_mutex> lock(cacheMutex);
    auto it = cache.begin();
    while (it != cache.end()) {
        if (isExpired(it->first)) {
            it = cache.erase(it);
            expirationTimes.erase(it->first);
            lastAccessTimes.erase(it->first);
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
        std::lock_guard<std::recursive_mutex> lock(cacheMutex);
        json jsonData;
        inputFile >> jsonData;
        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            T value = fromJson(it.value());
            cache[it.key()] = {value, std::chrono::steady_clock::now()};
            lastAccessTimes[it.key()] = std::chrono::steady_clock::now();
        }
        inputFile.close();
    }
}

template <typename T>
void ResourceCache<T>::writeToJsonFile(
    const std::string &filePath, const std::function<json(const T &)> &toJson) {
    std::ofstream outputFile(filePath);
    if (outputFile.is_open()) {
        std::lock_guard<std::recursive_mutex> lock(cacheMutex);
        json jsonData;
        for (const auto &pair : cache) {
            jsonData[pair.first] = toJson(pair.second.first);
        }
        outputFile << jsonData.dump(4);
        outputFile.close();
    }
}

#endif
