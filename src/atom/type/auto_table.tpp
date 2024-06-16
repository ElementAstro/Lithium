#include "auto_table.hpp"

namespace atom::type {
template <typename Key, typename Value>
CountingHashTable<Key, Value>::CountingHashTable() {}

template <typename Key, typename Value>
CountingHashTable<Key, Value>::~CountingHashTable() {
    stopAutoSorting();
}

template <typename Key, typename Value>
void CountingHashTable<Key, Value>::insert(const Key& key, const Value& value) {
    std::unique_lock lock(mtx);
    table[key] = Entry(value);
}

template <typename Key, typename Value>
std::optional<Value> CountingHashTable<Key, Value>::get(const Key& key) {
    std::shared_lock lock(mtx);
    auto it = table.find(key);
    if (it != table.end()) {
        it->second.count++;
        return it->second.value;
    }
    return std::nullopt;
}

template <typename Key, typename Value>
bool CountingHashTable<Key, Value>::erase(const Key& key) {
    std::unique_lock lock(mtx);
    return table.erase(key) > 0;
}

template <typename Key, typename Value>
void CountingHashTable<Key, Value>::clear() {
    std::unique_lock lock(mtx);
    table.clear();
}

template <typename Key, typename Value>
std::vector<std::pair<Key, typename CountingHashTable<Key, Value>::Entry>>
CountingHashTable<Key, Value>::getAllEntries() const {
    std::shared_lock lock(mtx);
    std::vector<std::pair<Key, Entry>> entries;
    for (const auto& [key, entry] : table) {
        entries.emplace_back(key, entry);
    }
    return entries;
}

template <typename Key, typename Value>
void CountingHashTable<Key, Value>::sortEntriesByCountDesc() {
    std::vector<std::pair<Key, Entry>> entries;
    {
        std::shared_lock lock(mtx);
        entries.assign(table.begin(), table.end());
    }
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.second.count > b.second.count;
    });
    {
        std::unique_lock lock(mtx);
        table.clear();
        for (const auto& [key, entry] : entries) {
            table[key] = entry;
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
