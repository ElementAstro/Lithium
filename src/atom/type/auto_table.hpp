#ifndef ATOM_TYPE_COUNTING_HASH_TABLE_HPP
#define ATOM_TYPE_COUNTING_HASH_TABLE_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <concepts>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace atom::type {
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
        Value value;                   ///< The value stored in the entry.
        std::atomic<size_t> count{0};  ///< The access count of the entry.

        /**
         * @brief Default constructor.
         */
        Entry() = default;

        /**
         * @brief Constructs an Entry with a given value.
         *
         * @param v The value to store in the entry.
         */
        explicit Entry(Value val) : value(std::move(val)) {}
    };

    /**
     * @brief Constructs a new CountingHashTable object.
     */
    CountingHashTable();

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
     * @brief Starts automatic sorting of the hash table entries at regular
     * intervals.
     *
     * @param interval The interval at which to sort the entries.
     */
    void startAutoSorting(std::chrono::milliseconds interval);

    /**
     * @brief Stops automatic sorting of the hash table entries.
     */
    void stopAutoSorting();

private:
    std::unordered_map<Key, Entry> table_;  ///< The underlying hash table.
    std::atomic_flag stopSorting =
        ATOMIC_FLAG_INIT;        ///< Flag to indicate whether to stop automatic
                                 ///< sorting.
    std::jthread sortingThread;  ///< Thread for automatic sorting.
};

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
CountingHashTable<Key, Value>::CountingHashTable() {}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
CountingHashTable<Key, Value>::~CountingHashTable() {
    stopAutoSorting();
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::insert(const Key& key, const Value& value) {
    Entry newEntry(value);
    auto it = table_.find(key);
    if (it == table_.end()) {
        table_.emplace(key, std::move(newEntry));
    } else {
        it->second.value = std::move(newEntry.value);
        it->second.count.store(newEntry.count.load(std::memory_order_relaxed),
                               std::memory_order_relaxed);
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::insertBatch(
    const std::vector<std::pair<Key, Value>>& items) {
    for (const auto& [key, value] : items) {
        insert(key, value);
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::get(const Key& key)
    -> std::optional<Value> {
    auto it = table_.find(key);
    if (it != table_.end()) {
        it->second.count.fetch_add(1, std::memory_order_relaxed);
        return it->second.value;
    }
    return std::nullopt;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::getBatch(const std::vector<Key>& keys)
    -> std::vector<std::optional<Value>> {
    std::vector<std::optional<Value>> results;
    results.reserve(keys.size());
    for (const auto& key : keys) {
        results.push_back(get(key));
    }
    return results;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::erase(const Key& key) -> bool {
    return table_.erase(key) > 0;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::clear() {
    table_.clear();
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
auto CountingHashTable<Key, Value>::getAllEntries() const
    -> std::vector<std::pair<Key, Entry>> {
    std::vector<std::pair<Key, Entry>> entries;
    for (const auto& [key, entry] : table_) {
        entries.emplace_back(key, entry);
    }
    return entries;
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::sortEntriesByCountDesc() {
    std::vector<std::pair<Key, Entry>> entries(table_.begin(), table_.end());
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.second.count.load(std::memory_order_relaxed) >
               b.second.count.load(std::memory_order_relaxed);
    });
    table_.clear();
    for (const auto& [key, entry] : entries) {
        table_.emplace(key, entry);
    }
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::startAutoSorting(
    std::chrono::milliseconds interval) {
    stopSorting.clear();
    sortingThread = std::jthread([this, interval](std::stop_token st) {
        while (!stopSorting.test() && !st.stop_requested()) {
            std::this_thread::sleep_for(interval);
            if (!stopSorting.test()) {
                sortEntriesByCountDesc();
            }
        }
    });
}

template <typename Key, typename Value>
    requires std::equality_comparable<Key> && std::movable<Value>
void CountingHashTable<Key, Value>::stopAutoSorting() {
    stopSorting.test_and_set();
}

}  // namespace atom::type

#endif  // ATOM_TYPE_COUNTING_HASH_TABLE_HPP
