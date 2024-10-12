# cstream Class Documentation

The `cstream` class is a part of the `atom::type` namespace and provides a stream-like interface for performing operations on containers. It offers a wide range of functionalities for manipulating and analyzing data in containers.

## Table of Contents

1. [Overview](#overview)
2. [Class Definition](#class-definition)
3. [Constructor](#constructor)
4. [Basic Operations](#basic-operations)
5. [Container Transformations](#container-transformations)
6. [Element Access and Information](#element-access-and-information)
7. [Aggregation Operations](#aggregation-operations)
8. [Utility Functions](#utility-functions)
9. [Helper Structures](#helper-structures)
10. [Usage Examples](#usage-examples)

## Overview

The `cstream` class provides a fluent interface for performing various operations on containers. It supports operations like sorting, filtering, transforming, and aggregating data. The class is designed to work with any container type that supports standard C++ container operations.

## Class Definition

```cpp
template <typename C>
class cstream {
public:
    using value_type = typename C::value_type;
    using it_type = decltype(std::begin(C{}));

    // ... (member functions)
};
```

## Constructor

The `cstream` class can be constructed in two ways:

1. From a container reference:

   ```cpp
   explicit cstream(C& c);
   ```

2. From an rvalue container:

   ```cpp
   explicit cstream(C&& c);
   ```

## Basic Operations

### getRef()

Returns a reference to the underlying container.

### getMove()

Moves the container out of the stream.

### get()

Returns a copy of the container.

### operator C&&()

Converts the stream to an rvalue container.

## Container Transformations

### sorted()

Sorts the container in ascending order.

```cpp
template <typename BinaryFunction = std::less<value_type>>
auto sorted(const BinaryFunction& op = {}) -> cstream<C>&;
```

### transform()

Transforms the container using a function.

```cpp
template <typename T, typename UnaryFunction>
auto transform(UnaryFunction transform_f) const -> cstream<T>;
```

### remove()

Removes elements from the container based on a predicate.

```cpp
template <typename UnaryFunction>
auto remove(UnaryFunction remove_f) -> cstream<C>&;
```

### erase()

Erases a specific value from the container.

```cpp
template <typename ValueType>
auto erase(const ValueType& v) -> cstream<C>&;
```

### filter()

Filters the container based on a predicate.

```cpp
template <typename UnaryFunction>
auto filter(UnaryFunction filter) -> cstream<C>&;
```

### cpFilter()

Creates a copy of the container and filters it based on a predicate.

```cpp
template <typename UnaryFunction>
auto cpFilter(UnaryFunction filter) const -> cstream<C>;
```

### map()

Maps the elements of the container using a function.

```cpp
template <typename UnaryFunction>
auto map(UnaryFunction f) const -> cstream<C>;
```

### flatMap()

Flat maps the elements of the container using a function.

```cpp
template <typename UnaryFunction>
auto flatMap(UnaryFunction f) const -> cstream<C>;
```

### distinct()

Removes duplicate elements from the container.

```cpp
auto distinct() -> cstream<C>&;
```

### reverse()

Reverses the elements of the container.

```cpp
auto reverse() -> cstream<C>&;
```

## Element Access and Information

### size()

Returns the size of the container.

```cpp
[[nodiscard]] auto size() const -> std::size_t;
```

### count()

Counts the number of elements that satisfy a predicate or the number of occurrences of a value.

```cpp
template <typename UnaryFunction>
auto count(UnaryFunction f) const -> std::size_t;

auto count(const value_type& v) const -> std::size_t;
```

### contains()

Checks if the container contains a value.

```cpp
auto contains(const value_type& value) const -> bool;
```

### first()

Gets the first element in the container or the first element that satisfies a predicate.

```cpp
auto first() const -> std::optional<value_type>;

template <typename UnaryFunction>
auto first(UnaryFunction f) const -> std::optional<value_type>;
```

## Aggregation Operations

### accumulate()

Accumulates the elements of the container using a binary function.

```cpp
template <typename UnaryFunction = std::plus<value_type>>
auto accumulate(value_type initial = {}, UnaryFunction op = {}) const -> value_type;
```

### forEach()

Applies a function to each element of the container.

```cpp
template <typename UnaryFunction>
auto forEach(UnaryFunction f) -> cstream<C>&;
```

### all()

Checks if all elements satisfy a predicate.

```cpp
template <typename UnaryFunction>
auto all(UnaryFunction f) const -> bool;
```

### any()

Checks if any element satisfies a predicate.

```cpp
template <typename UnaryFunction>
auto any(UnaryFunction f) const -> bool;
```

### none()

Checks if no elements satisfy a predicate.

```cpp
template <typename UnaryFunction>
auto none(UnaryFunction f) const -> bool;
```

### min()

Gets the minimum element in the container.

```cpp
auto min() const -> value_type;
```

### max()

Gets the maximum element in the container.

```cpp
auto max() const -> value_type;
```

### mean()

Calculates the mean of the elements in the container.

```cpp
[[nodiscard]] auto mean() const -> double;
```

## Utility Functions

### copy()

Creates a copy of the container.

```cpp
auto copy() const -> cstream<C>;
```

## Helper Structures

### ContainerAccumulate

A functor for accumulating containers.

### identity

A functor that returns the input value.

### JoinAccumulate

A functor for joining containers with a separator.

### Pair

A utility struct for working with pairs.

## Usage Examples

Here are some examples demonstrating how to use the `cstream` class:

```cpp
#include "cstream.hpp"
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<int> numbers = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    // Example 1: Basic operations
    auto result = atom::type::makeStream(numbers)
        .sorted()
        .filter([](int n) { return n % 2 == 0; })
        .transform<std::vector<std::string>>([](int n) { return std::to_string(n); })
        .get();

    std::cout << "Even numbers sorted and converted to strings:" << std::endl;
    for (const auto& s : result) {
        std::cout << s << " ";
    }
    std::cout << std::endl;

    // Example 2: Aggregation operations
    auto sum = atom::type::makeStream(numbers)
        .accumulate();

    auto product = atom::type::makeStream(numbers)
        .accumulate(1, std::multiplies<int>());

    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Product: " << product << std::endl;

    // Example 3: Element access and information
    auto firstEven = atom::type::makeStream(numbers)
        .first([](int n) { return n % 2 == 0; });

    std::cout << "First even number: " << (firstEven ? std::to_string(*firstEven) : "Not found") << std::endl;

    auto containsSeven = atom::type::makeStream(numbers)
        .contains(7);

    std::cout << "Contains 7: " << (containsSeven ? "Yes" : "No") << std::endl;

    // Example 4: Map and flatMap
    std::vector<std::vector<int>> nestedNumbers = {{1, 2}, {3, 4}, {5, 6}};

    auto mappedResult = atom::type::makeStream(nestedNumbers)
        .map([](const std::vector<int>& v) { return v.size(); })
        .get();

    std::cout << "Sizes of nested vectors:" << std::endl;
    for (const auto& size : mappedResult) {
        std::cout << size << " ";
    }
    std::cout << std::endl;

    auto flatMappedResult = atom::type::makeStream(nestedNumbers)
        .flatMap([](const std::vector<int>& v) { return v; })
        .get();

    std::cout << "Flattened nested vectors:" << std::endl;
    for (const auto& num : flatMappedResult) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Example 5: Distinct and reverse
    std::vector<int> duplicates = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};

    auto distinctResult = atom::type::makeStream(duplicates)
        .distinct()
        .reverse()
        .get();

    std::cout << "Distinct elements in reverse order:" << std::endl;
    for (const auto& num : distinctResult) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Example 6: Custom sorting
    std::vector<std::pair<std::string, int>> pairs = {{"apple", 5}, {"banana", 2}, {"cherry", 8}};

    auto sortedPairs = atom::type::makeStream(pairs)
        .sorted([](const auto& a, const auto& b) { return a.second < b.second; })
        .get();

    std::cout << "Pairs sorted by second element:" << std::endl;
    for (const auto& [fruit, count] : sortedPairs) {
        std::cout << fruit << ": " << count << std::endl;
    }

    // Example 7: Chaining multiple operations
    auto complexResult = atom::type::makeStream(numbers)
        .filter([](int n) { return n > 3; })
        .map([](int n) { return n * n; })
        .sorted(std::greater<int>())
        .take(3)
        .get();

    std::cout << "Top 3 squares of numbers greater than 3:" << std::endl;
    for (const auto& num : complexResult) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Example 8: Using utility functions
    std::vector<int> source = {1, 2, 3, 4, 5};
    auto streamCopy = atom::type::makeStreamCopy(source);
    streamCopy.forEach([](int& n) { n *= 2; });

    std::cout << "Original source:" << std::endl;
    for (const auto& num : source) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    std::cout << "Modified copy:" << std::endl;
    for (const auto& num : streamCopy.get()) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

This extended example demonstrates more advanced usage of the `cstream` class. Let's break down each example:

1. **Map and flatMap**: These operations allow you to transform the elements of the container. `map` applies a function to each element, while `flatMap` applies a function that returns a container and flattens the result.

2. **Distinct and reverse**: The `distinct` operation removes duplicate elements, and `reverse` reverses the order of elements in the container.

3. **Custom sorting**: You can provide a custom comparison function to `sorted` for more complex sorting logic.

4. **Chaining multiple operations**: The fluent interface of `cstream` allows you to chain multiple operations together, creating complex data processing pipelines in a readable manner.

5. **Using utility functions**: The `makeStreamCopy` function creates a new stream with a copy of the original container, allowing you to modify the copy without affecting the original.

When using the `cstream` class, keep in mind the following tips:

- The operations are generally lazy, meaning they are only executed when you call a terminal operation like `get()` or `accumulate()`.
- Some operations (like `sorted()`, `distinct()`, and `reverse()`) modify the underlying container, while others (like `filter()` and `map()`) create a new container.
- When chaining operations, consider the order of operations for efficiency. For example, filtering before mapping can reduce the number of mapping operations needed.
- The `cstream` class works with any container type that supports standard C++ container operations, making it very versatile.

By leveraging the power of `cstream`, you can write more expressive and functional-style code for container operations, improving readability and reducing the likelihood of errors in complex data processing tasks.
