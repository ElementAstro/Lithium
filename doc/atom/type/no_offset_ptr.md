# UnshiftedPtr Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Member Functions](#member-functions)
4. [Usage Examples](#usage-examples)
5. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The `UnshiftedPtr` class is a lightweight pointer-like class that manages an object of type `T` without dynamic memory allocation. It provides a way to store and manage an object inline, similar to `std::optional`, but with more control over the object's lifetime and construction.

## Class Definition

```cpp
template <typename T>
    requires std::is_object_v<T>
class UnshiftedPtr {
    // ... (implementation details)
};
```

The class is templated on the type `T` of the object it manages, with a requirement that `T` must be an object type.

## Member Functions

### Constructors

1. Default constructor:

   ```cpp
   constexpr UnshiftedPtr() noexcept(std::is_nothrow_default_constructible_v<T>);
   ```

   Constructs the managed object using `T`'s default constructor.

2. Parameterized constructor:
   ```cpp
   template <typename... Args>
       requires std::constructible_from<T, Args...>
   constexpr explicit UnshiftedPtr(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);
   ```
   Constructs the managed object with the given arguments.

### Destructor

```cpp
constexpr ~UnshiftedPtr() noexcept;
```

Destroys the managed object.

### Operators

1. Arrow operator:

   ```cpp
   constexpr auto operator->() noexcept -> T*;
   constexpr auto operator->() const noexcept -> const T*;
   ```

   Provides pointer-like access to the managed object.

2. Dereference operator:
   ```cpp
   constexpr auto operator*() noexcept -> T&;
   constexpr auto operator*() const noexcept -> const T&;
   ```
   Dereferences the managed object.

### Member Functions

1. Reset:

   ```cpp
   template <typename... Args>
       requires std::constructible_from<T, Args...>
   constexpr void reset(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_destructible_v<T>);
   ```

   Resets the managed object by calling its destructor and reconstructing it in-place.

2. Emplace:

   ```cpp
   template <typename... Args>
       requires std::constructible_from<T, Args...>
   constexpr void emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);
   ```

   Emplaces a new object in place with the provided arguments.

3. Release:

   ```cpp
   [[nodiscard]] constexpr auto release() noexcept -> T*;
   ```

   Releases ownership of the managed object without destroying it.

4. Has Value:
   ```cpp
   [[nodiscard]] constexpr auto hasValue() const noexcept -> bool;
   ```
   Checks if the managed object has a value.

## Usage Examples

Here are some examples demonstrating the usage of the `UnshiftedPtr` class:

```cpp
#include "no_offset_ptr.hpp"
#include <iostream>
#include <string>

class MyClass {
public:
    MyClass(int value) : value_(value) {}
    int getValue() const { return value_; }
    void setValue(int value) { value_ = value; }
private:
    int value_;
};

int main() {
    // Default construction
    UnshiftedPtr<int> intPtr;
    *intPtr = 42;
    std::cout << "Int value: " << *intPtr << std::endl;

    // Parameterized construction
    UnshiftedPtr<MyClass> myClassPtr(10);
    std::cout << "MyClass value: " << myClassPtr->getValue() << std::endl;

    // Using reset
    myClassPtr.reset(20);
    std::cout << "MyClass value after reset: " << myClassPtr->getValue() << std::endl;

    // Using emplace
    UnshiftedPtr<std::string> stringPtr;
    stringPtr.emplace("Hello, UnshiftedPtr!");
    std::cout << "String value: " << *stringPtr << std::endl;

    // Using arrow operator
    stringPtr->append(" More text.");
    std::cout << "Updated string value: " << *stringPtr << std::endl;

    // Using release
    MyClass* releasedPtr = myClassPtr.release();
    std::cout << "Released MyClass value: " << releasedPtr->getValue() << std::endl;
    delete releasedPtr;  // Don't forget to delete the released pointer

    // Checking if has value
    std::cout << "stringPtr has value: " << std::boolalpha << stringPtr.hasValue() << std::endl;

    return 0;
}
```

This example demonstrates:

1. Default construction of an `UnshiftedPtr<int>`.
2. Parameterized construction of an `UnshiftedPtr<MyClass>`.
3. Using the `reset` function to reconstruct the managed object.
4. Using the `emplace` function to construct a new object in-place.
5. Using the arrow operator to access members of the managed object.
6. Using the `release` function to transfer ownership of the managed object.
7. Using the `hasValue` function to check if the managed object has a value.

## Best Practices and Considerations

1. Use `UnshiftedPtr` when you need inline storage of an object with controlled lifetime:
   `UnshiftedPtr` is useful when you want to avoid dynamic memory allocation but still need pointer-like semantics.

2. Be careful with `release`:
   After calling `release`, the `UnshiftedPtr` no longer manages the object. Make sure to properly manage the returned pointer to avoid memory leaks.

3. Prefer `emplace` over `reset` when constructing new objects:
   `emplace` is more explicit about constructing a new object, while `reset` can be used for both construction and assignment.

4. Remember that `UnshiftedPtr` always contains a value:
   Unlike `std::optional`, `UnshiftedPtr` always contains a constructed object of type `T`. The `hasValue` function will always return true.

5. Use with move-only types:
   `UnshiftedPtr` can be particularly useful for managing move-only types that you want to store inline.

6. Be aware of the object's lifetime:
   The managed object is constructed when the `UnshiftedPtr` is constructed and destroyed when the `UnshiftedPtr` is destroyed. Make sure this aligns with your intended usage.

7. Consider exception safety:
   The `noexcept` specifiers on member functions depend on the `noexcept`-ness of `T`'s constructors and destructors. Be aware of this when using `UnshiftedPtr` with types that may throw exceptions.
