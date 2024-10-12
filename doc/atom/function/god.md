# God.hpp Documentation

## Overview

The `god.hpp` file is part of the `atom::meta` namespace and provides a collection of utility functions and type traits. These utilities are designed to assist with common programming tasks such as type casting, memory alignment, bit manipulation, and type checking.

## Table of Contents

1. [Utility Functions](#utility-functions)
2. [Type Aliases](#type-aliases)
3. [Type Traits](#type-traits)
4. [Usage Examples](#usage-examples)
5. [Notes and Considerations](#notes-and-considerations)

## Utility Functions

### blessNoBugs

A no-op function for blessing with no bugs.

```cpp
ATOM_INLINE void blessNoBugs();
```

### cast

Casts a value from one type to another.

```cpp
template <typename To, typename From>
constexpr auto cast(From&& fromValue) -> To;
```

### alignUp and alignDown

Align values or pointers up or down to the nearest multiple of a given alignment.

```cpp
template <std::size_t Alignment, typename ValueType>
constexpr auto alignUp(ValueType value) -> ValueType;

template <std::size_t Alignment, typename PointerType>
constexpr auto alignUp(PointerType* pointer) -> PointerType*;

template <typename ValueType, typename AlignmentType>
constexpr auto alignUp(ValueType value, AlignmentType alignment) -> ValueType;

template <typename PointerType, typename AlignmentType>
constexpr auto alignUp(PointerType* pointer, AlignmentType alignment) -> PointerType*;

// Similar functions exist for alignDown
```

### log2

Computes the base-2 logarithm of an integral value.

```cpp
template <typename IntegralType>
constexpr auto log2(IntegralType value) -> IntegralType;
```

### nb

Computes the number of blocks of size N needed to cover a value.

```cpp
template <std::size_t BlockSize, typename ValueType>
constexpr auto nb(ValueType value) -> ValueType;
```

### eq

Compares two values for equality.

```cpp
template <typename ValueType>
ATOM_INLINE auto eq(const void* first, const void* second) -> bool;
```

### copy

Copies N bytes from src to dst.

```cpp
template <std::size_t NumBytes>
ATOM_INLINE void copy(void* destination, const void* source);
```

### swap, fetchAdd, fetchSub, fetchAnd, fetchOr, fetchXor

Atomic-like operations on values.

```cpp
template <typename PointerType, typename ValueType>
ATOM_INLINE auto swap(PointerType* pointer, ValueType value) -> PointerType;

template <typename PointerType, typename ValueType>
ATOM_INLINE auto fetchAdd(PointerType* pointer, ValueType value) -> PointerType;

// Similar functions exist for fetchSub, fetchAnd, fetchOr, fetchXor
```

## Type Aliases

```cpp
template <bool Condition, typename Type = void>
using if_t = std::enable_if_t<Condition, Type>;

template <typename Type>
using rmRefT = std::remove_reference_t<Type>;

template <typename Type>
using rmCvT = std::remove_cv_t<Type>;

template <typename Type>
using rmCvRefT = rmCvT<rmRefT<Type>>;

template <typename Type>
using rmArrT = std::remove_extent_t<Type>;

template <typename Type>
using constT = std::add_const_t<Type>;

template <typename Type>
using constRefT = std::add_lvalue_reference_t<constT<rmRefT<Type>>>;
```

## Type Traits

```cpp
template <typename FirstType, typename SecondType, typename... RemainingTypes>
constexpr auto isSame() -> bool;

template <typename Type>
constexpr auto isRef() -> bool;

template <typename Type>
constexpr auto isArray() -> bool;

template <typename Type>
constexpr auto isClass() -> bool;

template <typename Type>
constexpr auto isScalar() -> bool;

template <typename Type>
constexpr auto isTriviallyCopyable() -> bool;

template <typename Type>
constexpr auto isTriviallyDestructible() -> bool;

template <typename BaseType, typename DerivedType>
constexpr auto isBaseOf() -> bool;

template <typename Type>
constexpr auto hasVirtualDestructor() -> bool;
```

## Usage Examples

### Alignment Functions

```cpp
#include "god.hpp"
#include <iostream>

int main() {
    int value = 17;
    std::cout << "Original value: " << value << std::endl;
    std::cout << "Aligned up to 8: " << atom::meta::alignUp<8>(value) << std::endl;
    std::cout << "Aligned down to 8: " << atom::meta::alignDown<8>(value) << std::endl;

    int* ptr = &value;
    std::cout << "Original pointer: " << ptr << std::endl;
    std::cout << "Aligned up to 16: " << atom::meta::alignUp<16>(ptr) << std::endl;

    return 0;
}
```

### Type Traits

```cpp
#include "god.hpp"
#include <iostream>

class BaseClass {};
class DerivedClass : public BaseClass {};

int main() {
    std::cout << "Is int a scalar? " << atom::meta::isScalar<int>() << std::endl;
    std::cout << "Is BaseClass a base of DerivedClass? "
              << atom::meta::isBaseOf<BaseClass, DerivedClass>() << std::endl;
    std::cout << "Does BaseClass have a virtual destructor? "
              << atom::meta::hasVirtualDestructor<BaseClass>() << std::endl;

    return 0;
}
```

### Atomic-like Operations

```cpp
#include "god.hpp"
#include <iostream>

int main() {
    int value = 10;
    std::cout << "Original value: " << value << std::endl;

    std::cout << "Fetch and add 5: " << atom::meta::fetchAdd(&value, 5) << std::endl;
    std::cout << "New value: " << value << std::endl;

    std::cout << "Fetch and AND with 14: " << atom::meta::fetchAnd(&value, 14) << std::endl;
    std::cout << "New value: " << value << std::endl;

    return 0;
}
```

## Notes and Considerations

1. Many functions in this library are marked `constexpr`, allowing for compile-time evaluation when possible.
2. The alignment functions assume that the alignment values are powers of 2. Using non-power-of-2 alignments may lead to unexpected results.
3. The atomic-like operations (swap, fetchAdd, etc.) are not actually atomic. They are simple wrappers around non-atomic operations and should not be used in multi-threaded contexts without proper synchronization.
4. The type traits provided are thin wrappers around standard C++ type traits, offering a more concise syntax in some cases.
5. The `blessNoBugs` function is a no-op and its purpose is likely humorous or superstitious.
6. When using template functions like `cast`, be careful about potential narrowing conversions or other unsafe casts.
7. The `ATOM_INLINE` macro is used frequently. Ensure this macro is properly defined in your project.

Remember to include the necessary headers and compile with C++17 or later support when using this library.
