#ifndef ATOM_SEARCH_TTL_CACHE_HPP
#define ATOM_SEARCH_TTL_CACHE_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace atom::search {
/**
 * @brief A Time-to-Live (TTL) Cache with a maximum capacity.
 *
 * This class implements a TTL cache with an LRU eviction policy. Items in the
 * cache expire after a specified duration and are evicted when the cache
 * exceeds its maximum capacity.
 *
 * @tparam Key The type of the cache keys.
 * @tparam Value The type of the cache values.
 */
template <typename Key, typename Value>
class TTLCache {
public:
    using Clock = std::chrono::steady_clock;  ///< Type alias for the clock used
                                              ///< for timestamps.
    using TimePoint =
        std::chrono::time_point<Clock>;  ///< Type alias for time points.
    using Duration = std::chrono::milliseconds;  ///< Type alias for durations.

    /**
     * @brief Constructs a TTLCache object with the given TTL and maximum
     * capacity.
     *
     * @param ttl Duration after which items expire and are removed from the
     * cache.
     * @param max_capacity Maximum number of items the cache can hold.
     */
    TTLCache(Duration ttl, size_t max_capacity);

    /**
     * @brief Destroys the TTLCache object and stops the cleaner thread.
     */
    ~TTLCache();

    /**
     * @brief Inserts a new key-value pair into the cache or updates an existing
     * key.
     *
     * @param key The key to insert or update.
     * @param value The value associated with the key.
     */
    void put(const Key& key, const Value& value);

    /**
     * @brief Retrieves the value associated with the given key from the cache.
     *
     * @param key The key whose associated value is to be retrieved.
     * @return An optional containing the value if found and not expired;
     * otherwise, std::nullopt.
     */
    std::optional<Value> get(const Key& key);

    /**
     * @brief Performs cache cleanup by removing expired items.
     */
    void cleanup();

    /**
     * @brief Gets the cache hit rate.
     *
     * @return The ratio of cache hits to total accesses.
     */
    double hitRate() const;

    /**
     * @brief Gets the current number of items in the cache.
     *
     * @return The number of items in the cache.
     */
    size_t size() const;

    /**
     * @brief Clears all items from the cache and resets hit/miss counts.
     */
    void clear();

private:
    /**
     * @brief Structure representing a cache item.
     */
    struct CacheItem {
        Key key;                ///< The key of the cache item.
        Value value;            ///< The value of the cache item.
        TimePoint expiry_time;  ///< The expiration time of the cache item.

        CacheItem(const Key& k, const Value& v, const TimePoint& t);
    };

    using CacheList = std::list<CacheItem>;  ///< Type alias for the list used
                                             ///< to store cache items.
    using CacheMap = std::unordered_map<
        Key, typename CacheList::iterator>;  ///< Type alias for the map used to
                                             ///< locate cache items.

    Duration ttl_;          ///< Duration after which cache items expire.
    size_t max_capacity_;   ///< Maximum capacity of the cache.
    CacheList cache_list_;  ///< List of cache items, ordered by recency.
    CacheMap cache_map_;    ///< Map of cache keys to iterators in the list.

    mutable std::shared_mutex
        mutex_;  ///< Mutex for synchronizing access to cache data.
    std::atomic<size_t> hit_count_;   ///< Number of cache hits.
    std::atomic<size_t> miss_count_;  ///< Number of cache misses.

    std::thread
        cleaner_thread_;  ///< Background thread for cleaning up expired items.
    std::atomic<bool> stop_;  ///< Flag to signal the cleaner thread to stop.
    std::condition_variable_any
        cv_;  ///< Condition variable used to wake up the cleaner thread.

    /**
     * @brief The task run by the cleaner thread to periodically clean up
     * expired items.
     */
    void cleanerTask();
};

template <typename Key, typename Value>
TTLCache<Key, Value>::TTLCache(Duration ttl, size_t max_capacity)
    : ttl_(ttl),
      max_capacity_(max_capacity),
      hit_count_(0),
      miss_count_(0),
      stop_(false) {
    cleaner_thread_ = std::thread([this] { this->cleanerTask(); });
}

template <typename Key, typename Value>
TTLCache<Key, Value>::~TTLCache() {
    stop_ = true;
    cv_.notify_all();
    if (cleaner_thread_.joinable()) {
        cleaner_thread_.join();
    }
}

template <typename Key, typename Value>
void TTLCache<Key, Value>::put(const Key& key, const Value& value) {
    std::unique_lock lock(mutex_);

    if (cache_map_.find(key) != cache_map_.end()) {
        cache_list_.erase(cache_map_[key]);
    } else if (cache_list_.size() >= max_capacity_) {
        auto last = cache_list_.back();
        cache_map_.erase(last.key);
        cache_list_.pop_back();
    }

    cache_list_.emplace_front(key, value, Clock::now() + ttl_);
    cache_map_[key] = cache_list_.begin();
}

template <typename Key, typename Value>
std::optional<Value> TTLCache<Key, Value>::get(const Key& key) {
    std::shared_lock lock(mutex_);
    auto now = Clock::now();

    auto it = cache_map_.find(key);
    if (it != cache_map_.end() && it->second->expiry_time > now) {
        hit_count_++;
        // Move the accessed item to the front (LRU logic)
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
        return it->second->value;
    }

    miss_count_++;
    return std::nullopt;
}

template <typename Key, typename Value>
void TTLCache<Key, Value>::cleanup() {
    std::unique_lock lock(mutex_);
    auto now = Clock::now();

    while (!cache_list_.empty() && cache_list_.back().expiry_time <= now) {
        auto last = cache_list_.back();
        cache_map_.erase(last.key);
        cache_list_.pop_back();
    }
}

template <typename Key, typename Value>
double TTLCache<Key, Value>::hitRate() const {
    std::shared_lock lock(mutex_);
    auto total = hit_count_ + miss_count_;
    return total > 0 ? static_cast<double>(hit_count_) / total : 0.0;
}

template <typename Key, typename Value>
size_t TTLCache<Key, Value>::size() const {
    std::shared_lock lock(mutex_);
    return cache_map_.size();
}

template <typename Key, typename Value>
void TTLCache<Key, Value>::clear() {
    std::unique_lock lock(mutex_);
    cache_list_.clear();
    cache_map_.clear();
    hit_count_ = 0;
    miss_count_ = 0;
}

template <typename Key, typename Value>
TTLCache<Key, Value>::CacheItem::CacheItem(const Key& k, const Value& v,
                                           const TimePoint& t)
    : key(k), value(v), expiry_time(t) {}

template <typename Key, typename Value>
void TTLCache<Key, Value>::cleanerTask() {
    while (!stop_) {
        std::this_thread::sleep_for(ttl_);
        cleanup();
    }
}

}  // namespace atom::search

#endif  // ATOM_SEARCH_TTL_CACHE_HPP
