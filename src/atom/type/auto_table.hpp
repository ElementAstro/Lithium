#ifndef ATOM_TYPE_COUNTING_HASH_TABLE_HPP
#define ATOM_TYPE_COUNTING_HASH_TABLE_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <shared_mutex>
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
class CountingHashTable {
public:
    /**
     * @brief Struct representing an entry in the hash table.
     */
    struct Entry {
        Value value;       ///< The value stored in the entry.
        size_t count = 0;  ///< The access count of the entry.

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
     * @brief Retrieves the value associated with a given key.
     *
     * @param key The key to retrieve the value for.
     * @return An optional containing the value if found, otherwise
     * std::nullopt.
     */
    auto get(const Key& key) -> std::optional<Value>;

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
    mutable std::shared_mutex
        mtx_;  ///< Mutex for synchronizing access to the hash table.
    std::unordered_map<Key, Entry> table_;  ///< The underlying hash table.
    std::atomic_flag stopSorting =
        ATOMIC_FLAG_INIT;        ///< Flag to indicate whether to stop automatic
                                 ///< sorting.
    std::jthread sortingThread;  ///< Thread for automatic sorting.
    std::condition_variable_any
        cv;  ///< Condition variable for controlling the sorting thread.
    std::mutex cv_mtx;  ///< Mutex for the condition variable.
};

template <typename Key, typename Value>
CountingHashTable<Key, Value>::~CountingHashTable() {
    stopAutoSorting();
}

template <typename Key, typename Value>
void CountingHashTable<Key, Value>::insert(const Key& key, const Value& value) {
    std::unique_lock lock(mtx_);
    table_[key] = Entry(value);
}

template <typename Key, typename Value>
std::optional<Value> CountingHashTable<Key, Value>::get(const Key& key) {
    std::shared_lock lock(mtx_);
    auto it = table_.find(key);
    if (it != table_.end()) {
        it->second.count++;
        return it->second.value;
    }
    return std::nullopt;
}

template <typename Key, typename Value>
bool CountingHashTable<Key, Value>::erase(const Key& key) {
    std::unique_lock lock(mtx_);
    return table_.erase(key) > 0;
}

template <typename Key, typename Value>
void CountingHashTable<Key, Value>::clear() {
    std::unique_lock lock(mtx_);
    table_.clear();
}

template <typename Key, typename Value>
std::vector<std::pair<Key, typename CountingHashTable<Key, Value>::Entry>>
CountingHashTable<Key, Value>::getAllEntries() const {
    std::shared_lock lock(mtx_);
    std::vector<std::pair<Key, Entry>> entries;
    for (const auto& [key, entry] : table_) {
        entries.emplace_back(key, entry);
    }
    return entries;
}

template <typename Key, typename Value>
void CountingHashTable<Key, Value>::sortEntriesByCountDesc() {
    std::vector<std::pair<Key, Entry>> entries;
    {
        std::shared_lock lock(mtx_);
        entries.assign(table_.begin(), table_.end());
    }
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.second.count > b.second.count;
    });
    {
        std::unique_lock lock(mtx_);
        table_.clear();
        for (const auto& [key, entry] : entries) {
            table_[key] = entry;
        }
    }
}

template <typename Key, typename Value>
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
void CountingHashTable<Key, Value>::stopAutoSorting() {
    stopSorting.test_and_set();
    cv.notify_all();
}
}  // namespace atom::type
#endif  // ATOM_TYPE_COUNTING_HASH_TABLE_HPP