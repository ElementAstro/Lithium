# Indestructible Class Template Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor](#constructor)
4. [Destructor](#destructor)
5. [Copy and Move Operations](#copy-and-move-operations)
6. [Accessor Methods](#accessor-methods)
7. [Conversion Operators](#conversion-operators)
8. [Utility Methods](#utility-methods)
9. [Usage Examples](#usage-examples)
10. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The `Indestructible` class template is designed to create objects that cannot be destructed normally. This can be useful in scenarios where you need to ensure that an object persists throughout the entire lifetime of a program or when you want to manage the destruction of an object manually.

## Class Overview

```cpp
template <typename T>
struct Indestructible {
    // ... (class implementation)
};
```

The `Indestructible` class template wraps an object of type `T` and provides controlled access to it while preventing its automatic destruction.

## Constructor

```cpp
template <typename... Args>
    requires std::is_constructible_v<T, Args...>
constexpr explicit Indestructible(std::in_place_t /*unused*/, Args&&... args)
    : object(std::forward<Args>(args)...) {}
```

The constructor takes an `std::in_place_t` as its first argument (which is unused) followed by any number of arguments that are forwarded to construct the wrapped object.

Usage:

```cpp
Indestructible<MyClass> obj(std::in_place, arg1, arg2, arg3);
```

## Destructor

```cpp
~Indestructible() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
        object.~T();
    }
}
```

The destructor explicitly calls the destructor of the wrapped object if it's not trivially destructible.

## Copy and Move Operations

The class provides both copy and move constructors and assignment operators. These operations are defaulted if `T` is trivially copyable or movable; otherwise, they are implemented to perform the appropriate copy or move operation on the wrapped object.

Example usage:

```cpp
Indestructible<MyClass> obj1(std::in_place, args...);
Indestructible<MyClass> obj2 = obj1;  // Copy construction
Indestructible<MyClass> obj3 = std::move(obj1);  // Move construction
```

## Accessor Methods

```cpp
constexpr auto get() & -> T&
constexpr auto get() const& -> const T&
constexpr auto get() && -> T&&
constexpr const T&& get() const&&
```

These methods provide access to the wrapped object, supporting both lvalue and rvalue contexts.

Usage:

```cpp
Indestructible<MyClass> obj(std::in_place, args...);
MyClass& ref = obj.get();
const MyClass& constRef = std::as_const(obj).get();
```

## Conversion Operators

```cpp
constexpr explicit operator T&() &
constexpr explicit operator const T&() const&
constexpr explicit operator T&&() &&
constexpr explicit operator const T&&() const&&
```

These operators allow explicit conversion to references of the wrapped type.

Usage:

```cpp
Indestructible<MyClass> obj(std::in_place, args...);
MyClass& ref = static_cast<MyClass&>(obj);
```

## Utility Methods

### reset

```cpp
template <typename... Args>
    requires std::is_constructible_v<T, Args...>
constexpr void reset(Args&&... args)
```

This method destroys the current object and constructs a new one in its place.

Usage:

```cpp
obj.reset(newArg1, newArg2);
```

### emplace

```cpp
template <typename... Args>
    requires std::is_constructible_v<T, Args...>
constexpr void emplace(Args&&... args)
```

This method is an alias for `reset`, providing a way to construct a new object in place.

Usage:

```cpp
obj.emplace(newArg1, newArg2);
```

## Usage Examples

1. Basic usage:

```cpp
#include "indestructible.hpp"
#include <iostream>

class MyClass {
public:
    MyClass(int value) : value_(value) {
        std::cout << "MyClass constructed with value " << value_ << std::endl;
    }
    ~MyClass() {
        std::cout << "MyClass destructed with value " << value_ << std::endl;
    }
    int getValue() const { return value_; }
private:
    int value_;
};

int main() {
    {
        Indestructible<MyClass> obj(std::in_place, 42);
        std::cout << "Value: " << obj->getValue() << std::endl;
    }
    std::cout << "End of scope" << std::endl;
    return 0;
}
```

Output:

```
MyClass constructed with value 42
Value: 42
End of scope
```

Note that the destructor of `MyClass` is not called when the `Indestructible` object goes out of scope.

2. Using reset and emplace:

```cpp
Indestructible<std::string> strObj(std::in_place, "Hello");
std::cout << strObj->c_str() << std::endl;  // Outputs: Hello

strObj.reset("World");
std::cout << strObj->c_str() << std::endl;  // Outputs: World

strObj.emplace("Indestructible");
std::cout << strObj->c_str() << std::endl;  // Outputs: Indestructible
```

3. Copy and move operations:

```cpp
Indestructible<std::vector<int>> vecObj1(std::in_place, {1, 2, 3});
Indestructible<std::vector<int>> vecObj2 = vecObj1;  // Copy
Indestructible<std::vector<int>> vecObj3 = std::move(vecObj1);  // Move

std::cout << vecObj2->size() << std::endl;  // Outputs: 3
std::cout << vecObj3->size() << std::endl;  // Outputs: 3
std::cout << vecObj1->size() << std::endl;  // Outputs: 0 (moved-from state)
```

## Best Practices and Considerations

1. Use `Indestructible` sparingly and only when you truly need an object to persist beyond normal scope rules.
2. Be aware that using `Indestructible` can lead to resource leaks if not managed properly, as the wrapped object's destructor is not automatically called.
3. When using `Indestructible` with types that manage resources (e.g., file handles, memory allocations), consider implementing a manual cleanup method or using it in conjunction with smart pointers for resource management.
4. The `Indestructible` class is most useful for singleton patterns or for objects that need to live for the entire duration of the program.
5. Remember that while the object is indestructible, it can still be modified. Use const correctness and access control to prevent unwanted modifications.
6. When using `Indestructible` in multithreaded environments, ensure proper synchronization to avoid race conditions.

By following these guidelines and understanding the behavior of the `Indestructible` class template, you can effectively use it in your C++ projects to manage object lifetimes in specialized scenarios.
