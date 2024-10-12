# SmallVector Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Member Types](#member-types)
4. [Constructors and Destructor](#constructors-and-destructor)
5. [Element Access](#element-access)
6. [Iterators](#iterators)
7. [Capacity](#capacity)
8. [Modifiers](#modifiers)
9. [Non-member Functions](#non-member-functions)
10. [Usage Examples](#usage-examples)

## Introduction

The `SmallVector` class is a custom implementation of a vector-like container that uses a small buffer optimization. It provides functionality similar to `std::vector` but is optimized for small sizes, potentially avoiding heap allocations for small numbers of elements.

## Class Definition

```cpp
template <typename T, std::size_t N>
class SmallVector {
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

## Constructors and Destructor

1. Default constructor:

   ```cpp
   SmallVector() = default;
   ```

2. Range constructor:

   ```cpp
   template <std::input_iterator InputIt>
   SmallVector(InputIt first, InputIt last);
   ```

3. Fill constructor:

   ```cpp
   explicit SmallVector(size_type count, const T& value = T());
   ```

4. Initializer list constructor:

   ```cpp
   SmallVector(std::initializer_list<T> init);
   ```

5. Copy constructor:

   ```cpp
   SmallVector(const SmallVector& other);
   ```

6. Move constructor:

   ```cpp
   SmallVector(SmallVector&& other) ATOM_NOEXCEPT;
   ```

7. Destructor:

   ```cpp
   ~SmallVector();
   ```

8. Copy assignment operator:

   ```cpp
   auto operator=(const SmallVector& other) -> SmallVector&;
   ```

9. Move assignment operator:

   ```cpp
   auto operator=(SmallVector&& other) ATOM_NOEXCEPT -> SmallVector&;
   ```

10. Initializer list assignment operator:
    ```cpp
    auto operator=(std::initializer_list<T> init) -> SmallVector&;
    ```

## Element Access

1. Access element with bounds checking:

   ```cpp
   auto at(size_type pos) -> reference;
   auto at(size_type pos) const -> const_reference;
   ```

2. Access element without bounds checking:

   ```cpp
   auto operator[](size_type pos) -> reference;
   auto operator[](size_type pos) const -> const_reference;
   ```

3. Access first element:

   ```cpp
   auto front() -> reference;
   auto front() const -> const_reference;
   ```

4. Access last element:

   ```cpp
   auto back() -> reference;
   auto back() const -> const_reference;
   ```

5. Access underlying array:
   ```cpp
   auto data() ATOM_NOEXCEPT -> T*;
   auto data() const ATOM_NOEXCEPT -> const T*;
   ```

## Iterators

1. Get iterator to the beginning:

   ```cpp
   auto begin() ATOM_NOEXCEPT -> iterator;
   auto begin() const ATOM_NOEXCEPT -> const_iterator;
   auto cbegin() const ATOM_NOEXCEPT -> const_iterator;
   ```

2. Get iterator to the end:

   ```cpp
   auto end() ATOM_NOEXCEPT -> iterator;
   auto end() const ATOM_NOEXCEPT -> const_iterator;
   auto cend() const ATOM_NOEXCEPT -> const_iterator;
   ```

3. Get reverse iterator to the beginning:

   ```cpp
   auto rbegin() ATOM_NOEXCEPT -> reverse_iterator;
   auto rbegin() const ATOM_NOEXCEPT -> const_reverse_iterator;
   auto crbegin() const ATOM_NOEXCEPT -> const_reverse_iterator;
   ```

4. Get reverse iterator to the end:
   ```cpp
   auto rend() ATOM_NOEXCEPT -> reverse_iterator;
   auto rend() const ATOM_NOEXCEPT -> const_reverse_iterator;
   auto crend() const ATOM_NOEXCEPT -> const_reverse_iterator;
   ```

## Capacity

1. Check if the vector is empty:

   ```cpp
   ATOM_NODISCARD auto empty() const ATOM_NOEXCEPT -> bool;
   ```

2. Get the number of elements:

   ```cpp
   ATOM_NODISCARD auto size() const ATOM_NOEXCEPT -> size_type;
   ```

3. Get the maximum possible number of elements:

   ```cpp
   ATOM_NODISCARD auto maxSize() const ATOM_NOEXCEPT -> size_type;
   ```

4. Reserve capacity:

   ```cpp
   void reserve(size_type new_cap);
   ```

5. Get the current capacity:
   ```cpp
   ATOM_NODISCARD auto capacity() const ATOM_NOEXCEPT -> size_type;
   ```

## Modifiers

1. Clear the contents:

   ```cpp
   void clear() ATOM_NOEXCEPT;
   ```

2. Insert elements:

   ```cpp
   auto insert(const_iterator pos, const T& value) -> iterator;
   auto insert(const_iterator pos, T&& value) -> iterator;
   auto insert(const_iterator pos, size_type count, const T& value) -> iterator;
   template <std::input_iterator InputIt>
   auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator;
   auto insert(const_iterator pos, std::initializer_list<T> init) -> iterator;
   ```

3. Emplace element:

   ```cpp
   template <typename... Args>
   auto emplace(const_iterator pos, Args&&... args) -> iterator;
   ```

4. Erase elements:

   ```cpp
   auto erase(const_iterator pos) -> iterator;
   auto erase(const_iterator first, const_iterator last) -> iterator;
   ```

5. Add element to the end:

   ```cpp
   void pushBack(const T& value);
   void pushBack(T&& value);
   ```

6. Construct element in-place at the end:

   ```cpp
   template <typename... Args>
   auto emplaceBack(Args&&... args) -> reference;
   ```

7. Remove the last element:

   ```cpp
   void popBack();
   ```

8. Resize the vector:

   ```cpp
   void resize(size_type count, const T& value = T());
   ```

9. Swap contents with another vector:
   ```cpp
   void swap(SmallVector& other) ATOM_NOEXCEPT;
   ```

## Non-member Functions

1. Comparison operators:

   ```cpp
   template <typename T, std::size_t N>
   auto operator==(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) -> bool;
   ```

2. Relational operators:

   ```cpp
   template <typename T, std::size_t N>
   auto operator<(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) -> bool;

   template <typename T, std::size_t N>
   auto operator<=(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) -> bool;

   template <typename T, std::size_t N>
   auto operator>(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) -> bool;

   template <typename T, std::size_t N>
   auto operator>=(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) -> bool;
   ```

3. Non-member swap:
   ```cpp
   template <typename T, std::size_t N>
   void swap(SmallVector<T, N>& lhs, SmallVector<T, N>& rhs) ATOM_NOEXCEPT;
   ```

## Usage Examples

Here are some examples demonstrating the usage of the `SmallVector` class:

```cpp
#include "small_vector.hpp"
#include <iostream>
#include <string>

int main() {
    // Create a SmallVector of integers with a small buffer size of 5
    SmallVector<int, 5> intVector;

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
    SmallVector<std::string, 3> stringVector {"apple", "banana", "cherry"};

    // Insert an element
    auto it = stringVector.begin();
    ++it;
    stringVector.insert(it, "blueberry");

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

    // Resize the vector
    stringVector.resize(6, "new");
    std::cout << "After resize: ";
    for (const auto& str : stringVector) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    // Demonstrate emplace_back
    stringVector.emplaceBack("emplace");
    std::cout << "After emplace_back: " << stringVector.back() << std::endl;

    // Erase an element
    stringVector.erase(stringVector.begin() + 2);
    std::cout << "After erase: ";
    for (const auto& str : stringVector) {
        std::cout << str << " ";
    }
    std::cout << std::endl;

    // Demonstrate move semantics
    SmallVector<std::string, 3> movedVector = std::move(stringVector);
    std::cout << "Moved vector size: " << movedVector.size() << std::endl;
    std::cout << "Original vector size: " << stringVector.size() << std::endl;

    // Demonstrate comparison operators
    SmallVector<int, 5> vec1 {1, 2, 3};
    SmallVector<int, 5> vec2 {1, 2, 3};
    SmallVector<int, 5> vec3 {1, 2, 3, 4};

    std::cout << "vec1 == vec2: " << (vec1 == vec2) << std::endl;
    std::cout << "vec1 < vec3: " << (vec1 < vec3) << std::endl;

    // Demonstrate swap
    swap(vec1, vec3);
    std::cout << "After swap, vec1 size: " << vec1.size() << ", vec3 size: " << vec3.size() << std::endl;

    return 0;
}
```

This example demonstrates:

1. Creating `SmallVector` objects with different types and small buffer sizes.
2. Adding elements using `pushBack()` and initializer lists.
3. Inserting elements at specific positions.
4. Accessing elements using `front()`, `back()`, and `at()`.
5. Checking capacity and size.
6. Resizing the vector.
7. Using `emplaceBack()` for in-place construction.
8. Erasing elements.
9. Moving vectors using move semantics.
10. Comparing vectors using comparison operators.
11. Swapping vectors.

The `SmallVector` class provides a flexible and efficient alternative to `std::vector` for cases where the number of elements is expected to be small most of the time. It avoids heap allocations for small sizes while still allowing growth beyond the small buffer size when needed.

When using `SmallVector`, consider the expected size of your data to choose an appropriate small buffer size (`N` template parameter). This can help optimize performance and memory usage for your specific use case.

```

```
