# Hash Utility

**Author:** Max Qian
**Date:** March 28, 2024
**Version:** 1.0

---

## Overview

The Atom Algorithm Hash Library is a collection of hash algorithms implemented in C++ to provide developers with a versatile and efficient way to compute hash values for various data types. It offers functionality for computing hash values for individual values, vectors, tuples, and arrays.

## Features

- **Generic Hashing:** The library supports hashing for any data type that satisfies the `Hashable` concept, including built-in types and user-defined types with a defined `std::hash` specialization.
- **Support for Containers:** Hashing functions are provided for `std::vector`, `std::tuple`, and `std::array`, allowing users to compute hash values for collections of elements.

- **Customizable Hashing:** Users can easily extend the library by providing custom `std::hash` specializations for their own types, enabling seamless integration with existing codebases.

## Concepts

### Hashable Concept

The `Hashable` concept specifies that a type must be hashable using `std::hash`. It is satisfied by any type for which `std::hash` is specialized.

### computeHash Functions

The library provides several overloaded `computeHash` functions, each tailored to compute hash values for different types of input:

- `computeHash(const T& value)`: Computes the hash value for a single value of type `T`.
- `computeHash(const std::vector<T>& values)`: Computes the hash value for a vector of elements of type `T`.

- `computeHash(const std::tuple<Ts...>& tuple)`: Computes the hash value for a tuple of elements, where `Ts...` represents the types of the tuple elements.

- `computeHash(const std::array<T, N>& array)`: Computes the hash value for an array of elements of type `T` and size `N`.

## Usage

To use the Atom Algorithm Hash Library, include the `hash.hpp` header file in your C++ project and make sure to compile with C++17 or later.

```cpp
#include "hash.hpp"
#include <iostream>
#include <vector>
#include <tuple>

int main() {
    std::vector<int> values = {1, 2, 3, 4, 5};
    std::size_t hashValue = computeHash(values);
    std::cout << "Hash value for vector: " << hashValue << std::endl;

    std::tuple<int, double, char> tuple = {1, 3.14, 'a'};
    hashValue = computeHash(tuple);
    std::cout << "Hash value for tuple: " << hashValue << std::endl;

    return 0;
}
```
