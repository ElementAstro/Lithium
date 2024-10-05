# CountingHashTable Class Documentation

The `CountingHashTable` is a thread-safe hash table implementation that counts the number of accesses to each entry. It is part of the `atom::type` namespace and is defined in the `auto_table.hpp` header file.

## Table of Contents

1. [Overview](#overview)
2. [Template Parameters](#template-parameters)
3. [Class Members](#class-members)
4. [Constructor and Destructor](#constructor-and-destructor)
5. [Public Methods](#public-methods)
6. [Usage Examples](#usage-examples)

## Overview

The `CountingHashTable` class provides a thread-safe implementation of a hash table that keeps track of the number of times each entry is accessed. It supports operations such as insertion, retrieval, deletion, and automatic sorting based on access counts.

## Template Parameters

The class is templated with two parameters:

- `Key`: The type of the keys in the hash table. Must be equality comparable.
- `Value`: The type of the values in the hash table. Must be movable.

## Class Members

### Entry Struct

The `Entry` struct represents an individual entry in the hash table:

```cpp
struct Entry {
    Value value;                   // The value stored in the entry
    std::atomic<size_t> count{0};  // The access count of the entry

    Entry() = default;
    explicit Entry(Value val) : value(std::move(val)) {}
};
```

### Private Members

- `std::unordered_map<Key, Entry> table_`: The underlying hash table.
- `std::atomic_flag stopSorting`: Flag to indicate whether to stop automatic sorting.
- `std::jthread sortingThread`: Thread for automatic sorting.

## Constructor and Destructor

```cpp
CountingHashTable();
~CountingHashTable();
```

The constructor initializes an empty hash table. The destructor ensures that automatic sorting is stopped before the object is destroyed.

## Public Methods

### Insertion

```cpp
void insert(const Key& key, const Value& value);
void insertBatch(const std::vector<std::pair<Key, Value>>& items);
```

Insert a single key-value pair or multiple pairs into the hash table.

### Retrieval

```cpp
auto get(const Key& key) -> std::optional<Value>;
auto getBatch(const std::vector<Key>& keys) -> std::vector<std::optional<Value>>;
```

Retrieve the value associated with a single key or multiple keys. Returns `std::nullopt` if the key is not found.

### Deletion

```cpp
auto erase(const Key& key) -> bool;
void clear();
```

Remove a single entry or clear all entries from the hash table.

### Entry Management

```cpp
auto getAllEntries() const -> std::vector<std::pair<Key, Entry>>;
void sortEntriesByCountDesc();
```

Retrieve all entries or sort them by access count in descending order.

### Automatic Sorting

```cpp
void startAutoSorting(std::chrono::milliseconds interval);
void stopAutoSorting();
```

Start or stop automatic sorting of entries at regular intervals.

## Usage Examples

Here are some examples demonstrating how to use the `CountingHashTable` class:

```cpp
#include "auto_table.hpp"
#include <iostream>
#include <string>

int main() {
    atom::type::CountingHashTable<std::string, int> table;

    // Inserting key-value pairs
    table.insert("one", 1);
    table.insert("two", 2);
    table.insert("three", 3);

    // Retrieving values
    auto value = table.get("two");
    if (value) {
        std::cout << "Value for 'two': " << *value << std::endl;
    }

    // Batch insertion
    std::vector<std::pair<std::string, int>> batch = {
        {"four", 4},
        {"five", 5}
    };
    table.insertBatch(batch);

    // Batch retrieval
    auto results = table.getBatch({"one", "three", "five"});
    for (const auto& result : results) {
        if (result) {
            std::cout << "Found value: " << *result << std::endl;
        } else {
            std::cout << "Value not found" << std::endl;
        }
    }

    // Erasing an entry
    bool erased = table.erase("two");
    std::cout << "Erased 'two': " << (erased ? "yes" : "no") << std::endl;

    // Getting all entries
    auto allEntries = table.getAllEntries();
    std::cout << "All entries:" << std::endl;
    for (const auto& [key, entry] : allEntries) {
        std::cout << key << ": " << entry.value << " (count: " << entry.count.load() << ")" << std::endl;
    }

    // Sorting entries by count
    table.sortEntriesByCountDesc();

    // Start auto-sorting
    table.startAutoSorting(std::chrono::seconds(5));

    // Perform some operations...

    // Stop auto-sorting
    table.stopAutoSorting();

    // Clear the table
    table.clear();

    return 0;
}
```
