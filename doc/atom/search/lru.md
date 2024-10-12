# ThreadSafeLRUCache Class Documentation

## Overview

The `ThreadSafeLRUCache` class is a template-based, thread-safe implementation of a Least Recently Used (LRU) cache. It is part of the `atom::search` namespace and provides efficient caching with expiration, persistence, and various utility functions.

## Table of Contents

1. [Class Declaration](#class-declaration)
2. [Constructor](#constructor)
3. [Basic Operations](#basic-operations)
4. [Cache Management](#cache-management)
5. [Callbacks](#callbacks)
6. [Statistics](#statistics)
7. [Persistence](#persistence)
8. [Usage Examples](#usage-examples)

## Class Declaration

```cpp
namespace atom::search {
template <typename Key, typename Value>
class ThreadSafeLRUCache {
    // ... (class members and methods)
};
}
```

The `ThreadSafeLRUCache` class is templated with `Key` and `Value` types, allowing for flexible usage with different data types.

## Constructor

```cpp
explicit ThreadSafeLRUCache(size_t max_size);
```

Creates a new `ThreadSafeLRUCache` with the specified maximum size.

- `max_size`: The maximum number of items that the cache can hold.

## Basic Operations

### Get

```cpp
auto get(const Key& key) -> std::optional<Value>;
```

Retrieves a value from the cache. If the item is found and not expired, it's moved to the front of the cache (marking it as recently used).

- Returns: An `std::optional<Value>` containing the value if found and not expired, otherwise `std::nullopt`.

### Put

```cpp
void put(const Key& key, const Value& value, std::optional<std::chrono::seconds> ttl = std::nullopt);
```

Inserts or updates a value in the cache.

- `key`: The key to insert or update.
- `value`: The value to associate with the key.
- `ttl`: Optional time-to-live duration for the cache item.

If the cache is full, the least recently used item is removed.

### Erase

```cpp
void erase(const Key& key);
```

Removes an item from the cache.

### Clear

```cpp
void clear();
```

Removes all items from the cache.

## Cache Management

### Resize

```cpp
void resize(size_t new_max_size);
```

Resizes the cache to a new maximum size. If the new size is smaller, the least recently used items are removed until the cache size fits.

### Size

```cpp
auto size() const -> size_t;
```

Returns the current number of items in the cache.

### Load Factor

```cpp
auto loadFactor() const -> float;
```

Returns the current load factor of the cache (ratio of current size to maximum size).

### Keys

```cpp
auto keys() const -> std::vector<Key>;
```

Returns a vector containing all keys currently in the cache.

### Pop LRU

```cpp
auto popLru() -> std::optional<KeyValuePair>;
```

Removes and returns the least recently used item from the cache.

## Callbacks

The class provides methods to set callback functions for various events:

```cpp
void setInsertCallback(std::function<void(const Key&, const Value&)> callback);
void setEraseCallback(std::function<void(const Key&)> callback);
void setClearCallback(std::function<void()> callback);
```

These methods allow you to set custom behavior when items are inserted, erased, or when the cache is cleared.

## Statistics

### Hit Rate

```cpp
auto hitRate() const -> float;
```

Returns the hit rate of the cache (ratio of cache hits to total cache accesses).

## Persistence

### Save to File

```cpp
void saveToFile(const std::string& filename) const;
```

Saves the cache contents to a file.

### Load from File

```cpp
void loadFromFile(const std::string& filename);
```

Loads cache contents from a file.

## Usage Examples

### Basic Usage

```cpp
#include "ThreadSafeLRUCache.h"
#include <iostream>
#include <string>

int main() {
    atom::search::ThreadSafeLRUCache<int, std::string> cache(3);  // Cache with max size 3

    // Insert items
    cache.put(1, "One");
    cache.put(2, "Two");
    cache.put(3, "Three");

    // Retrieve an item
    auto value = cache.get(2);
    if (value) {
        std::cout << "Value for key 2: " << *value << std::endl;
    }

    // Insert a new item, causing the least recently used item to be evicted
    cache.put(4, "Four");

    // Try to retrieve the evicted item
    value = cache.get(1);
    if (!value) {
        std::cout << "Key 1 not found in cache" << std::endl;
    }

    return 0;
}
```

### Using TTL (Time-To-Live)

```cpp
atom::search::ThreadSafeLRUCache<int, std::string> cache(5);

// Insert an item with a 2-second TTL
cache.put(1, "Short-lived", std::chrono::seconds(2));

// Retrieve the item immediately
auto value = cache.get(1);
if (value) {
    std::cout << "Value found: " << *value << std::endl;
}

// Wait for 3 seconds
std::this_thread::sleep_for(std::chrono::seconds(3));

// Try to retrieve the expired item
value = cache.get(1);
if (!value) {
    std::cout << "Item has expired and was not found" << std::endl;
}
```

### Using Callbacks

```cpp
atom::search::ThreadSafeLRUCache<int, std::string> cache(3);

cache.setInsertCallback([](const int& key, const std::string& value) {
    std::cout << "Inserted: " << key << " -> " << value << std::endl;
});

cache.setEraseCallback([](const int& key) {
    std::cout << "Erased key: " << key << std::endl;
});

cache.put(1, "One");  // Triggers insert callback
cache.put(2, "Two");  // Triggers insert callback
cache.erase(1);       // Triggers erase callback
```

### Persistence

```cpp
atom::search::ThreadSafeLRUCache<int, std::string> cache(5);

// Populate the cache
cache.put(1, "One");
cache.put(2, "Two");
cache.put(3, "Three");

// Save the cache to a file
cache.saveToFile("cache_backup.bin");

// Clear the cache
cache.clear();

// Load the cache from the file
cache.loadFromFile("cache_backup.bin");

// Verify the loaded data
for (int i = 1; i <= 3; ++i) {
    auto value = cache.get(i);
    if (value) {
        std::cout << "Loaded: " << i << " -> " << *value << std::endl;
    }
}
```

### Statistics and Cache Management (Continued)

```cpp
std::cout << "Load factor: " << cache.loadFactor() << std::endl;

// Perform some cache operations
for (int i = 0; i < 20; ++i) {
    cache.get(i % 10);  // This will cause both hits and misses
}

std::cout << "Hit rate: " << cache.hitRate() << std::endl;

// Resize the cache
cache.resize(5);
std::cout << "After resizing to 5, cache size: " << cache.size() << std::endl;

// Get all keys
auto keys = cache.keys();
std::cout << "Current keys in cache: ";
for (const auto& key : keys) {
    std::cout << key << " ";
}
std::cout << std::endl;

// Pop the least recently used item
auto lru = cache.popLru();
if (lru) {
    std::cout << "Popped LRU item: " << lru->first << " -> " << lru->second << std::endl;
}
```

### Thread Safety Demonstration

This example shows how the cache can be safely used from multiple threads:

```cpp
#include <thread>
#include <vector>

atom::search::ThreadSafeLRUCache<int, int> cache(100);

void worker(int id, int num_operations) {
    for (int i = 0; i < num_operations; ++i) {
        int key = std::rand() % 200;
        if (std::rand() % 2 == 0) {
            cache.put(key, id * 1000 + i);
        } else {
            auto value = cache.get(key);
            if (value) {
                // Do something with the value
            }
        }
    }
}

int main() {
    std::vector<std::thread> threads;
    int num_threads = 4;
    int operations_per_thread = 10000;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i, operations_per_thread);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Final cache size: " << cache.size() << std::endl;
    std::cout << "Hit rate: " << cache.hitRate() << std::endl;

    return 0;
}
```

## Best Practices and Tips

1. **Choose Appropriate Key and Value Types**: The performance and memory usage of the cache can be significantly affected by the choice of key and value types. Use simple and compact types when possible.

2. **Set a Reasonable Cache Size**: Choose a maximum cache size that balances memory usage with hit rate for your specific use case.

3. **Use TTL Wisely**: Time-To-Live (TTL) can help ensure that cached data doesn't become stale, but it can also reduce the hit rate. Use TTL values that make sense for your data's update frequency.

4. **Monitor Cache Performance**: Regularly check the cache's hit rate and load factor to ensure it's performing well for your use case. Adjust the cache size or eviction policy if needed.

5. **Handle Cache Misses Gracefully**: Always check if the `get` operation returns a valid value, and have a strategy for handling cache misses.

6. **Use Callbacks for Logging or Synchronization**: The insert, erase, and clear callbacks can be useful for logging cache operations or synchronizing the cache with an external data store.

7. **Be Cautious with File I/O**: The `saveToFile` and `loadFromFile` operations can be slow for large caches. Use them judiciously, perhaps during application startup/shutdown or at specific checkpoints.

8. **Thread-Safety Considerations**: While the cache is thread-safe, be mindful of the operations you perform in callbacks, as they may introduce thread-safety issues if not handled carefully.

## Advanced Usage

### Custom Key and Value Types

The `ThreadSafeLRUCache` can be used with custom key and value types, as long as they meet certain requirements:

- The key type must be hashable and comparable.
- Both key and value types should be copyable.

Here's an example with custom types:

```cpp
struct CustomKey {
    int id;
    std::string name;

    bool operator==(const CustomKey& other) const {
        return id == other.id && name == other.name;
    }
};

struct CustomValue {
    double data;
    std::vector<int> more_data;
};

// Custom hash function for CustomKey
namespace std {
    template <>
    struct hash<CustomKey> {
        std::size_t operator()(const CustomKey& k) const {
            return std::hash<int>()(k.id) ^ std::hash<std::string>()(k.name);
        }
    };
}

// Usage
atom::search::ThreadSafeLRUCache<CustomKey, CustomValue> custom_cache(10);

custom_cache.put(CustomKey{1, "one"}, CustomValue{1.0, {1, 2, 3}});
```

### Implementing a Read-Through Cache

You can implement a read-through cache pattern by combining the `get` method with a data fetching function:

```cpp
template<typename Key, typename Value>
Value getOrFetch(atom::search::ThreadSafeLRUCache<Key, Value>& cache, const Key& key, std::function<Value(const Key&)> fetchFunc) {
    auto cachedValue = cache.get(key);
    if (cachedValue) {
        return *cachedValue;
    }

    Value fetchedValue = fetchFunc(key);
    cache.put(key, fetchedValue);
    return fetchedValue;
}

// Usage
atom::search::ThreadSafeLRUCache<int, std::string> cache(100);

auto value = getOrFetch(cache, 42, [](int key) {
    // Simulate fetching data from a slow source
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return "Value for key " + std::to_string(key);
});
```

This approach ensures that the cache is always checked first, and only fetches the data from the slow source when necessary.

## Conclusion

The `ThreadSafeLRUCache` class provides a robust, thread-safe implementation of an LRU cache with additional features like TTL, persistence, and performance monitoring. By following the best practices and examples provided, you can effectively use this cache in various scenarios to improve the performance of your applications.
