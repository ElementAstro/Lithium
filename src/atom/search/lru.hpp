#ifndef THREADSAFE_LRU_CACHE_H
#define THREADSAFE_LRU_CACHE_H

#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace atom::search {
/**
 * @brief A thread-safe LRU (Least Recently Used) cache implementation.
 *
 * This class implements an LRU cache with thread safety using a combination
 * of a doubly-linked list and an unordered map. It supports adding, retrieving,
 * and removing cache items, as well as persisting cache contents to and loading
 * from a file.
 *
 * @tparam Key Type of the cache keys.
 * @tparam Value Type of the cache values.
 */
template <typename Key, typename Value>
class ThreadSafeLRUCache {
public:
    using KeyValuePair =
        std::pair<Key, Value>;  ///< Type alias for a key-value pair.
    using ListIterator =
        typename std::list<KeyValuePair>::iterator;  ///< Iterator type for the
                                                     ///< list.
    using Clock =
        std::chrono::steady_clock;  ///< Clock type for timing operations.
    using TimePoint =
        std::chrono::time_point<Clock>;  ///< Time point type for expiry times.

    struct CacheItem {
        Value value;
        TimePoint expiryTime;
        ListIterator iterator;
    };

    /**
     * @brief Constructs a ThreadSafeLRUCache with a specified maximum size.
     *
     * @param max_size The maximum number of items that the cache can hold.
     */
    explicit ThreadSafeLRUCache(size_t max_size);

    /**
     * @brief Retrieves a value from the cache.
     *
     * Moves the accessed item to the front of the cache, indicating it was
     * recently used.
     *
     * @param key The key of the item to retrieve.
     * @return An optional containing the value if found and not expired,
     * otherwise std::nullopt.
     */
    auto get(const Key& key) -> std::optional<Value>;

    /**
     * @brief Inserts or updates a value in the cache.
     *
     * If the cache is full, the least recently used item is removed.
     *
     * @param key The key of the item to insert or update.
     * @param value The value to associate with the key.
     * @param ttl Optional time-to-live (TTL) duration for the cache item.
     */
    void put(const Key& key, const Value& value,
             std::optional<std::chrono::seconds> ttl = std::nullopt);

    /**
     * @brief Erases an item from the cache.
     *
     * @param key The key of the item to remove.
     */
    void erase(const Key& key);

    /**
     * @brief Clears all items from the cache.
     */
    void clear();

    /**
     * @brief Retrieves all keys in the cache.
     *
     * @return A vector containing all keys currently in the cache.
     */
    auto keys() const -> std::vector<Key>;

    /**
     * @brief Removes and returns the least recently used item.
     *
     * @return An optional containing the key-value pair if the cache is not
     * empty, otherwise std::nullopt.
     */
    auto popLru() -> std::optional<KeyValuePair>;

    /**
     * @brief Resizes the cache to a new maximum size.
     *
     * If the new size is smaller, the least recently used items are removed
     * until the cache size fits.
     *
     * @param new_max_size The new maximum size of the cache.
     */
    void resize(size_t new_max_size);

    /**
     * @brief Gets the current size of the cache.
     *
     * @return The number of items currently in the cache.
     */
    auto size() const -> size_t;

    /**
     * @brief Gets the current load factor of the cache.
     *
     * The load factor is the ratio of the current size to the maximum size.
     *
     * @return The load factor of the cache.
     */
    auto loadFactor() const -> float;

    /**
     * @brief Sets the callback function to be called when a new item is
     * inserted.
     *
     * @param callback The callback function that takes a key and value.
     */
    void setInsertCallback(
        std::function<void(const Key&, const Value&)> callback);

    /**
     * @brief Sets the callback function to be called when an item is erased.
     *
     * @param callback The callback function that takes a key.
     */
    void setEraseCallback(std::function<void(const Key&)> callback);

    /**
     * @brief Sets the callback function to be called when the cache is cleared.
     *
     * @param callback The callback function.
     */
    void setClearCallback(std::function<void()> callback);

    /**
     * @brief Gets the hit rate of the cache.
     *
     * The hit rate is the ratio of cache hits to the total number of cache
     * accesses.
     *
     * @return The hit rate of the cache.
     */
    auto hitRate() const -> float;

    /**
     * @brief Saves the cache contents to a file.
     *
     * @param filename The name of the file to save to.
     * @throws std::runtime_error If a deadlock is avoided while locking.
     */
    void saveToFile(const std::string& filename) const;

    /**
     * @brief Loads cache contents from a file.
     *
     * @param filename The name of the file to load from.
     * @throws std::runtime_error If a deadlock is avoided while locking.
     */
    void loadFromFile(const std::string& filename);

private:
    mutable std::shared_mutex mutex_;  ///< Mutex for protecting shared data.
    std::list<KeyValuePair>
        cache_items_list_;  ///< List for maintaining item order.
    std::unordered_map<Key, CacheItem>
        cache_items_map_;  ///< Map for fast key lookups.
    size_t max_size_;      ///< Maximum number of items in the cache.
    size_t hit_count_{};   ///< Number of cache hits.
    size_t miss_count_{};  ///< Number of cache misses.

    std::function<void(const Key&, const Value&)>
        on_insert_;  ///< Callback for item insertion.
    std::function<void(const Key&)> on_erase_;  ///< Callback for item removal.
    std::function<void()> on_clear_;  ///< Callback for cache clearing.

    /**
     * @brief Checks if a cache item has expired.
     *
     * @param item The cache item to check.
     * @return True if the item is expired, false otherwise.
     */
    auto isExpired(const CacheItem& item) const -> bool;
};

template <typename Key, typename Value>
ThreadSafeLRUCache<Key, Value>::ThreadSafeLRUCache(size_t max_size)
    : max_size_(max_size) {}

template <typename Key, typename Value>
auto ThreadSafeLRUCache<Key, Value>::get(const Key& key)
    -> std::optional<Value> {
    std::unique_lock lock(mutex_, std::try_to_lock);
    if (!lock) {
        return std::nullopt;  // Avoid deadlock
    }

    auto it = cache_items_map_.find(key);
    if (it == cache_items_map_.end() || isExpired(it->second)) {
        ++miss_count_;
        if (it != cache_items_map_.end()) {
            erase(key);  // Remove expired item
        }
        return std::nullopt;
    }
    ++hit_count_;
    cache_items_list_.splice(cache_items_list_.begin(), cache_items_list_,
                             it->second.iterator);
    return it->second.value;
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::put(
    const Key& key, const Value& value,
    std::optional<std::chrono::seconds> ttl) {
    std::unique_lock lock(mutex_);
    auto it = cache_items_map_.find(key);
    auto expiryTime = ttl ? Clock::now() + *ttl : TimePoint::max();

    if (it != cache_items_map_.end()) {
        cache_items_list_.splice(cache_items_list_.begin(), cache_items_list_,
                                 it->second.iterator);
        it->second.value = value;
        it->second.expiryTime = expiryTime;
    } else {
        cache_items_list_.emplace_front(key, value);
        cache_items_map_[key] = {value, expiryTime, cache_items_list_.begin()};

        if (cache_items_map_.size() > max_size_) {
            auto last = cache_items_list_.end();
            --last;
            cache_items_map_.erase(last->first);
            cache_items_list_.pop_back();
        }
    }
    if (on_insert_) {
        on_insert_(key, value);
    }
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::erase(const Key& key) {
    std::unique_lock lock(mutex_);
    auto it = cache_items_map_.find(key);
    if (it != cache_items_map_.end()) {
        cache_items_list_.erase(it->second.iterator);
        cache_items_map_.erase(it);
        if (on_erase_) {
            on_erase_(key);
        }
    }
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::clear() {
    std::unique_lock lock(mutex_);
    cache_items_list_.clear();
    cache_items_map_.clear();
    if (on_clear_) {
        on_clear_();
    }
}

template <typename Key, typename Value>
auto ThreadSafeLRUCache<Key, Value>::keys() const -> std::vector<Key> {
    std::shared_lock lock(mutex_);
    std::vector<Key> keys;
    for (const auto& pair : cache_items_list_) {
        keys.push_back(pair.first);
    }
    return keys;
}

template <typename Key, typename Value>
auto ThreadSafeLRUCache<Key, Value>::popLru()
    -> std::optional<typename ThreadSafeLRUCache<Key, Value>::KeyValuePair> {
    std::unique_lock lock(mutex_);
    if (cache_items_list_.empty()) {
        return std::nullopt;
    }
    auto last = cache_items_list_.end();
    --last;
    KeyValuePair kv = *last;
    cache_items_map_.erase(last->first);
    cache_items_list_.pop_back();
    return kv;
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::resize(size_t new_max_size) {
    std::unique_lock lock(mutex_);
    max_size_ = new_max_size;
    while (cache_items_map_.size() > max_size_) {
        auto last = cache_items_list_.end();
        --last;
        cache_items_map_.erase(last->first);
        cache_items_list_.pop_back();
    }
}

template <typename Key, typename Value>
size_t ThreadSafeLRUCache<Key, Value>::size() const {
    std::shared_lock lock(mutex_);
    return cache_items_map_.size();
}

template <typename Key, typename Value>
auto ThreadSafeLRUCache<Key, Value>::loadFactor() const -> float {
    std::shared_lock lock(mutex_);
    return static_cast<float>(cache_items_map_.size()) / max_size_;
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::setInsertCallback(
    std::function<void(const Key&, const Value&)> callback) {
    on_insert_ = std::move(callback);
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::setEraseCallback(
    std::function<void(const Key&)> callback) {
    on_erase_ = std::move(callback);
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::setClearCallback(
    std::function<void()> callback) {
    on_clear_ = std::move(callback);
}

template <typename Key, typename Value>
auto ThreadSafeLRUCache<Key, Value>::hitRate() const -> float {
    std::shared_lock lock(mutex_);
    size_t total = hit_count_ + miss_count_;
    return total == 0 ? 0.0F
                      : static_cast<float>(static_cast<double>(hit_count_) /
                                           static_cast<double>(total));
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::saveToFile(
    const std::string& filename) const {
    std::unique_lock lock(mutex_, std::try_to_lock);
    if (!lock) {
        throw std::runtime_error("Resource deadlock avoided");
    }

    std::ofstream ofs(filename, std::ios::binary);
    if (ofs.is_open()) {
        size_t size = cache_items_map_.size();
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (const auto& pair : cache_items_list_) {
            ofs.write(reinterpret_cast<const char*>(&pair.first),
                      sizeof(pair.first));
            size_t valueSize = pair.second.size();
            ofs.write(reinterpret_cast<const char*>(&valueSize),
                      sizeof(valueSize));
            ofs.write(pair.second.c_str(), valueSize);
        }
    }
}

template <typename Key, typename Value>
void ThreadSafeLRUCache<Key, Value>::loadFromFile(const std::string& filename) {
    std::unique_lock lock(mutex_, std::try_to_lock);
    if (!lock) {
        throw std::runtime_error("Resource deadlock avoided");
    }

    std::ifstream ifs(filename, std::ios::binary);
    if (ifs.is_open()) {
        clear();
        size_t size;
        ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
        for (size_t i = 0; i < size; ++i) {
            Key key;
            ifs.read(reinterpret_cast<char*>(&key), sizeof(key));
            size_t valueSize;
            ifs.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));
            std::string value(valueSize, '\0');
            ifs.read(value.data(), static_cast<std::streamsize>(valueSize));
            put(key, value);
        }
    }
}

template <typename Key, typename Value>
auto ThreadSafeLRUCache<Key, Value>::isExpired(const CacheItem& item) const -> bool {
    return Clock::now() > item.expiryTime;
}

}  // namespace atom::search

#endif  // THREADSAFE_LRU_CACHE_H
