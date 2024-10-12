# Function Traits Documentation

## Overview

The `func_traits.hpp` file provides a comprehensive set of tools for compile-time introspection of function types in C++20. It's part of the `atom::meta` namespace and offers functionality to analyze various aspects of functions, including member functions, lambdas, and function objects.

## Table of Contents

1. [Key Components](#key-components)
2. [FunctionTraits](#functiontraits)
3. [Utility Variable Templates](#utility-variable-templates)
4. [Debug Utilities](#debug-utilities)
5. [Function Pipe](#function-pipe)
6. [Method Detection](#method-detection)
7. [Usage Examples](#usage-examples)
8. [Notes and Considerations](#notes-and-considerations)

## Key Components

### FunctionTraits

The core of this library is the `FunctionTraits` struct template, which provides detailed information about function types.

```cpp
template <typename Func>
struct FunctionTraits;
```

### FunctionTraitsBase

A base struct that provides common functionality for all function traits specializations.

```cpp
template <typename Return, typename... Args>
struct FunctionTraitsBase;
```

## FunctionTraits

`FunctionTraits` is specialized for various function types, including:

- Regular functions
- Member functions (const, volatile, lvalue reference, rvalue reference)
- Noexcept functions
- Variadic functions
- Lambdas and function objects

It provides the following information:

- `return_type`: The return type of the function
- `argument_types`: A tuple type containing the argument types
- `arity`: The number of arguments
- `argument_t<N>`: The type of the Nth argument
- Various boolean flags for function properties (e.g., `is_member_function`, `is_const_member_function`, etc.)
- `full_name`: A string containing the demangled full name of the function type

## Utility Variable Templates

The library provides convenience variable templates for easy access to function properties:

```cpp
template <typename Func>
inline constexpr bool is_member_function_v;

template <typename Func>
inline constexpr bool is_const_member_function_v;

template <typename Func>
inline constexpr bool is_volatile_member_function_v;

template <typename Func>
inline constexpr bool is_lvalue_reference_member_function_v;

template <typename Func>
inline constexpr bool is_rvalue_reference_member_function_v;

template <typename Func>
inline constexpr bool is_noexcept_v;

template <typename Func>
inline constexpr bool is_variadic_v;
```

## Debug Utilities

When `ENABLE_DEBUG` is defined, the library provides debug utilities:

```cpp
template <typename Tuple>
void print_tuple_types();

template <typename F>
void print_function_info(const std::string &name, F &&);
```

These functions can be used to print detailed information about function types during development.

## Function Pipe

The library includes a `function_pipe` class that allows for function composition using the pipe operator (`|`):

```cpp
template <typename Func>
class function_pipe;
```

## Method Detection

The library provides templates and macros for detecting the presence of methods in classes:

```cpp
template <typename, typename T, typename = void>
struct has_method;

#define DEFINE_HAS_METHOD(MethodName)

template <typename, typename T, typename = void>
struct has_static_method;

#define DEFINE_HAS_STATIC_METHOD(MethodName)

template <typename, typename T, typename = void>
struct has_const_method;

#define DEFINE_HAS_CONST_METHOD(MethodName)
```

## Usage Examples

### Basic Function Traits

```cpp
#include "func_traits.hpp"
#include <iostream>

int add(int a, int b) { return a + b; }

int main() {
    using traits = atom::meta::FunctionTraits<decltype(add)>;

    std::cout << "Return type: " << typeid(traits::return_type).name() << std::endl;
    std::cout << "Number of arguments: " << traits::arity << std::endl;
    std::cout << "First argument type: " << typeid(traits::argument_t<0>).name() << std::endl;

    return 0;
}
```

### Member Function Detection

```cpp
#include "func_traits.hpp"
#include <iostream>

class MyClass {
public:
    void myMethod(int) {}
    static void myStaticMethod(double) {}
    void myConstMethod() const {}
};

DEFINE_HAS_METHOD(myMethod);
DEFINE_HAS_STATIC_METHOD(myStaticMethod);
DEFINE_HAS_CONST_METHOD(myConstMethod);

int main() {
    std::cout << "Has myMethod: " << has_myMethod<MyClass, void, int>::value << std::endl;
    std::cout << "Has myStaticMethod: " << has_static_myStaticMethod<MyClass, void, double>::value << std::endl;
    std::cout << "Has myConstMethod: " << has_const_myConstMethod<MyClass, void>::value << std::endl;

    return 0;
}
```

### Function Pipe

```cpp
#include "func_traits.hpp"
#include <iostream>

int main() {
    auto add = [](int a, int b) { return a + b; };
    auto multiply = [](int a, int b) { return a * b; };

    atom::meta::function_pipe add_pipe(add);
    atom::meta::function_pipe multiply_pipe(multiply);

    int result = 5 | add_pipe(3) | multiply_pipe(2);
    std::cout << "Result: " << result << std::endl;  // Output: 16

    return 0;
}
```

## Notes and Considerations

1. This library requires C++20 or later due to the use of concepts and other modern C++ features.
2. The `FunctionTraits` struct provides comprehensive information about function types, including member functions with various qualifiers (const, volatile, &, &&, noexcept).
3. The debug utilities (`print_tuple_types` and `print_function_info`) are only available when `ENABLE_DEBUG` is defined.
4. The `function_pipe` class allows for function composition using the pipe operator, which can be useful for creating readable function chains.
5. The method detection macros (`DEFINE_HAS_METHOD`, `DEFINE_HAS_STATIC_METHOD`, and `DEFINE_HAS_CONST_METHOD`) provide a convenient way to check for the existence of methods in classes.
6. When using the library, be aware of potential compile-time overhead for complex function types.
7. The library uses template metaprogramming techniques, which may impact compile times for large projects.

Remember to compile with C++20 support enabled (e.g., `-std=c++20` for GCC/Clang) when using this header.
