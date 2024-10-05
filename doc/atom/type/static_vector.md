# StaticVector Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Member Types](#member-types)
4. [Constructors](#constructors)
5. [Element Access](#element-access)
6. [Capacity](#capacity)
7. [Modifiers](#modifiers)
8. [Iterators](#iterators)
9. [Non-member Functions](#non-member-functions)
10. [Usage Examples](#usage-examples)

## Introduction

The `StaticVector` class is a custom implementation of a fixed-capacity vector in C++. It provides functionality similar to `std::vector` but with a predetermined maximum size, allowing for more efficient memory usage and potentially better performance in certain scenarios.

## Class Definition

```cpp
template <typename T, std::size_t Capacity>
    requires(Capacity > 0)
class StaticVector {
    // ... (implementation details)
};
```

## Member Types

- `value_type`: The type of elements stored in the vector (T)
- `size_type`: Unsigned integer type for sizes and indices (std::size_t)
- `difference_type`: Signed integer type for differences between iterators (std::ptrdiff_t)
- `reference`: Reference to value_type (value_type&)
- `const_reference`: Const reference to value_type (const value_type&)
- `pointer`: Pointer to value_type (value_type\*)
- `const_pointer`: Const pointer to value_type (const value_type\*)
- `iterator`: Random access iterator (pointer)
- `const_iterator`: Const random access iterator (const_pointer)
- `reverse_iterator`: Reverse iterator (std::reverse_iterator<iterator>)
- `const_reverse_iterator`: Const reverse iterator (std::reverse_iterator<const_iterator>)

## Constructors

1. Default constructor:

   ```cpp
   constexpr StaticVector() noexcept = default;
   ```

2. Initializer list constructor:

   ```cpp
   constexpr StaticVector(std::initializer_list<T> init) noexcept;
   ```

3. Copy constructor:

   ```cpp
   constexpr StaticVector(const StaticVector& other) noexcept = default;
   ```

4. Move constructor:

   ```cpp
   constexpr StaticVector(StaticVector&& other) noexcept = default;
   ```

5. Copy assignment operator:

   ```cpp
   constexpr auto operator=(const StaticVector& other) noexcept -> StaticVector& = default;
   ```

6. Move assignment operator:
   ```cpp
   constexpr auto operator=(StaticVector&& other) noexcept -> StaticVector& = default;
   ```

## Element Access

1. Access element without bounds checking:

   ```cpp
   [[nodiscard]] constexpr auto operator[](size_type index) noexcept -> reference;
   [[nodiscard]] constexpr auto operator[](size_type index) const noexcept -> const_reference;
   ```

2. Access element with bounds checking:

   ```cpp
   [[nodiscard]] constexpr auto at(size_type index) -> reference;
   [[nodiscard]] constexpr auto at(size_type index) const -> const_reference;
   ```

3. Access first element:

   ```cpp
   [[nodiscard]] constexpr auto front() noexcept -> reference;
   [[nodiscard]] constexpr auto front() const noexcept -> const_reference;
   ```

4. Access last element:

   ```cpp
   [[nodiscard]] constexpr auto back() noexcept -> reference;
   [[nodiscard]] constexpr auto back() const noexcept -> const_reference;
   ```

5. Access underlying array:
   ```cpp
   [[nodiscard]] constexpr auto data() noexcept -> pointer;
   [[nodiscard]] constexpr auto data() const noexcept -> const_pointer;
   ```

## Capacity

1. Check if the vector is empty:

   ```cpp
   [[nodiscard]] constexpr auto empty() const noexcept -> bool;
   ```

2. Get the number of elements:

   ```cpp
   [[nodiscard]] constexpr auto size() const noexcept -> size_type;
   ```

3. Get the capacity of the vector:
   ```cpp
   [[nodiscard]] constexpr auto capacity() const noexcept -> size_type;
   ```

## Modifiers

1. Add an element to the end:

   ```cpp
   constexpr void pushBack(const T& value) noexcept;
   constexpr void pushBack(T&& value) noexcept;
   ```

2. Construct an element in place at the end:

   ```cpp
   template <typename... Args>
   constexpr auto emplaceBack(Args&&... args) noexcept -> reference;
   ```

3. Remove the last element:

   ```cpp
   constexpr void popBack() noexcept;
   ```

4. Clear all elements:

   ```cpp
   constexpr void clear() noexcept;
   ```

5. Swap contents with another StaticVector:
   ```cpp
   constexpr void swap(StaticVector& other) noexcept;
   ```

## Iterators

1. Get iterator to the beginning:

   ```cpp
   [[nodiscard]] constexpr auto begin() noexcept -> iterator;
   [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator;
   [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator;
   ```

2. Get iterator to the end:

   ```cpp
   [[nodiscard]] constexpr auto end() noexcept -> iterator;
   [[nodiscard]] constexpr auto end() const noexcept -> const_iterator;
   [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator;
   ```

3. Get reverse iterator to the beginning:

   ```cpp
   [[nodiscard]] constexpr auto rbegin() noexcept -> reverse_iterator;
   [[nodiscard]] constexpr auto rbegin() const noexcept -> const_reverse_iterator;
   [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator;
   ```

4. Get reverse iterator to the end:
   ```cpp
   [[nodiscard]] constexpr auto rend() noexcept -> reverse_iterator;
   [[nodiscard]] constexpr auto rend() const noexcept -> const_reverse_iterator;
   [[nodiscard]] constexpr auto crend() const noexcept -> const_reverse_iterator;
   ```

## Non-member Functions

1. Equality operator:

   ```cpp
   template <typename T, std::size_t Capacity>
   constexpr auto operator==(const StaticVector<T, Capacity>& lhs,
                             const StaticVector<T, Capacity>& rhs) noexcept -> bool;
   ```

2. Three-way comparison operator:

   ```cpp
   template <typename T, std::size_t Capacity>
   constexpr auto operator<=>(const StaticVector<T, Capacity>& lhs,
                              const StaticVector<T, Capacity>& rhs) noexcept;
   ```

3. Swap function:
   ```cpp
   template <typename T, std::size_t Capacity>
   constexpr void swap(StaticVector<T, Capacity>& lhs,
                       StaticVector<T, Capacity>& rhs) noexcept;
   ```

## Usage Examples

Here are some examples demonstrating the usage of the `StaticVector` class:

```cpp
#include "static_vector.hpp"
#include <iostream>
#include <string>
#include <algorithm>

int main() {
    // Create a StaticVector of integers with a capacity of 5
    StaticVector<int, 5> intVector;

    // Add elements to the vector
    intVector.pushBack(1);
    intVector.pushBack(2);
    intVector.pushBack(3);

    // Print the vector
    std::cout << "Vector contents: ";
    for (const auto& value : intVector) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    // Use initializer list constructor
    StaticVector<std::string, 4> stringVector {"apple", "banana", "cherry"};

    // Add another element
    stringVector.emplaceBack("date");

    // Print the string vector
    std::cout << "String vector: ";
    for (const auto& str : stringVector) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    // Demonstrate capacity and size
    std::cout << "Capacity: " << stringVector.capacity() << std::endl;
    std::cout << "Size: " << stringVector.size() << std::endl;

    // Access elements
    std::cout << "First element: " << stringVector.front() << std::endl;
    std::cout << "Last element: " << stringVector.back() << std::endl;
    std::cout << "Element at index 2: " << stringVector.at(2) << std::endl;

    // Modify an element
    stringVector[1] = "blueberry";

    // Remove the last element
    stringVector.popBack();

    // Print the modified vector
    std::cout << "Modified string vector: ";
    for (const auto& str : stringVector) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    // Demonstrate iterators
    std::cout << "Reverse order: ";
    for (auto it = stringVector.rbegin(); it != stringVector.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Using algorithms with StaticVector
    StaticVector<int, 10> numbers {5, 2, 8, 1, 9};
    std::sort(numbers.begin(), numbers.end());

    std::cout << "Sorted numbers: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Demonstrating comparison operators
    StaticVector<int, 5> vec1 {1, 2, 3};
    StaticVector<int, 5> vec2 {1, 2, 3};
    StaticVector<int, 5> vec3 {1, 2, 3, 4};

    std::cout << "vec1 == vec2: " << (vec1 == vec2) << std::endl;
    std::cout << "vec1 < vec3: " << (vec1 < vec3) << std::endl;

    // Demonstrating swap
    swap(vec1, vec3);
    std::cout << "After swap, vec1 size: " << vec1.size() << ", vec3 size: " << vec3.size() << std::endl;

    // Trying to add more elements than capacity
    try {
        for (int i = 0; i < 6; ++i) {
            vec1.pushBack(i);
        }
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    return 0;
}
```

This example demonstrates:

1. Creating `StaticVector` objects with different types and capacities.
2. Adding elements using `pushBack()` and `emplaceBack()`.
3. Initializing a `StaticVector` with an initializer list.
4. Accessing elements using `front()`, `back()`, `at()`, and the subscript operator `[]`.
5. Checking capacity and size.
6. Modifying elements.
7. Removing elements using `popBack()`.
8. Using iterators, including reverse iterators.
9. Using `StaticVector` with standard algorithms like `std::sort`.
10. Comparing `StaticVector` objects using comparison operators.
11. Swapping the contents of two `StaticVector` objects.
12. Handling the case when trying to add more elements than the vector's capacity.

Best Practices and Considerations:

1. Choose the appropriate capacity: When using `StaticVector`, carefully consider the maximum number of elements you expect to store. This helps optimize memory usage and performance.

2. Use `at()` for bounds-checked access: When you need bounds checking, use the `at()` method instead of the subscript operator `[]`.

3. Prefer `emplaceBack()` over `pushBack()`: When possible, use `emplaceBack()` to construct elements in place, potentially avoiding unnecessary copies or moves.

4. Check capacity before adding elements: If you're unsure whether the vector has enough capacity, check `size()` against `capacity()` before adding new elements to avoid out-of-range errors.

5. Use with standard algorithms: `StaticVector` is compatible with standard library algorithms, so take advantage of this for operations like sorting, searching, or transforming data.

6. Consider using `StaticVector` for performance-critical code: In scenarios where you need to avoid dynamic memory allocation and know the maximum size upfront, `StaticVector` can offer better performance than `std::vector`.

7. Be aware of the fixed capacity: Unlike `std::vector`, `StaticVector` cannot grow beyond its initial capacity. Design your code with this limitation in mind.

The `StaticVector` class provides a safe and efficient way to work with fixed-capacity vectors, offering many of the benefits of `std::vector` while avoiding dynamic memory allocation. It's particularly useful in embedded systems, real-time applications, or any scenario where memory usage needs to be predictable and constrained.
