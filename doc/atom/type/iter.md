# Iterator Classes Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [PointerIterator](#pointeriterator)
3. [EarlyIncIterator](#earlyinciterator)
4. [TransformIterator](#transformiterator)
5. [FilterIterator](#filteriterator)
6. [ReverseIterator](#reverseiterator)
7. [ZipIterator](#zipiterator)

## Introduction

This document provides detailed information about various iterator classes implemented in the `iter.hpp` file. These iterators extend the functionality of standard C++ iterators, offering additional features such as pointer dereferencing, early increment, transformation, filtering, reversal, and zipping multiple iterators.

## PointerIterator

### Description

`PointerIterator` is an iterator that returns pointers to the elements of another iterator instead of the elements themselves.

### Class Definition

```cpp
template <typename IteratorT>
class PointerIterator {
    // ... (implementation details)
};
```

### Usage

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
auto [begin, end] = makePointerRange(vec.begin(), vec.end());

for (auto it = begin; it != end; ++it) {
    std::cout << **it << " ";  // Dereference twice to get the value
}
// Output: 1 2 3 4 5
```

### Key Methods

- `operator*()`: Returns a pointer to the element.
- `operator++()`: Pre-increment operator.
- `operator++(int)`: Post-increment operator.
- `operator==()` and `operator!=()`: Comparison operators.

### Helper Function

```cpp
template <typename IteratorT>
auto makePointerRange(IteratorT begin, IteratorT end);
```

This function creates a pair of `PointerIterator`s representing a range.

## EarlyIncIterator

### Description

`EarlyIncIterator` is an iterator that increments the underlying iterator early in its operations.

### Class Definition

```cpp
template <std::input_or_output_iterator I>
class EarlyIncIterator {
    // ... (implementation details)
};
```

### Usage

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
auto it = makeEarlyIncIterator(vec.begin());

std::cout << *it << " ";  // Prints 1
++it;  // Moves to 2
std::cout << *it << " ";  // Prints 2
// Output: 1 2
```

### Key Methods

- `operator*()`: Returns the value pointed to by the underlying iterator.
- `operator++()`: Pre-increment operator.
- `operator++(int)`: Post-increment operator.
- `operator==()` and `operator!=()`: Comparison operators.

### Helper Function

```cpp
template <std::input_or_output_iterator I>
auto makeEarlyIncIterator(I iterator);
```

This function creates an `EarlyIncIterator` from an underlying iterator.

## TransformIterator

### Description

`TransformIterator` applies a transformation function to the elements of the underlying iterator.

### Class Definition

```cpp
template <typename IteratorT, typename FuncT>
class TransformIterator {
    // ... (implementation details)
};
```

### Usage

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
auto square = [](int x) { return x * x; };
auto it = makeTransformIterator(vec.begin(), square);

for (int i = 0; i < 5; ++i, ++it) {
    std::cout << *it << " ";
}
// Output: 1 4 9 16 25
```

### Key Methods

- `operator*()`: Returns the transformed value.
- `operator->()`: Returns a pointer to the transformed value.
- `operator++()`: Pre-increment operator.
- `operator++(int)`: Post-increment operator.
- `operator==()` and `operator!=()`: Comparison operators.

### Helper Function

```cpp
template <typename IteratorT, typename FuncT>
auto makeTransformIterator(IteratorT iterator, FuncT function);
```

This function creates a `TransformIterator` from an underlying iterator and a transformation function.

## FilterIterator

### Description

`FilterIterator` filters elements of the underlying iterator based on a predicate.

### Class Definition

```cpp
template <typename IteratorT, typename PredicateT>
class FilterIterator {
    // ... (implementation details)
};
```

### Usage

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
auto isEven = [](int x) { return x % 2 == 0; };
auto begin = makeFilterIterator(vec.begin(), vec.end(), isEven);
auto end = makeFilterIterator(vec.end(), vec.end(), isEven);

for (auto it = begin; it != end; ++it) {
    std::cout << *it << " ";
}
// Output: 2 4 6 8 10
```

### Key Methods

- `operator*()`: Returns a reference to the current element.
- `operator->()`: Returns a pointer to the current element.
- `operator++()`: Pre-increment operator.
- `operator++(int)`: Post-increment operator.
- `operator==()` and `operator!=()`: Comparison operators.

### Helper Function

```cpp
template <typename IteratorT, typename PredicateT>
auto makeFilterIterator(IteratorT iter, IteratorT end, PredicateT pred);
```

This function creates a `FilterIterator` from an underlying iterator range and a predicate.

## ReverseIterator

### Description

`ReverseIterator` reverses the direction of another iterator.

### Class Definition

```cpp
template <typename IteratorT>
class ReverseIterator {
    // ... (implementation details)
};
```

### Usage

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};
ReverseIterator<std::vector<int>::iterator> rbegin(vec.end());
ReverseIterator<std::vector<int>::iterator> rend(vec.begin());

for (auto it = rbegin; it != rend; ++it) {
    std::cout << *it << " ";
}
// Output: 5 4 3 2 1
```

### Key Methods

- `base()`: Returns the underlying iterator.
- `operator*()`: Returns a reference to the element.
- `operator->()`: Returns a pointer to the element.
- `operator++()` and `operator--()`: Pre-increment and pre-decrement operators.
- `operator++(int)` and `operator--(int)`: Post-increment and post-decrement operators.
- `operator==()` and `operator!=()`: Comparison operators.

## ZipIterator

### Description

`ZipIterator` combines multiple iterators into a single iterator that produces tuples of values.

### Class Definition

```cpp
template <typename... Iterators>
class ZipIterator {
    // ... (implementation details)
};
```

### Usage

```cpp
std::vector<int> vec1 = {1, 2, 3};
std::vector<char> vec2 = {'a', 'b', 'c'};
auto it = makeZipIterator(vec1.begin(), vec2.begin());

for (int i = 0; i < 3; ++i, ++it) {
    auto [num, ch] = *it;
    std::cout << num << ch << " ";
}
// Output: 1a 2b 3c
```

### Key Methods

- `operator*()`: Returns a tuple containing the values from all zipped iterators.
- `operator++()`: Pre-increment operator.
- `operator++(int)`: Post-increment operator.
- `operator==()` and `operator!=()`: Comparison operators.

### Helper Function

```cpp
template <typename... Iterators>
auto makeZipIterator(Iterators... its);
```

This function creates a `ZipIterator` from multiple underlying iterators.

## Best Practices and Considerations

When using these custom iterator classes, keep the following best practices and considerations in mind:

1. **Iterator Validity**: Ensure that the underlying iterators remain valid throughout the lifetime of the custom iterator. Modifying the container while iterating can lead to undefined behavior.

2. **Performance**: Some of these iterators, like `FilterIterator`, may have different performance characteristics compared to standard iterators. Be mindful of this when working with large datasets.

3. **Compatibility**: These custom iterators are designed to work with C++17 and later. Ensure your compiler supports the required features.

4. **Error Handling**: These iterators do not perform extensive error checking. It's the responsibility of the user to ensure proper usage, such as not dereferencing end iterators.

5. **Const Correctness**: When working with const containers or iterators, make sure to use the appropriate const versions of these custom iterators.

6. **Iterator Categories**: Be aware of the iterator categories for each custom iterator. For example, `FilterIterator` is a forward iterator, which may limit its use in algorithms requiring bidirectional or random access iterators.

7. **Memory Management**: These iterators do not manage memory themselves. Ensure proper lifetime management of the underlying containers and iterators.

8. **Combining Iterators**: You can often combine these iterators for more complex operations. For example, you could use a `TransformIterator` with a `FilterIterator` to transform and then filter elements.

9. **Debug Mode**: The `processContainer` function includes debug output when `ENABLE_DEBUG` is defined. Use this for troubleshooting but remember to disable it in release builds for performance.

10. **Template Constraints**: Some iterators use C++20 concepts (like `std::input_or_output_iterator`). If you're using an earlier C++ standard, you may need to modify these constraints.

## Example: Combining Multiple Custom Iterators

Here's an example that demonstrates how to combine multiple custom iterators:

```cpp
#include <vector>
#include <iostream>
#include "iter.hpp"

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Create a filter for even numbers
    auto isEven = [](int x) { return x % 2 == 0; };

    // Create a transform to square numbers
    auto square = [](int x) { return x * x; };

    // Combine FilterIterator and TransformIterator
    auto filteredBegin = makeFilterIterator(numbers.begin(), numbers.end(), isEven);
    auto filteredEnd = makeFilterIterator(numbers.end(), numbers.end(), isEven);

    auto transformedBegin = makeTransformIterator(filteredBegin, square);
    auto transformedEnd = makeTransformIterator(filteredEnd, square);

    // Use ReverseIterator to print in reverse order
    ReverseIterator<decltype(transformedBegin)> rbegin(transformedEnd);
    ReverseIterator<decltype(transformedEnd)> rend(transformedBegin);

    std::cout << "Squared even numbers in reverse order: ";
    for (auto it = rbegin; it != rend; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Expected output: Squared even numbers in reverse order: 100 64 36 16 4

    return 0;
}
```

This example demonstrates how to:

1. Use `FilterIterator` to select only even numbers.
2. Apply `TransformIterator` to square the filtered numbers.
3. Use `ReverseIterator` to iterate over the results in reverse order.

By combining these custom iterators, you can create powerful and flexible data processing pipelines.

## Conclusion

These custom iterator classes provide powerful tools for manipulating and traversing data structures in C++. By understanding their behavior and following best practices, you can write more expressive and efficient code. Remember to always consider the specific requirements of your project and the performance implications when choosing and combining these iterators.
