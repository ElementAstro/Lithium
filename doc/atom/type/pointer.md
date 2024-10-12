# PointerSentinel Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Template Parameters](#template-parameters)
4. [Constructors](#constructors)
5. [Member Functions](#member-functions)
6. [Helper Methods](#helper-methods)
7. [Usage Examples](#usage-examples)

## Introduction

The `PointerSentinel` class is a versatile wrapper for different types of pointers, including raw pointers, `std::shared_ptr`, `std::unique_ptr`, and `std::weak_ptr`. It provides a unified interface for working with these pointer types, allowing for easier management and manipulation of pointers in C++ code.

## Class Definition

```cpp
template <typename T>
class PointerSentinel {
    // ... (implementation details)
};
```

## Template Parameters

- `T`: The type of the object being pointed to.

## Constructors

1. Default constructor:

   ```cpp
   PointerSentinel() = default;
   ```

2. Shared pointer constructor:

   ```cpp
   explicit PointerSentinel(std::shared_ptr<T> p);
   ```

3. Unique pointer constructor:

   ```cpp
   explicit PointerSentinel(std::unique_ptr<T>&& p);
   ```

4. Weak pointer constructor:

   ```cpp
   explicit PointerSentinel(std::weak_ptr<T> p);
   ```

5. Raw pointer constructor:

   ```cpp
   explicit PointerSentinel(T* p);
   ```

6. Copy constructor:

   ```cpp
   PointerSentinel(const PointerSentinel& other);
   ```

7. Move constructor:

   ```cpp
   PointerSentinel(PointerSentinel&& other) noexcept = default;
   ```

## Member Functions

1. Copy assignment operator:

   ```cpp
   auto operator=(const PointerSentinel& other) -> PointerSentinel&;
   ```

2. Move assignment operator:

   ```cpp
   auto operator=(PointerSentinel&& other) noexcept -> PointerSentinel& = default;
   ```

3. Get raw pointer:

   ```cpp
   ATOM_NODISCARD auto get() const -> T*;
   ```

## Helper Methods

1. Invoke member function:

   ```cpp
   template <typename Func, typename... Args>
   ATOM_NODISCARD auto invoke(Func func, Args&&... args);
   ```

2. Apply callable object:

   ```cpp
   template <typename Callable>
   ATOM_NODISCARD auto apply(Callable&& callable);
   ```

3. Apply void function:

   ```cpp
   template <typename Func, typename... Args>
   void applyVoid(Func func, Args&&... args);
   ```

## Usage Examples

Here are some examples demonstrating the usage of the `PointerSentinel` class:

```cpp
#include "pointer.hpp"
#include <iostream>
#include <memory>

// Example class
class ExampleClass {
public:
    void printMessage() const {
        std::cout << "Hello from ExampleClass!" << std::endl;
    }

    int add(int a, int b) const {
        return a + b;
    }
};

int main() {
    // Using PointerSentinel with different pointer types

    // 1. With std::shared_ptr
    auto sharedPtr = std::make_shared<ExampleClass>();
    PointerSentinel<ExampleClass> sharedSentinel(sharedPtr);

    // 2. With std::unique_ptr
    auto uniquePtr = std::make_unique<ExampleClass>();
    PointerSentinel<ExampleClass> uniqueSentinel(std::move(uniquePtr));

    // 3. With std::weak_ptr
    std::weak_ptr<ExampleClass> weakPtr = sharedPtr;
    PointerSentinel<ExampleClass> weakSentinel(weakPtr);

    // 4. With raw pointer
    ExampleClass* rawPtr = new ExampleClass();
    PointerSentinel<ExampleClass> rawSentinel(rawPtr);

    // Using the get() method
    std::cout << "Raw pointer from sharedSentinel: " << sharedSentinel.get() << std::endl;

    // Using the invoke() method to call a member function
    sharedSentinel.invoke(&ExampleClass::printMessage);

    // Using the invoke() method with arguments
    int result = sharedSentinel.invoke(&ExampleClass::add, 5, 3);
    std::cout << "Result of add: " << result << std::endl;

    // Using the apply() method with a lambda
    auto lambdaResult = sharedSentinel.apply([](ExampleClass* ptr) {
        return ptr->add(10, 20);
    });
    std::cout << "Result of lambda: " << lambdaResult << std::endl;

    // Using the applyVoid() method
    sharedSentinel.applyVoid([](ExampleClass* ptr) {
        ptr->printMessage();
    });

    // Clean up the raw pointer
    delete rawPtr;

    return 0;
}
```

This example demonstrates:

1. Creating `PointerSentinel` objects with different pointer types.
2. Using the `get()` method to access the raw pointer.
3. Using the `invoke()` method to call member functions with and without arguments.
4. Using the `apply()` method with a lambda function.
5. Using the `applyVoid()` method for functions that don't return a value.

Note: When using `PointerSentinel` with raw pointers, remember to manage the memory manually (as shown with the `delete rawPtr;` statement in the example).

The `PointerSentinel` class provides a flexible and safe way to work with different pointer types, allowing for consistent pointer manipulation regardless of the underlying pointer type. This can be particularly useful in template programming or when dealing with APIs that may return different pointer types.
