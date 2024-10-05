# Field Count Documentation

## Overview

The `field_count.hpp` file provides utilities for counting the number of fields in aggregate types at compile-time. This is part of the `atom::meta` namespace and is designed to work with C++20 or later.

## Contents

1. [Structs and Classes](#structs-and-classes)
2. [Functions](#functions)
3. [Usage Examples](#usage-examples)
4. [Notes and Considerations](#notes-and-considerations)

## Structs and Classes

### `Any`

A utility struct that can be converted to any type.

```cpp
struct Any {
    template <typename T>
    explicit consteval operator T() const noexcept;
};
```

### `TypeInfo`

A template struct to hold type information.

```cpp
template <typename T>
struct TypeInfo;
```

This struct is left undefined in the header and can be specialized for specific types to provide custom field count information.

## Functions

### `isBracesConstructible`

Checks if a type `T` is constructible with braces.

```cpp
template <typename T, std::size_t... I>
consteval auto isBracesConstructible(std::index_sequence<I...> /*indices*/) noexcept -> bool;
```

### `fieldCount`

Recursively counts the number of fields in a type `T`.

```cpp
template <typename T, std::size_t N = 0>
consteval auto fieldCount() noexcept -> std::size_t;
```

### `fieldCountOf`

Gets the number of fields in a type `T`.

```cpp
template <typename T>
consteval auto fieldCountOf() noexcept -> std::size_t;
```

There's also a specialization for arrays:

```cpp
template <typename T, std::size_t N>
consteval auto fieldCountOf() noexcept -> std::size_t;
```

## Usage Examples

### Basic Usage

```cpp
#include "field_count.hpp"
#include <iostream>

struct Point {
    int x;
    int y;
};

struct Empty {};

int main() {
    std::cout << "Fields in Point: " << atom::meta::fieldCountOf<Point>() << std::endl;
    std::cout << "Fields in Empty: " << atom::meta::fieldCountOf<Empty>() << std::endl;

    int arr[5];
    std::cout << "Elements in arr: " << atom::meta::fieldCountOf<decltype(arr)>() << std::endl;
}
```

Output:

```
Fields in Point: 2
Fields in Empty: 0
Elements in arr: 5
```

### Custom Type Information

You can specialize the `TypeInfo` struct for types where the default field counting method might not work:

```cpp
#include "field_count.hpp"
#include <iostream>

struct ComplexType {
    // Some complex implementation
};

namespace atom::meta {
template <>
struct TypeInfo<ComplexType> {
    static constexpr std::size_t count = 3;  // Manually specify the field count
};
}

int main() {
    std::cout << "Fields in ComplexType: " << atom::meta::fieldCountOf<ComplexType>() << std::endl;
}
```

Output:

```txt
Fields in ComplexType: 3
```

## Notes and Considerations

1. This implementation uses C++20 features, particularly concepts and `consteval`.
2. The field counting method works for aggregate types. For non-aggregate types, it returns 0.
3. The `fieldCountOf` function first checks if the type is an aggregate. If it is, it then checks if there's a specialized `TypeInfo` for the type. If not, it falls back to the `fieldCount` implementation.
4. For arrays, a specialized `fieldCountOf` function returns the number of elements.
5. This method of field counting has limitations and may not work for all types, especially those with complex structures or non-public members.
6. The `Any` struct and `isBracesConstructible` function are implementation details used by the `fieldCount` function to perform the compile-time field counting.

Remember to compile with C++20 support enabled (e.g., `-std=c++20` for GCC/Clang) when using this header.
