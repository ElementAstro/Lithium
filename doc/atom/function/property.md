# Overload.hpp Documentation

## Overview

The `overload.hpp` file, part of the `atom::meta` namespace, provides utility functions to simplify the casting of overloaded member functions and free functions. It introduces the `OverloadCast` struct and the `overload_cast` helper variable template, which allow for better type deduction when working with function overloads.

## Table of Contents

1. [OverloadCast Struct](#overloadcast-struct)
2. [overload_cast Variable Template](#overload_cast-variable-template)
3. [decayCopy Function](#decaycopy-function)
4. [Usage Examples](#usage-examples)
5. [Notes and Considerations](#notes-and-considerations)

## OverloadCast Struct

```cpp
template <typename... Args>
struct OverloadCast { /* ... */ };
```

The `OverloadCast` struct is a utility to simplify the casting of overloaded member functions and free functions. It provides several overloads of the `operator()` to handle different types of function pointers:

- Non-const member functions
- Const member functions
- Volatile member functions
- Const volatile member functions
- Free functions
- Noexcept versions of all the above

Each `operator()` is marked as `constexpr` and `noexcept`, allowing for compile-time evaluation and guaranteed no-throw behavior.

## overload_cast Variable Template

```cpp
template <typename... Args>
constexpr auto overload_cast = OverloadCast<Args...>{};
```

The `overload_cast` variable template is a helper that instantiates `OverloadCast` with the specified argument types. It simplifies the usage of `OverloadCast` by allowing users to specify only the argument types of the function they want to cast.

## decayCopy Function

```cpp
template <class T>
constexpr auto decayCopy(T&& value) noexcept(
    std::is_nothrow_convertible_v<T, std::decay_t<T>>) -> std::decay_t<T>;
```

The `decayCopy` function template creates a decayed copy of the input value. It uses perfect forwarding and is conditionally `noexcept` based on whether the conversion to the decayed type is nothrow convertible.

## Usage Examples

### Basic Usage with Member Functions

```cpp
#include "overload.hpp"
#include <iostream>

class Example {
public:
    void foo(int x) { std::cout << "foo(int): " << x << std::endl; }
    void foo(double x) { std::cout << "foo(double): " << x << std::endl; }
};

int main() {
    Example e;

    // Using overload_cast to select the int overload
    auto int_foo = atom::meta::overload_cast<int>(&Example::foo);
    (e.*int_foo)(42);  // Prints: foo(int): 42

    // Using overload_cast to select the double overload
    auto double_foo = atom::meta::overload_cast<double>(&Example::foo);
    (e.*double_foo)(3.14);  // Prints: foo(double): 3.14

    return 0;
}
```

### Usage with Const and Non-const Member Functions

```cpp
#include "overload.hpp"
#include <iostream>

class ConstExample {
public:
    void print() { std::cout << "Non-const print()" << std::endl; }
    void print() const { std::cout << "Const print()" << std::endl; }
};

int main() {
    ConstExample ce;
    const ConstExample const_ce;

    // Select non-const version
    auto non_const_print = atom::meta::overload_cast<>(&ConstExample::print);
    (ce.*non_const_print)();  // Prints: Non-const print()

    // Select const version
    auto const_print = atom::meta::overload_cast<>(&ConstExample::print);
    (const_ce.*const_print)();  // Prints: Const print()

    return 0;
}
```

### Usage with Free Functions

```cpp
#include "overload.hpp"
#include <iostream>

void greet(const char* name) {
    std::cout << "Hello, " << name << "!" << std::endl;
}

void greet(int times) {
    for (int i = 0; i < times; ++i) {
        std::cout << "Hello!" << std::endl;
    }
}

int main() {
    // Select the const char* overload
    auto greet_name = atom::meta::overload_cast<const char*>(greet);
    greet_name("Alice");  // Prints: Hello, Alice!

    // Select the int overload
    auto greet_times = atom::meta::overload_cast<int>(greet);
    greet_times(3);  // Prints: Hello! Hello! Hello!

    return 0;
}
```

### Using decayCopy

```cpp
#include "overload.hpp"
#include <iostream>
#include <string>

int main() {
    std::string str = "Hello, world!";
    const char* decayed = atom::meta::decayCopy(str.c_str());
    std::cout << decayed << std::endl;  // Prints: Hello, world!

    return 0;
}
```

## Notes and Considerations

1. The `OverloadCast` struct and `overload_cast` variable template are particularly useful when dealing with overloaded functions, especially in template contexts where the compiler might have difficulty deducing the correct overload.

2. All member function overloads in `OverloadCast` are marked `constexpr` and `noexcept`, allowing for potential compile-time optimizations and guaranteeing no exceptions will be thrown during the cast operation.

3. The `decayCopy` function is useful for creating copies of values with their decayed types, which can be helpful in generic programming scenarios.

4. When using `overload_cast` with member functions, remember to use the `.*` or `->*` operators to call the function on an object.

5. The `overload_cast` utility works with both member functions and free functions, providing a unified interface for handling various function types.

6. This utility can significantly improve code readability when working with complex overloaded function sets, especially in template metaprogramming contexts.
