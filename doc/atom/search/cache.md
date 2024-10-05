# ResourceCache Class Documentation

## Overview

The `ResourceCache` class is a template-based caching system designed for efficient storage and retrieval of resources. It's part of the `atom::search` namespace and provides various features such as expiration, least recently used (LRU) eviction, asynchronous operations, and persistent storage.

## Table of Contents

1. [Class Declaration](#class-declaration)
2. [Constructor and Destructor](#constructor-and-destructor)
3. [Basic Operations](#basic-operations)
4. [Asynchronous Operations](#asynchronous-operations)
5. [Cache Management](#cache-management)
6. [Expiration and Cleanup](#expiration-and-cleanup)
7. [Persistence](#persistence)
8. [Bulk Operations](#bulk-operations)
9. [Usage Examples](#usage-examples)

## Class Declaration

```cpp
template <typename T>
concept Cacheable = std::copy_constructible<T> && std::is_copy_assignable_v<T>;

template <Cacheable T>
class ResourceCache {
    // ... (class members and methods)
};
```

The `ResourceCache` class is templated with a type `T` that must satisfy the `Cacheable` concept, ensuring it's copy-constructible and copy-assignable.

## Constructor and Destructor

```cpp
explicit ResourceCache(int maxSize);
~ResourceCache();
```

- The constructor takes a `maxSize` parameter to set the maximum number of items in the cache.
- The destructor ensures proper cleanup of resources, including stopping the cleanup thread.

## Basic Operations

### Insert

```cpp
void insert(const std::string &key, const T &value, std::chrono::seconds expirationTime);
```

Inserts a new item into the cache with the specified key, value, and expiration time.

### Get

```cpp
auto get(const std::string &key) -> std::optional<T>;
```

Retrieves an item from the cache. Returns `std::nullopt` if the item doesn't exist or has expired.

### Remove

```cpp
void remove(const std::string &key);
```

Removes an item from the cache.

### Contains

```cpp
auto contains(const std::string &key) const -> bool;
```

Checks if an item exists in the cache.

## Asynchronous Operations

### Async Get

```cpp
auto asyncGet(const std::string &key) -> std::future<std::optional<T>>;
```

Asynchronously retrieves an item from the cache.

### Async Insert

```cpp
auto asyncInsert(const std::string &key, const T &value, std::chrono::seconds expirationTime) -> std::future<void>;
```

Asynchronously inserts an item into the cache.

### Async Load

```cpp
auto asyncLoad(const std::string &key, std::function<T()> loadDataFunction) -> std::future<void>;
```

Asynchronously loads data into the cache using a provided function.

## Cache Management

### Clear

```cpp
void clear();
```

Removes all items from the cache.

### Size

```cpp
auto size() const -> size_t;
```

Returns the number of items in the cache.

### Empty

```cpp
auto empty() const -> bool;
```

Checks if the cache is empty.

### Set Max Size

```cpp
void setMaxSize(int maxSize);
```

Sets the maximum number of items the cache can hold.

## Expiration and Cleanup

### Is Expired

```cpp
auto isExpired(const std::string &key) const -> bool;
```

Checks if an item has expired.

### Set Expiration Time

```cpp
void setExpirationTime(const std::string &key, std::chrono::seconds expirationTime);
```

Sets or updates the expiration time for a specific item.

### Remove Expired

```cpp
void removeExpired();
```

Removes all expired items from the cache.

## Persistence

### Read From File

```cpp
void readFromFile(const std::string &filePath, const std::function<T(const std::string &)> &deserializer);
```

Loads cache contents from a file using a custom deserializer.

### Write To File

```cpp
void writeToFile(const std::string &filePath, const std::function<std::string(const T &)> &serializer);
```

Saves cache contents to a file using a custom serializer.

### Read From JSON File

```cpp
void readFromJsonFile(const std::string &filePath, const std::function<T(const json &)> &fromJson);
```

Loads cache contents from a JSON file using a custom JSON deserializer.

### Write To JSON File

```cpp
void writeToJsonFile(const std::string &filePath, const std::function<json(const T &)> &toJson);
```

Saves cache contents to a JSON file using a custom JSON serializer.

## Bulk Operations

### Insert Batch

```cpp
void insertBatch(const std::vector<std::pair<std::string, T>> &items, std::chrono::seconds expirationTime);
```

Inserts multiple items into the cache at once.

### Remove Batch

```cpp
void removeBatch(const std::vector<std::string> &keys);
```

Removes multiple items from the cache at once.

## Usage Examples

### Basic Usage

```cpp
ResourceCache<std::string> cache(100); // Create a cache with max size 100

// Insert an item
cache.insert("key1", "value1", std::chrono::seconds(60));

// Retrieve an item
auto value = cache.get("key1");
if (value) {
    std::cout << "Value: " << *value << std::endl;
} else {
    std::cout << "Key not found or expired" << std::endl;
}

// Remove an item
cache.remove("key1");
```

### Asynchronous Operations

```cpp
auto future = cache.asyncInsert("key2", "value2", std::chrono::seconds(120));
future.wait(); // Wait for the insertion to complete

auto valueFuture = cache.asyncGet("key2");
auto value = valueFuture.get();
if (value) {
    std::cout << "Async retrieved value: " << *value << std::endl;
}
```

### JSON Persistence

```cpp
// Save to JSON file
cache.writeToJsonFile("cache_data.json", [](const std::string& value) {
    return json{{"data", value}};
});

// Load from JSON file
cache.readFromJsonFile("cache_data.json", [](const json& j) {
    return j["data"].get<std::string>();
});
```

### Bulk Operations

```cpp
std::vector<std::pair<std::string, std::string>> items = {
    {"key3", "value3"},
    {"key4", "value4"},
    {"key5", "value5"}
};
cache.insertBatch(items, std::chrono::seconds(300));

std::vector<std::string> keysToRemove = {"key3", "key4"};
cache.removeBatch(keysToRemove);
```
