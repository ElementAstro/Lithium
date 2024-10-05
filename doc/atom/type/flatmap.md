# atom::type::QuickFlatMap and QuickFlatMultiMap Documentation

The `atom::type` namespace provides two flat map implementations: `QuickFlatMap` and `QuickFlatMultiMap`. These classes offer efficient key-value storage using a sorted vector as the underlying container.

## Table of Contents

1. [Overview](#overview)
2. [QuickFlatMap](#quickflatmap)
   - [Template Parameters](#template-parameters)
   - [Member Functions](#member-functions)
   - [Usage Examples](#quickflatmap-usage-examples)
3. [QuickFlatMultiMap](#quickflatmultimap)
   - [Template Parameters](#template-parameters-1)
   - [Member Functions](#member-functions-1)
   - [Usage Examples](#quickflatmultimap-usage-examples)
4. [Performance Considerations](#performance-considerations)

## Overview

Both `QuickFlatMap` and `QuickFlatMultiMap` are designed to provide fast lookup times while maintaining a small memory footprint. They use a sorted vector as the underlying container, which allows for cache-friendly operations and efficient memory usage.

## QuickFlatMap

`QuickFlatMap` is a flat map implementation that stores unique key-value pairs.

### Template Parameters

```cpp
template <typename Key, typename Value, typename Comparator = std::equal_to<>>
```

- `Key`: The type of the keys in the map.
- `Value`: The type of the values in the map.
- `Comparator`: A binary predicate that defines equality between keys (default is `std::equal_to<>`).

### Member Functions

- `find`: Finds an element with the specified key.
- `size`: Returns the number of elements in the map.
- `empty`: Checks if the map is empty.
- `begin`, `end`: Returns iterators to the beginning and end of the map.
- `operator[]`: Accesses or inserts an element with the given key.
- `atIndex`: Accesses the value at the specified index.
- `at`: Accesses an element with bounds checking.
- `insertOrAssign`: Inserts a new element or assigns to an existing one.
- `insert`: Inserts a new element into the map.
- `assign`: Assigns a range of elements to the map.
- `contains`: Checks if the map contains a specific key.
- `erase`: Removes an element with the specified key.

### QuickFlatMap Usage Examples

```cpp
#include "flatmap.hpp"
#include <iostream>
#include <string>

int main() {
    atom::type::QuickFlatMap<std::string, int> map;

    // Inserting elements
    map.insert({"apple", 5});
    map.insert({"banana", 3});
    map["cherry"] = 7;

    // Accessing elements
    std::cout << "Value of 'apple': " << map["apple"] << std::endl;
    std::cout << "Value of 'banana': " << map.at("banana") << std::endl;

    // Checking if a key exists
    if (map.contains("cherry")) {
        std::cout << "Cherry exists in the map" << std::endl;
    }

    // Iterating through the map
    for (const auto& [key, value] : map) {
        std::cout << key << ": " << value << std::endl;
    }

    // Erasing an element
    map.erase("banana");

    // Using insertOrAssign
    map.insertOrAssign("date", 4);

    // Accessing by index
    std::cout << "First element value: " << map.atIndex(0) << std::endl;

    return 0;
}
```

## QuickFlatMultiMap

`QuickFlatMultiMap` is a flat multi-map implementation that allows multiple values for the same key.

### Template Parameters

```cpp
template <typename Key, typename Value, typename Comparator = std::equal_to<>>
```

- `Key`: The type of the keys in the multi-map.
- `Value`: The type of the values in the multi-map.
- `Comparator`: A binary predicate that defines equality between keys (default is `std::equal_to<>`).

### Member Functions

- `find`: Finds an element with the specified key.
- `equalRange`: Finds the range of elements with the specified key.
- `size`: Returns the number of elements in the multi-map.
- `empty`: Checks if the multi-map is empty.
- `begin`, `end`: Returns iterators to the beginning and end of the multi-map.
- `operator[]`: Accesses or inserts an element with the given key.
- `atIndex`: Accesses the value at the specified index.
- `at`: Accesses an element with bounds checking.
- `insert`: Inserts a new element into the multi-map.
- `assign`: Assigns a range of elements to the multi-map.
- `count`: Counts the number of elements with the specified key.
- `contains`: Checks if the multi-map contains a specific key.
- `erase`: Removes all elements with the specified key.

### QuickFlatMultiMap Usage Examples

```cpp
#include "flatmap.hpp"
#include <iostream>
#include <string>

int main() {
    atom::type::QuickFlatMultiMap<std::string, int> multimap;

    // Inserting elements
    multimap.insert({"apple", 5});
    multimap.insert({"apple", 7});
    multimap.insert({"banana", 3});
    multimap["cherry"] = 7;

    // Accessing elements
    std::cout << "First value of 'apple': " << multimap["apple"] << std::endl;

    // Counting elements with a specific key
    std::cout << "Number of 'apple' entries: " << multimap.count("apple") << std::endl;

    // Iterating through all elements
    for (const auto& [key, value] : multimap) {
        std::cout << key << ": " << value << std::endl;
    }

    // Using equalRange to iterate through elements with a specific key
    auto [begin, end] = multimap.equalRange("apple");
    std::cout << "All 'apple' values:" << std::endl;
    for (auto it = begin; it != end; ++it) {
        std::cout << it->second << " ";
    }
    std::cout << std::endl;

    // Erasing all elements with a specific key
    multimap.erase("apple");

    // Checking if a key exists
    if (multimap.contains("banana")) {
        std::cout << "Banana exists in the multimap" << std::endl;
    }

    return 0;
}
```

## Performance Considerations

1. **Lookup Performance**: Both `QuickFlatMap` and `QuickFlatMultiMap` use linear search for lookups. This is efficient for small to medium-sized containers but may not be suitable for very large datasets.

2. **Insertion Performance**: Insertions are always performed at the end of the vector, which is efficient. However, this means that the container is not kept sorted, which affects lookup performance.

3. **Memory Efficiency**: These implementations use a vector as the underlying container, which provides better cache locality and memory efficiency compared to node-based containers like `std::map` or `std::unordered_map`.

4. **Use Cases**: These containers are best suited for scenarios where:

   - The number of elements is small to medium.
   - Lookups are less frequent than insertions or iterations.
   - Memory efficiency and cache performance are important.

5. **Custom Comparators**: The ability to use custom comparators allows for flexible key comparison, which can be useful in scenarios involving case-insensitive string keys or custom object comparisons.
