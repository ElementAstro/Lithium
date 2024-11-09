#ifndef ATOM_TYPE_COUNTING_HASH_TABLE_HPP
#define ATOM_TYPE_COUNTING_HASH_TABLE_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <concepts>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "atom/type/json.hpp"

namespace atom::type {
using json = nlohmann::json;
/**
 * @brief A thread-safe hash table that counts the number of accesses to each
 * entry.
 *
 * @tparam Key The type of the keys in the hash table.
 * @tparam Value The type of the values in the hash table.
 */
template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
class CountingHashTable {
public:
    /**
     * @brief Struct representing an entry in the hash table.
     */
    struct Entry {
        std::atomic<size_t> count{0};  ///< The access count of the entry.
        Value value;                   ///< The value stored in the entry.

        /**
         * @brief Default constructor.
         */
        Entry() = default;

        /**
         * @brief Constructs an Entry with a given value.
         *
         * @param val The value to store in the entry.
         */
        explicit Entry(Value val) : value(std::move(val)) {}

        // Disable copy constructor and copy assignment
        Entry(const Entry&) = delete;
        auto operator=(const Entry&) -> Entry& = delete;

        /**
         * @brief Move constructor.
         */
        Entry(Entry&& other) noexcept
            : count(other.count.load(std::memory_order_relaxed)),
              value(std::move(other.value)) {}

        /**
         * @brief Move assignment operator.
         */
        auto operator=(Entry&& other) noexcept -> Entry& {
            if (this != &other) {
                value = std::move(other.value);
                count.store(other.count.load(std::memory_order_relaxed),
                            std::memory_order_relaxed);
            }
            return *this;
        }
    };

    /**
     * @brief Constructs a new CountingHashTable object.
     */
    CountingHashTable(size_t num_mutexes = 16,
                      size_t initial_bucket_count = 1024);

    /**
     * @brief Destroys the CountingHashTable object.
     */
    ~CountingHashTable();

    /**
     * @brief Inserts a key-value pair into the hash table.
     *
     * @param key The key to insert.
     * @param value The value to insert.
     */
    void insert(const Key& key, const Value& value);

    /**
     * @brief Inserts multiple key-value pairs into the hash table.
     *
     * @param items A vector of key-value pairs to insert.
     */
    void insertBatch(const std::vector<std::pair<Key, Value>>& items);

    /**
     * @brief Retrieves the value associated with a given key.
     *
     * @param key The key to retrieve the value for.
     * @return An optional containing the value if found, otherwise
     * std::nullopt.
     */
    auto get(const Key& key) -> std::optional<Value>;

    /**
     * @brief Retrieves the access count for a given key.
     *
     * @param key The key to retrieve the access count for.
     * @return An optional containing the access count if key exists, otherwise
     * std::nullopt.
     */
    auto getAccessCount(const Key& key) const -> std::optional<size_t>;

    /**
     * @brief Retrieves the values associated with multiple keys.
     *
     * @param keys A vector of keys to retrieve the values for.
     * @return A vector of optionals containing the values if found, otherwise
     * std::nullopt.
     */
    auto getBatch(const std::vector<Key>& keys)
        -> std::vector<std::optional<Value>>;

    /**
     * @brief Erases the entry associated with a given key.
     *
     * @param key The key to erase.
     * @return true if the key was found and erased, false otherwise.
     */
    auto erase(const Key& key) -> bool;

    /**
     * @brief Clears all entries in the hash table.
     */
    void clear();

    /**
     * @brief Retrieves all entries in the hash table.
     *
     * @return A vector of key-entry pairs representing all entries in the hash
     * table.
     */
    auto getAllEntries() const -> std::vector<std::pair<Key, Entry>>;

    /**
     * @brief Sorts the entries in the hash table by their access count in
     * descending order.
     */
    void sortEntriesByCountDesc();

    /**
     * @brief Retrieves the top N entries with the highest access counts.
     *
     * @param N The number of top entries to retrieve.
     * @return A vector of key-entry pairs representing the top N entries.
     */
    auto getTopNEntries(size_t N) const -> std::vector<std::pair<Key, Entry>>;

    /**
     * @brief Starts automatic sorting of the hash table entries at regular
     * intervals.
     *
     * @param interval The interval at which to sort the entries.
     * @param ascending Whether to sort in ascending order (default:
     * descending).
     */
    void startAutoSorting(std::chrono::milliseconds interval,
                          bool ascending = false);

    /**
     * @brief Stops automatic sorting of the hash table entries.
     */
    void stopAutoSorting();

    /**
     * @brief Serializes the hash table to a JSON object.
     *
     * @return A JSON object representing the hash table.
     */
    json serializeToJson() const;

    /**
     * @brief Deserializes the hash table from a JSON object.
     *
     * @param j The JSON object to deserialize from.
     */
    void deserializeFromJson(const json& j);

private:
    mutable std::vector<std::mutex>
        mutexes_;  ///< Vector of mutexes for lock striping.
    std::unordered_map<Key, Entry> table_;  ///< The underlying hash table.
    std::atomic<bool> stopSorting{
        false};  ///< Flag to indicate whether to stop automatic sorting.
    std::thread sortingThread_;  ///< Thread for automatic sorting.
    size_t num_mutexes_;         ///< Number of mutexes for lock striping.

    /**
     * @brief The worker function for automatic sorting.
     *
     * @param interval The interval at which to sort the entries.
     * @param ascending Whether to sort in ascending order.
     */
    void sortingWorker(std::chrono::milliseconds interval, bool ascending);

    /**
     * @brief Gets the mutex index for a given key.
     *
     * @param key The key to get the mutex index for.
     * @return size_t The index of the mutex.
     */
    size_t getMutexIndex(const Key& key) const;
};

/////////////////////////// Implementation ///////////////////////////

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
CountingHashTable<Key, Value>::CountingHashTable(size_t num_mutexes,
                                                 size_t initial_bucket_count)
    : mutexes_(num_mutexes), num_mutexes_(num_mutexes) {
    table_.reserve(initial_bucket_count);
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
CountingHashTable<Key, Value>::~CountingHashTable() {
    stopAutoSorting();
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
size_t CountingHashTable<Key, Value>::getMutexIndex(const Key& key) const {
    return std::hash<Key>{}(key) % num_mutexes_;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::insert(const Key& key, const Value& value) {
    size_t index = getMutexIndex(key);
    std::unique_lock lock(mutexes_[index]);
    auto it = table_.find(key);
    if (it == table_.end()) {
        table_.emplace(key, Entry(value));
    } else {
        it->second.value = std::move(
            const_cast<Value&>(value));  // Assuming value can be moved
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::insertBatch(
    const std::vector<std::pair<Key, Value>>& items) {
    // Group items by mutex to minimize locking overhead
    std::unordered_map<size_t, std::vector<std::pair<Key, Value>>> grouped;
    for (const auto& [key, value] : items) {
        size_t index = getMutexIndex(key);
        grouped[index].emplace_back(key, value);
    }

    for (auto& [index, group] : grouped) {
        std::unique_lock lock(mutexes_[index]);
        for (auto& [key, value] : group) {
            auto it = table_.find(key);
            if (it == table_.end()) {
                table_.emplace(key, Entry(value));
            } else {
                it->second.value = std::move(
                    const_cast<Value&>(value));  // Assuming value can be moved
            }
        }
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::get(const Key& key)
    -> std::optional<Value> {
    size_t index = getMutexIndex(key);
    std::shared_lock lock(mutexes_[index]);
    auto it = table_.find(key);
    if (it != table_.end()) {
        it->second.count.fetch_add(1, std::memory_order_relaxed);
        return it->second.value;
    }
    return std::nullopt;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::getAccessCount(const Key& key) const
    -> std::optional<size_t> {
    size_t index = getMutexIndex(key);
    std::shared_lock lock(mutexes_[index]);
    auto it = table_.find(key);
    if (it != table_.end()) {
        return it->second.count.load(std::memory_order_relaxed);
    }
    return std::nullopt;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::getBatch(const std::vector<Key>& keys)
    -> std::vector<std::optional<Value>> {
    std::vector<std::optional<Value>> results;
    results.reserve(keys.size());

    // Group keys by mutex to minimize locking overhead
    std::unordered_map<size_t, std::vector<const Key*>> grouped;
    for (const auto& key : keys) {
        size_t index = getMutexIndex(key);
        grouped[index].emplace_back(&key);
    }

    for (auto& [index, group] : grouped) {
        std::shared_lock lock(mutexes_[index]);
        for (const auto* keyPtr : group) {
            auto it = table_.find(*keyPtr);
            if (it != table_.end()) {
                it->second.count.fetch_add(1, std::memory_order_relaxed);
                results.emplace_back(it->second.value);
            } else {
                results.emplace_back(std::nullopt);
            }
        }
    }

    return results;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::erase(const Key& key) -> bool {
    size_t index = getMutexIndex(key);
    std::unique_lock lock(mutexes_[index]);
    return table_.erase(key) > 0;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::clear() {
    for (size_t i = 0; i < num_mutexes_; ++i) {
        std::unique_lock lock(mutexes_[i]);
    }
    table_.clear();
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::getAllEntries() const
    -> std::vector<std::pair<Key, Entry>> {
    std::vector<std::pair<Key, Entry>> entries;
    // Lock all mutexes in a consistent order to avoid deadlocks
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].lock();
    }
    for (const auto& [key, entry] : table_) {
        entries.emplace_back(key, entry);
    }
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].unlock();
    }
    return entries;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::sortEntriesByCountDesc() {
    std::unique_lock<std::mutex> global_lock(
        mutexes_[0]);  // Simple approach: lock first mutex
    std::vector<std::pair<Key, Entry>> entries(table_.begin(), table_.end());
    global_lock.unlock();

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.second.count.load(std::memory_order_relaxed) >
               b.second.count.load(std::memory_order_relaxed);
    });

    // Rebuild the table
    for (auto& [key, entry] : entries) {
        size_t index = getMutexIndex(key);
        std::unique_lock lock(mutexes_[index]);
        table_[key] = std::move(entry);
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::getTopNEntries(size_t N) const
    -> std::vector<std::pair<Key, Entry>> {
    std::vector<std::pair<Key, Entry>> entries;
    // Lock all mutexes in a consistent order to avoid deadlocks
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].lock();
    }
    entries.reserve(table_.size());
    for (const auto& [key, entry] : table_) {
        entries.emplace_back(key, entry);
    }
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].unlock();
    }

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.second.count.load(std::memory_order_relaxed) >
               b.second.count.load(std::memory_order_relaxed);
    });

    if (N > entries.size()) {
        N = entries.size();
    }
    entries.resize(N);
    return entries;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::startAutoSorting(
    std::chrono::milliseconds interval, bool ascending) {
    {
        if (sortingThread_.joinable()) {
            return;
        }
        stopSorting.store(false, std::memory_order_relaxed);
    }
    sortingThread_ = std::thread(&CountingHashTable::sortingWorker, this,
                                 interval, ascending);
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::stopAutoSorting() {
    stopSorting.store(true, std::memory_order_relaxed);
    if (sortingThread_.joinable()) {
        sortingThread_.join();
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::sortingWorker(
    std::chrono::milliseconds interval, bool ascending) {
    while (!stopSorting.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(interval);
        if (stopSorting.load(std::memory_order_relaxed)) {
            break;
        }
        std::vector<std::pair<Key, Entry>> entries;
        // Lock all mutexes in a consistent order to avoid deadlocks
        for (size_t i = 0; i < num_mutexes_; ++i) {
            mutexes_[i].lock();
        }
        entries.reserve(table_.size());
        for (const auto& [key, entry] : table_) {
            entries.emplace_back(key, entry);
        }
        for (size_t i = 0; i < num_mutexes_; ++i) {
            mutexes_[i].unlock();
        }

        std::sort(
            entries.begin(), entries.end(),
            [ascending](const auto& a, const auto& b) -> bool {
                if (ascending) {
                    return a.second.count.load(std::memory_order_relaxed) <
                           b.second.count.load(std::memory_order_relaxed);
                }
                return a.second.count.load(std::memory_order_relaxed) >
                       b.second.count.load(std::memory_order_relaxed);
            });

        // Rebuild the table
        for (auto& [key, entry] : entries) {
            size_t index = getMutexIndex(key);
            std::unique_lock lock(mutexes_[index]);
            table_[key] = std::move(entry);
        }
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::serializeToJson() const -> json {
    json j;
    // Lock all mutexes in a consistent order to avoid deadlocks
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].lock();
    }
    for (const auto& [key, entry] : table_) {
        j.push_back({{"key", key},
                     {"value", entry.value},
                     {"count", entry.count.load(std::memory_order_relaxed)}});
    }
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].unlock();
    }
    return j;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::deserializeFromJson(const json& j) {
    // Lock all mutexes in a consistent order to avoid deadlocks
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].lock();
    }
    table_.clear();
    for (const auto& item : j) {
        Key key = item.at("key").get<Key>();
        Value value = item.at("value").get<Value>();
        size_t count = item.at("count").get<size_t>();
        Entry entry(std::move(value));
        entry.count.store(count, std::memory_order_relaxed);
        table_.emplace(std::move(key), std::move(entry));
    }
    for (size_t i = 0; i < num_mutexes_; ++i) {
        mutexes_[i].unlock();
    }
}

}  // namespace atom::type

#endif  // ATOM_TYPE_COUNTING_HASH_TABLE_HPP
