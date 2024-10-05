# atom::type::FlatSet Documentation

The `atom::type::FlatSet` is a flat set implementation using a sorted vector as the underlying container. It provides similar functionality to `std::set` but with potentially better cache locality and memory usage for small to medium-sized sets.

## Table of Contents

1. [Overview](#overview)
2. [Template Parameters](#template-parameters)
3. [Member Types](#member-types)
4. [Constructors](#constructors)
5. [Element Access](#element-access)
6. [Iterators](#iterators)
7. [Capacity](#capacity)
8. [Modifiers](#modifiers)
9. [Lookup](#lookup)
10. [Observers](#observers)
11. [Non-Member Functions](#non-member-functions)
12. [Usage Examples](#usage-examples)
13. [Performance Considerations](#performance-considerations)

## Overview

`FlatSet` is a container that stores unique elements in a sorted manner. Unlike `std::set`, which typically uses a balanced binary search tree, `FlatSet` uses a sorted vector as its underlying container. This can lead to better performance for smaller sets due to improved cache locality.

## Template Parameters

```cpp
template <typename T, typename Compare = std::less<T>>
```

- `T`: The type of elements in the set.
- `Compare`: The comparison function object type (default is `std::less<T>`).

## Member Types

- `value_type`: `T`
- `size_type`: `std::size_t`
- `difference_type`: `std::ptrdiff_t`
- `reference`: `value_type&`
- `const_reference`: `const value_type&`
- `pointer`: `value_type*`
- `const_pointer`: `const value_type*`
- `iterator`: Constant iterator type
- `const_iterator`: Constant iterator type
- `reverse_iterator`: Reverse iterator type
- `const_reverse_iterator`: Constant reverse iterator type
- `key_compare`: `Compare`
- `value_compare`: `Compare`

## Constructors

```cpp
FlatSet();
explicit FlatSet(const Compare& comp);
template <std::input_iterator InputIt>
FlatSet(InputIt first, InputIt last, const Compare& comp = Compare());
FlatSet(std::initializer_list<T> init, const Compare& comp = Compare());
FlatSet(const FlatSet& other);
FlatSet(FlatSet&& other) noexcept;
```

## Element Access

```cpp
iterator begin() noexcept;
const_iterator begin() const noexcept;
const_iterator cbegin() const noexcept;
iterator end() noexcept;
const_iterator end() const noexcept;
const_iterator cend() const noexcept;
reverse_iterator rbegin() noexcept;
const_reverse_iterator rbegin() const noexcept;
const_reverse_iterator crbegin() const noexcept;
reverse_iterator rend() noexcept;
const_reverse_iterator rend() const noexcept;
const_reverse_iterator crend() const noexcept;
```

## Capacity

```cpp
[[nodiscard]] bool empty() const noexcept;
[[nodiscard]] size_type size() const noexcept;
[[nodiscard]] size_type max_size() const noexcept;
```

## Modifiers

```cpp
void clear() noexcept;
std::pair<iterator, bool> insert(const T& value);
std::pair<iterator, bool> insert(T&& value);
iterator insert(const_iterator hint, const T& value);
iterator insert(const_iterator hint, T&& value);
template <std::input_iterator InputIt>
void insert(InputIt first, InputIt last);
void insert(std::initializer_list<T> ilist);
template <typename... Args>
std::pair<iterator, bool> emplace(Args&&... args);
template <typename... Args>
iterator emplace_hint(const_iterator hint, Args&&... args);
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
size_type erase(const T& value);
void swap(FlatSet& other) noexcept;
```

## Lookup

```cpp
size_type count(const T& value) const;
iterator find(const T& value);
const_iterator find(const T& value) const;
std::pair<iterator, iterator> equalRange(const T& value);
std::pair<const_iterator, const_iterator> equalRange(const T& value) const;
iterator lowerBound(const T& value);
const_iterator lowerBound(const T& value) const;
iterator upperBound(const T& value);
const_iterator upperBound(const T& value) const;
bool contains(const T& value) const;
```

## Observers

```cpp
key_compare keyComp() const;
value_compare valueComp() const;
```

## Non-Member Functions

```cpp
template <typename T, typename Compare>
bool operator==(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs);
template <typename T, typename Compare>
bool operator!=(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs);
template <typename T, typename Compare>
bool operator<(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs);
template <typename T, typename Compare>
bool operator<=(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs);
template <typename T, typename Compare>
bool operator>(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs);
template <typename T, typename Compare>
bool operator>=(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs);
template <typename T, typename Compare>
void swap(FlatSet<T, Compare>& lhs, FlatSet<T, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs)));
```

## Usage Examples

Here are some examples demonstrating how to use the `FlatSet` class:

```cpp
#include "flatset.hpp"
#include <iostream>
#include <string>

int main() {
    // Create a FlatSet of integers
    atom::type::FlatSet<int> intSet;

    // Insert elements
    intSet.insert(5);
    intSet.insert(2);
    intSet.insert(8);
    intSet.insert(1);
    intSet.insert(9);

    // Print the set
    std::cout << "Set contents: ";
    for (const auto& value : intSet) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    // Check if an element exists
    if (intSet.contains(5)) {
        std::cout << "5 is in the set" << std::endl;
    }

    // Find an element
    auto it = intSet.find(8);
    if (it != intSet.end()) {
        std::cout << "Found 8 in the set" << std::endl;
    }

    // Erase an element
    intSet.erase(2);

    // Create a FlatSet with a custom comparator
    auto stringComp = [](const std::string& a, const std::string& b) {
        return a.length() < b.length() || (a.length() == b.length() && a < b);
    };
    atom::type::FlatSet<std::string, decltype(stringComp)> stringSet(stringComp);

    // Insert elements
    stringSet.insert("apple");
    stringSet.insert("banana");
    stringSet.insert("cherry");
    stringSet.insert("date");

    // Print the set
    std::cout << "String set contents: ";
    for (const auto& value : stringSet) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    // Use emplace to construct elements in-place
}
```
