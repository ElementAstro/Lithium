# Any Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor and Destructor](#constructor-and-destructor)
4. [Assignment and Swap](#assignment-and-swap)
5. [Type Checking and Casting](#type-checking-and-casting)
6. [Value Manipulation](#value-manipulation)
7. [Special Operations](#special-operations)
8. [Usage Examples](#usage-examples)
9. [Best Practices](#best-practices)

## Introduction

The `Any` class, part of the `atom::meta` namespace, is a type-safe container for single values of any type. It provides a flexible way to store and manipulate values of different types within a single object, similar to `std::any` but with additional features.

## Class Overview

The `Any` class uses a technique called "Small Object Optimization" to store small objects directly within the `Any` object, avoiding dynamic memory allocation for these cases. It also provides type-safe access to the contained value, as well as additional operations like `toString()`, `invoke()`, and `foreach()`.

## Constructor and Destructor

### Constructors

1. Default constructor:

   ```cpp
   Any() noexcept;
   ```

2. Copy constructor:

   ```cpp
   Any(const Any& other);
   ```

3. Move constructor:

   ```cpp
   Any(Any&& other) noexcept;
   ```

4. Template constructor for any type:
   ```cpp
   template <typename T>
   explicit Any(T&& value);
   ```

### Destructor

The destructor automatically cleans up the stored object.

## Assignment and Swap

### Assignment Operators

1. Copy assignment:

   ```cpp
   auto operator=(const Any& other) -> Any&;
   ```

2. Move assignment:

   ```cpp
   auto operator=(Any&& other) noexcept -> Any&;
   ```

3. Template assignment for any type:
   ```cpp
   template <typename T>
   auto operator=(T&& value) -> Any&;
   ```

### Swap

```cpp
void swap(Any& other) noexcept;
```

## Type Checking and Casting

### Type Checking

1. Check if the `Any` object contains a value:

   ```cpp
   [[nodiscard]] auto hasValue() const noexcept -> bool;
   ```

2. Get the type information of the contained value:

   ```cpp
   [[nodiscard]] auto type() const -> const std::type_info&;
   ```

3. Check if the contained value is of a specific type:
   ```cpp
   template <typename T>
   [[nodiscard]] auto is() const -> bool;
   ```

### Casting

1. Cast to a reference of the contained type:

   ```cpp
   template <typename T>
   auto cast() -> T&;
   ```

2. Cast to a const reference of the contained type:
   ```cpp
   template <typename T>
   [[nodiscard]] auto cast() const -> const T&;
   ```

## Value Manipulation

1. Reset the `Any` object, destroying the contained value:
   ```cpp
   void reset();
   ```

## Special Operations

1. Convert the contained value to a string:

   ```cpp
   [[nodiscard]] auto toString() const -> std::string;
   ```

2. Invoke a function with the contained value:

   ```cpp
   void invoke(const std::function<void(const void*)>& func) const;
   ```

3. Iterate over the contained value (if it's iterable):
   ```cpp
   void foreach(const std::function<void(const Any&)>& func) const;
   ```

## Usage Examples

### Basic Usage

```cpp
#include "any.hpp"
#include <iostream>

int main() {
    atom::meta::Any a = 42;
    std::cout << "Value: " << a.toString() << std::endl;
    std::cout << "Type: " << a.type().name() << std::endl;

    a = std::string("Hello, World!");
    std::cout << "New value: " << a.toString() << std::endl;
    std::cout << "New type: " << a.type().name() << std::endl;

    return 0;
}
```

### Type Checking and Casting

```cpp
#include "any.hpp"
#include <iostream>

int main() {
    atom::meta::Any a = 3.14;

    if (a.is<double>()) {
        std::cout << "a contains a double: " << a.cast<double>() << std::endl;
    }

    try {
        int value = a.cast<int>();
    } catch (const std::bad_cast& e) {
        std::cout << "Cast failed: " << e.what() << std::endl;
    }

    return 0;
}
```

### Using with Custom Types

```cpp
#include "any.hpp"
#include <iostream>

class MyClass {
public:
    MyClass(int value) : value_(value) {}
    int getValue() const { return value_; }
private:
    int value_;
};

int main() {
    atom::meta::Any a = MyClass(42);

    if (a.is<MyClass>()) {
        const MyClass& obj = a.cast<MyClass>();
        std::cout << "MyClass value: " << obj.getValue() << std::endl;
    }

    return 0;
}
```

### Using Special Operations

```cpp
#include "any.hpp"
#include <iostream>
#include <vector>

int main() {
    atom::meta::Any a = std::vector<int>{1, 2, 3, 4, 5};

    // Using toString
    std::cout << "ToString: " << a.toString() << std::endl;

    // Using invoke
    a.invoke([](const void* ptr) {
        const auto& vec = *static_cast<const std::vector<int>*>(ptr);
        std::cout << "Vector size: " << vec.size() << std::endl;
    });

    // Using foreach
    a.foreach([](const atom::meta::Any& item) {
        std::cout << "Item: " << item.cast<int>() << std::endl;
    });

    return 0;
}
```

## Best Practices

1. **Type Safety**: Always use `is<T>()` before calling `cast<T>()` to ensure type safety.

2. **Exception Handling**: Be prepared to handle `std::bad_cast` exceptions when using `cast<T>()`.

3. **Performance**: For frequently accessed values, consider using native types instead of `Any` to avoid the overhead of type checking and casting.

4. **Memory Management**: The `Any` class handles memory management internally, but be mindful of the lifetime of objects stored in `Any`, especially when storing pointers or references.

5. **Custom Types**: When using custom types with `Any`, ensure they are copyable and movable for best compatibility.

6. **Iterables**: When using `foreach()`, ensure that the contained type is iterable. Otherwise, it will throw an exception.

7. **toString() Method**: The default `toString()` implementation may not be suitable for all types. Consider providing custom string conversion for complex types.

8. **Small Object Optimization**: Be aware that the `Any` class uses small object optimization. This means that small objects (typically up to 24 bytes on most systems) are stored directly in the `Any` object, avoiding dynamic memory allocation.

By following these practices, you can effectively utilize the `Any` class in your C++ projects, allowing for more flexible and dynamic type handling while maintaining type safety.
