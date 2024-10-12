# TTLCache Class Documentation

## Overview

The `TTLCache` class is a template-based implementation of a Time-to-Live (TTL) cache with a maximum capacity and Least Recently Used (LRU) eviction policy. It is part of the `atom::search` namespace and provides efficient caching with automatic expiration of items.

## Table of Contents

1. [Class Declaration](#class-declaration)
2. [Constructor and Destructor](#constructor-and-destructor)
3. [Basic Operations](#basic-operations)
4. [Cache Management](#cache-management)
5. [Statistics](#statistics)
6. [Thread Safety](#thread-safety)
7. [Usage Examples](#usage-examples)
8. [Best Practices](#best-practices)

## Class Declaration

```cpp
namespace atom::search {
template <typename Key, typename Value>
class TTLCache {
    // ... (class members and methods)
};
}
```

The `TTLCache` class is templated with `Key` and `Value` types, allowing for flexible usage with different data types.

## Constructor and Destructor

### Constructor

```cpp
TTLCache(Duration ttl, size_t max_capacity);
```

Creates a new `TTLCache` with the specified TTL and maximum capacity.

- `ttl`: Duration after which items expire and are removed from the cache.
- `max_capacity`: Maximum number of items the cache can hold.

### Destructor

```cpp
~TTLCache();
```

Destroys the `TTLCache` object and stops the cleaner thread.

## Basic Operations

### Put

```cpp
void put(const Key& key, const Value& value);
```

Inserts a new key-value pair into the cache or updates an existing key.

- If the key already exists, its value is updated and moved to the front of the cache (most recently used).
- If the cache is at maximum capacity, the least recently used item is evicted before inserting the new item.

### Get

```cpp
std::optional<Value> get(const Key& key);
```

Retrieves the value associated with the given key from the cache.

- Returns an `std::optional<Value>` containing the value if found and not expired; otherwise, returns `std::nullopt`.
- Accessing an item moves it to the front of the cache (most recently used).

## Cache Management

### Cleanup

```cpp
void cleanup();
```

Performs cache cleanup by removing expired items. This method is called automatically by the cleaner thread, but can also be called manually if needed.

### Clear

```cpp
void clear();
```

Clears all items from the cache and resets hit/miss counts.

## Statistics

### Hit Rate

```cpp
double hitRate() const;
```

Returns the cache hit rate (ratio of cache hits to total accesses).

### Size

```cpp
size_t size() const;
```

Returns the current number of items in the cache.

## Thread Safety

The `TTLCache` class is designed to be thread-safe:

- It uses a `std::shared_mutex` for read-write locking, allowing multiple concurrent reads but exclusive writes.
- The cleaner thread runs in the background, periodically removing expired items.

## Usage Examples

### Basic Usage

```cpp
#include "TTLCache.hpp"
#include <iostream>
#include <string>

int main() {
    // Create a cache with 5-second TTL and maximum capacity of 100 items
    atom::search::TTLCache<int, std::string> cache(std::chrono::seconds(5), 100);

    // Insert items
    cache.put(1, "One");
    cache.put(2, "Two");
    cache.put(3, "Three");

    // Retrieve items
    auto value = cache.get(2);
    if (value) {
        std::cout << "Value for key 2: " << *value << std::endl;
    } else {
        std::cout << "Key 2 not found or expired" << std::endl;
    }

    // Wait for items to expire
    std::this_thread::sleep_for(std::chrono::seconds(6));

    // Try to retrieve an expired item
    value = cache.get(1);
    if (!value) {
        std::cout << "Key 1 has expired" << std::endl;
    }

    // Check cache statistics
    std::cout << "Cache size: " << cache.size() << std::endl;
    std::cout << "Hit rate: " << cache.hitRate() << std::endl;

    return 0;
}
```

### LRU Eviction Example

```cpp
atom::search::TTLCache<int, std::string> cache(std::chrono::minutes(5), 3);

// Insert 3 items (maximum capacity)
cache.put(1, "One");
cache.put(2, "Two");
cache.put(3, "Three");

// Access item 1, making it the most recently used
cache.get(1);

// Insert a new item, which should evict item 2 (least recently used)
cache.put(4, "Four");

// Check if items exist
std::cout << "Item 1 exists: " << (cache.get(1).has_value() ? "Yes" : "No") << std::endl;
std::cout << "Item 2 exists: " << (cache.get(2).has_value() ? "Yes" : "No") << std::endl;
std::cout << "Item 3 exists: " << (cache.get(3).has_value() ? "Yes" : "No") << std::endl;
std::cout << "Item 4 exists: " << (cache.get(4).has_value() ? "Yes" : "No") << std::endl;
```

### Thread-Safe Usage

```cpp
#include <thread>
#include <vector>

atom::search::TTLCache<int, int> cache(std::chrono::seconds(30), 1000);

void worker(int id, int num_operations) {
    for (int i = 0; i < num_operations; ++i) {
        int key = std::rand() % 1000;
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

## Best Practices

1. **Choose Appropriate TTL**: Set a TTL that balances data freshness with cache efficiency. Too short a TTL may result in frequent cache misses, while too long a TTL may serve stale data.

2. **Set Reasonable Max Capacity**: Choose a maximum capacity that fits your memory constraints and expected data volume.

3. **Monitor Hit Rate**: Regularly check the cache's hit rate to ensure it's providing good performance. A low hit rate might indicate that the TTL is too short or the max capacity is too low.

4. **Use Appropriate Key and Value Types**: Choose key and value types that are efficient to copy and compare, as these operations are performed frequently in the cache.
