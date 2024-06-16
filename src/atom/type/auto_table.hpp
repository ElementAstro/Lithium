#ifndef COUNTING_HASH_TABLE_HPP
#define COUNTING_HASH_TABLE_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
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
        Entry(Value v) : value(std::move(v)) {}
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
    std::optional<Value> get(const Key& key);

    /**
     * @brief Erases the entry associated with a given key.
     *
     * @param key The key to erase.
     * @return true if the key was found and erased, false otherwise.
     */
    bool erase(const Key& key);

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
    std::vector<std::pair<Key, Entry>> getAllEntries() const;

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
        mtx;  ///< Mutex for synchronizing access to the hash table.
    std::unordered_map<Key, Entry> table;  ///< The underlying hash table.
    std::atomic_flag stopSorting =
        ATOMIC_FLAG_INIT;        ///< Flag to indicate whether to stop automatic
                                 ///< sorting.
    std::jthread sortingThread;  ///< Thread for automatic sorting.
    std::condition_variable_any
        cv;  ///< Condition variable for controlling the sorting thread.
    std::mutex cv_mtx;  ///< Mutex for the condition variable.
};
}  // namespace atom::type

#include "auto_table.tpp"

#endif  // COUNTING_HASH_TABLE_HPP