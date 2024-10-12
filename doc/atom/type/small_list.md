# SmallList Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Constructors and Destructor](#constructors-and-destructor)
4. [Element Access](#element-access)
5. [Capacity](#capacity)
6. [Modifiers](#modifiers)
7. [Operations](#operations)
8. [Iterators](#iterators)
9. [Usage Examples](#usage-examples)

## Introduction

The `SmallList` class is a custom implementation of a doubly linked list in C++. It provides functionality similar to `std::list` but is designed to be more lightweight and efficient for small collections of elements.

## Class Definition

```cpp
namespace atom::type {

template <typename T>
class SmallList {
    // ... (implementation details)
};

}  // namespace atom::type
```

## Constructors and Destructor

1. Default constructor:

   ```cpp
   constexpr SmallList() noexcept;
   ```

2. Initializer list constructor:

   ```cpp
   constexpr SmallList(std::initializer_list<T> init_list);
   ```

3. Copy constructor:

   ```cpp
   SmallList(const SmallList& other);
   ```

4. Move constructor:

   ```cpp
   SmallList(SmallList&& other) noexcept;
   ```

5. Copy assignment operator:

   ```cpp
   SmallList& operator=(SmallList other) noexcept;
   ```

6. Destructor:
   ```cpp
   ~SmallList() noexcept;
   ```

## Element Access

1. Access the first element:

   ```cpp
   constexpr T& front() noexcept;
   constexpr const T& front() const noexcept;
   ```

2. Access the last element:
   ```cpp
   constexpr T& back() noexcept;
   constexpr const T& back() const noexcept;
   ```

## Capacity

1. Check if the list is empty:

   ```cpp
   ATOM_NODISCARD constexpr bool empty() const noexcept;
   ```

2. Get the number of elements:
   ```cpp
   ATOM_NODISCARD constexpr size_t size() const noexcept;
   ```

## Modifiers

1. Add an element to the end:

   ```cpp
   constexpr void pushBack(const T& value);
   ```

2. Add an element to the front:

   ```cpp
   constexpr void pushFront(const T& value);
   ```

3. Remove the last element:

   ```cpp
   constexpr void popBack() noexcept;
   ```

4. Remove the first element:

   ```cpp
   constexpr void popFront() noexcept;
   ```

5. Insert an element at a specific position:

   ```cpp
   void insert(Iterator pos, const T& value);
   ```

6. Erase an element at a specific position:

   ```cpp
   Iterator erase(Iterator pos) noexcept;
   ```

7. Clear all elements:

   ```cpp
   constexpr void clear() noexcept;
   ```

8. Emplace an element at the end:

   ```cpp
   template <typename... Args>
   void emplaceBack(Args&&... args);
   ```

9. Emplace an element at the front:

   ```cpp
   template <typename... Args>
   void emplaceFront(Args&&... args);
   ```

10. Emplace an element at a specific position:
    ```cpp
    template <typename... Args>
    Iterator emplace(Iterator pos, Args&&... args);
    ```

## Operations

1. Remove all elements with a specific value:

   ```cpp
   void remove(const T& value);
   ```

2. Remove consecutive duplicate elements:

   ```cpp
   void unique();
   ```

3. Sort the elements:

   ```cpp
   void sort();
   ```

4. Swap contents with another list:
   ```cpp
   void swap(SmallList<T>& other) noexcept;
   ```

## Iterators

1. Get an iterator to the beginning:

   ```cpp
   Iterator begin() noexcept;
   ```

2. Get an iterator to the end:

   ```cpp
   Iterator end() noexcept;
   ```

3. Get a reverse iterator to the beginning:

   ```cpp
   ReverseIterator rbegin() noexcept;
   ```

4. Get a reverse iterator to the end:
   ```cpp
   ReverseIterator rend() noexcept;
   ```

## Usage Examples

Here are some examples demonstrating the usage of the `SmallList` class:

```cpp
#include "small_list.hpp"
#include <iostream>
#include <string>

int main() {
    // Create a SmallList of integers
    atom::type::SmallList<int> intList;

    // Add elements to the list
    intList.pushBack(1);
    intList.pushBack(2);
    intList.pushFront(0);

    // Print the list
    std::cout << "List contents: ";
    for (const auto& value : intList) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    // Access elements
    std::cout << "First element: " << intList.front() << std::endl;
    std::cout << "Last element: " << intList.back() << std::endl;

    // Use initializer list constructor
    atom::type::SmallList<std::string> stringList {"apple", "banana", "cherry"};

    // Insert an element
    auto it = stringList.begin();
    ++it;
    stringList.insert(it, "blueberry");

    // Print the string list
    std::cout << "String list: ";
    for (const auto& str : stringList) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    // Remove an element
    stringList.remove("banana");

    // Sort the list
    stringList.sort();

    // Print the sorted list
    std::cout << "Sorted string list: ";
    for (const auto& str : stringList) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    // Demonstrate unique() function
    atom::type::SmallList<int> duplicateList {1, 2, 2, 3, 3, 3, 4, 5, 5};
    duplicateList.unique();

    std::cout << "List after removing consecutive duplicates: ";
    for (const auto& value : duplicateList) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    // Demonstrate emplace operations
    atom::type::SmallList<std::pair<int, std::string>> pairList;
    pairList.emplaceBack(1, "one");
    pairList.emplaceFront(0, "zero");
    auto pairIt = pairList.begin();
    ++pairIt;
    pairList.emplace(pairIt, 2, "two");

    std::cout << "Pair list: ";
    for (const auto& pair : pairList) {
        std::cout << "(" << pair.first << ", " << pair.second << ") ";
    }
    std::cout << std::endl;

    return 0;
}
```

This example demonstrates:

1. Creating `SmallList` objects with different types (int, string, and pair).
2. Adding elements using `pushBack()`, `pushFront()`, and initializer lists.
3. Accessing elements using `front()` and `back()`.
4. Iterating through the list using range-based for loops.
5. Inserting and removing elements.
6. Sorting the list.
7. Using the `unique()` function to remove consecutive duplicates.
8. Using emplace operations (`emplaceBack()`, `emplaceFront()`, and `emplace()`) to construct elements in place.
