# Hash Algorithms Documentation

## Overview

The `hash.hpp` file provides a collection of hash algorithms and utility functions for computing hash values of various data types. It includes implementations for hashing single values, vectors, tuples, and arrays, as well as a FNV-1a hash implementation for strings.

## Namespace

All the main functions and concepts are defined within the `atom::algorithm` namespace.

```cpp
namespace atom::algorithm {
    // ...
}
```

## Concepts

### Hashable

```cpp
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};
```

The `Hashable` concept defines types that can be hashed. A type is considered `Hashable` if it supports hashing via `std::hash` and the result is convertible to `std::size_t`.

## Functions

### computeHash (Single Value)

```cpp
template <Hashable T>
auto computeHash(const T& value) -> std::size_t;
```

Computes the hash value for a single `Hashable` value.

- **Parameters:**
  - `value`: The value to hash.
- **Returns:** Hash value of the input value as `std::size_t`.
- **Usage:**
  ```cpp
  int num = 42;
  std::size_t hash_value = atom::algorithm::computeHash(num);
  ```

### computeHash (Vector)

```cpp
template <Hashable T>
auto computeHash(const std::vector<T>& values) -> std::size_t;
```

Computes the hash value for a vector of `Hashable` values.

- **Parameters:**
  - `values`: The vector of values to hash.
- **Returns:** Hash value of the vector of values as `std::size_t`.
- **Usage:**
  ```cpp
  std::vector<int> numbers = {1, 2, 3, 4, 5};
  std::size_t hash_value = atom::algorithm::computeHash(numbers);
  ```

### computeHash (Tuple)

```cpp
template <Hashable... Ts>
auto computeHash(const std::tuple<Ts...>& tuple) -> std::size_t;
```

Computes the hash value for a tuple of `Hashable` values.

- **Parameters:**
  - `tuple`: The tuple of values to hash.
- **Returns:** Hash value of the tuple of values as `std::size_t`.
- **Usage:**
  ```cpp
  auto my_tuple = std::make_tuple(1, "hello", 3.14);
  std::size_t hash_value = atom::algorithm::computeHash(my_tuple);
  ```

### computeHash (Array)

```cpp
template <Hashable T, std::size_t N>
auto computeHash(const std::array<T, N>& array) -> std::size_t;
```

Computes the hash value for an array of `Hashable` values.

- **Parameters:**
  - `array`: The array of values to hash.
- **Returns:** Hash value of the array of values as `std::size_t`.
- **Usage:**
  ```cpp
  std::array<int, 5> numbers = {1, 2, 3, 4, 5};
  std::size_t hash_value = atom::algorithm::computeHash(numbers);
  ```

## Global Functions

### hash (FNV-1a for Strings)

```cpp
constexpr auto hash(const char* str, unsigned int basis = 2166136261U) -> unsigned int;
```

Computes a hash value for a null-terminated string using the FNV-1a algorithm.

- **Parameters:**
  - `str`: Pointer to the null-terminated string to hash.
  - `basis`: Initial basis value for hashing (default: 2166136261U).
- **Returns:** Hash value of the string as `unsigned int`.
- **Usage:**
  ```cpp
  const char* my_string = "Hello, World!";
  unsigned int hash_value = hash(my_string);
  ```

### operator""\_hash (User-defined Literal)

```cpp
constexpr auto operator""_hash(const char* str, std::size_t size) -> unsigned int;
```

User-defined literal for computing hash values of string literals.

- **Parameters:**
  - `str`: Pointer to the string literal to hash.
  - `size`: Size of the string literal (unused in the implementation).
- **Returns:** Hash value of the string literal as `unsigned int`.
- **Usage:**
  ```cpp
  auto hash_value = "example"_hash;
  ```

## Best Practices

1. Use the `Hashable` concept to ensure that types you're working with can be hashed properly.
2. When hashing multiple values, prefer using the appropriate `computeHash` overload (vector, tuple, or array) rather than combining hashes manually.
3. For string hashing, use the global `hash` function or the `_hash` user-defined literal for compile-time hashing of string literals.
4. Be cautious when using hash values for security-sensitive applications, as these implementations are not cryptographically secure.

## Notes

- The hash functions within the `atom::algorithm` namespace use a combination of XOR and bit shifting for combining hash values, which is effective for general-purpose hashing.
- The FNV-1a algorithm is used for string hashing, which provides a good balance between speed and hash distribution for strings.
- The `_hash` user-defined literal allows for compile-time hashing of string literals, which can be useful for switch statements or other compile-time applications.

## Example: Combining Different Hash Types

Here's an example that demonstrates how to use various hash functions together:

```cpp
#include "hash.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <tuple>

int main() {
    // Single value
    int single_value = 42;
    std::cout << "Single value hash: " << atom::algorithm::computeHash(single_value) << std::endl;

    // Vector
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::cout << "Vector hash: " << atom::algorithm::computeHash(vec) << std::endl;

    // Array
    std::array<double, 3> arr = {1.1, 2.2, 3.3};
    std::cout << "Array hash: " << atom::algorithm::computeHash(arr) << std::endl;

    // Tuple
    auto tup = std::make_tuple(10, "hello", 3.14);
    std::cout << "Tuple hash: " << atom::algorithm::computeHash(tup) << std::endl;

    // String (using global hash function)
    const char* str = "Hello, World!";
    std::cout << "String hash: " << hash(str) << std::endl;

    // String literal (using user-defined literal)
    auto literal_hash = "example"_hash;
    std::cout << "String literal hash: " << literal_hash << std::endl;

    return 0;
}
```

This example shows how to use the various hash functions provided in the `hash.hpp` file, including hashing of single values, containers, tuples, and strings.
