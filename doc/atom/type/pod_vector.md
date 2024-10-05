# PodVector Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Member Types](#member-types)
4. [Constructors](#constructors)
5. [Capacity](#capacity)
6. [Element Access](#element-access)
7. [Iterators](#iterators)
8. [Modifiers](#modifiers)
9. [Other Member Functions](#other-member-functions)
10. [Examples](#examples)

## Introduction

The `PodVector` class is a custom implementation of a dynamic array specifically designed for POD (Plain Old Data) types. It provides similar functionality to `std::vector` but is optimized for POD types, offering better performance in certain scenarios.

## Class Definition

```cpp
namespace atom::type {

template <PodType T, int Growth = 2>
class PodVector {
    // ... (implementation details)
};

}  // namespace atom::type
```

The `PodVector` class is defined in the `atom::type` namespace and takes two template parameters:

- `T`: The type of elements stored in the vector (must satisfy the `PodType` concept)
- `Growth`: The growth factor for capacity expansion (default is 2)

## Member Types

- `size_type`: Alias for `int`, used to represent sizes and indices
- `iterator`: Random access iterator for `PodVector`
- `const_iterator`: Const random access iterator for `PodVector`

## Constructors

1. Default constructor:

   ```cpp
   constexpr PodVector() noexcept = default;
   ```

2. Initializer list constructor:

   ```cpp
   constexpr PodVector(std::initializer_list<T> il);
   ```

3. Size constructor:

   ```cpp
   explicit constexpr PodVector(int size);
   ```

4. Copy constructor:

   ```cpp
   PodVector(const PodVector& other);
   ```

5. Move constructor:

   ```cpp
   PodVector(PodVector&& other) noexcept;
   ```

## Capacity

- `empty()`: Check if the vector is empty
- `size()`: Get the number of elements
- `capacity()`: Get the current capacity
- `reserve(int cap)`: Reserve capacity for at least `cap` elements
- `resize(int size)`: Resize the vector to contain `size` elements

## Element Access

- `operator[](int index)`: Access element at specified index
- `back()`: Access the last element
- `data()`: Get a pointer to the underlying array

## Iterators

- `begin()`: Return an iterator to the beginning
- `end()`: Return an iterator to the end
- `begin() const`: Return a const_iterator to the beginning
- `end() const`: Return a const_iterator to the end

## Modifiers

- `pushBack(ValueT&& t)`: Add an element to the end
- `emplaceBack(Args&&... args)`: Construct an element in-place at the end
- `popBack()`: Remove the last element
- `popxBack()`: Remove and return the last element
- `extend(const PodVector& other)`: Append elements from another PodVector
- `extend(const T* begin, const T* end)`: Append elements from a range
- `insert(int i, ValueT&& val)`: Insert an element at a specified position
- `erase(int i)`: Remove an element at a specified position
- `clear()`: Remove all elements
- `reverse()`: Reverse the order of elements

## Other Member Functions

- `detach()`: Detach the underlying array from the vector

## Examples

Here are some examples demonstrating the usage of the `PodVector` class:

```cpp
#include "pod_vector.hpp"
#include <iostream>

int main() {
    // Create a PodVector of integers
    atom::type::PodVector<int> vec;

    // Add elements
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);

    // Use initializer list constructor
    atom::type::PodVector<int> vec2 = {4, 5, 6};

    // Extend vec with vec2
    vec.extend(vec2);

    // Print elements
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // Access elements
    std::cout << "First element: " << vec[0] << std::endl;
    std::cout << "Last element: " << vec.back() << std::endl;

    // Modify elements
    vec[0] = 10;

    // Insert and erase
    vec.insert(2, 100);
    vec.erase(4);

    // Reverse the vector
    vec.reverse();

    // Print elements after modifications
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // Use with custom POD type
    struct Point {
        int x;
        int y;
    };

    atom::type::PodVector<Point> points;
    points.emplaceBack(1, 2);
    points.emplaceBack(3, 4);

    for (const auto& point : points) {
        std::cout << "(" << point.x << ", " << point.y << ") ";
    }
    std::cout << std::endl;

    return 0;
}
```
