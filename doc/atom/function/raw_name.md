# Raw Name Extraction Utilities

This document describes the usage of the `raw_name_of` functions and related utilities from the `atom::meta` namespace, which provide ways to extract the raw names of types and values at compile-time.

## Table of Contents

1. [Introduction](#introduction)
2. [Function Overview](#function-overview)
3. [Usage Examples](#usage-examples)
4. [Compiler Support](#compiler-support)
5. [Advanced Features](#advanced-features)

## Introduction

The `raw_name.hpp` file provides a set of template functions that allow you to extract the raw names of types, values, and enum members at compile-time. These functions are particularly useful for reflection-like capabilities and debugging purposes.

## Function Overview

### `raw_name_of<T>()`

```cpp
template <typename T>
constexpr auto raw_name_of();
```

Extracts the raw name of a type `T`.

### `raw_name_of_template<T>()`

```cpp
template <typename T>
constexpr auto raw_name_of_template();
```

Extracts the raw name of a template type `T`.

### `raw_name_of<Value>()`

```cpp
template <auto Value>
constexpr auto raw_name_of();
```

Extracts the raw name of a compile-time value `Value`.

### `raw_name_of_enum<Value>()`

```cpp
template <auto Value>
constexpr auto raw_name_of_enum();
```

Extracts the raw name of an enum value `Value`.

### `raw_name_of_member<T>()` (C++20 and above)

```cpp
template <Wrapper T>
constexpr auto raw_name_of_member();
```

Extracts the raw name of a member wrapped in a `Wrapper` struct.

## Usage Examples

Here are some examples demonstrating how to use the `raw_name_of` functions:

### Example 1: Basic Usage

```cpp
#include "raw_name.hpp"
#include <iostream>

struct MyStruct {};
enum class MyEnum { Value1, Value2 };

int main() {
    std::cout << "Type name: " << atom::meta::raw_name_of<int>() << std::endl;
    std::cout << "Struct name: " << atom::meta::raw_name_of<MyStruct>() << std::endl;
    std::cout << "Enum name: " << atom::meta::raw_name_of<MyEnum>() << std::endl;
    std::cout << "Enum value name: " << atom::meta::raw_name_of_enum<MyEnum::Value1>() << std::endl;

    return 0;
}
```

Expected output:

```
Type name: int
Struct name: MyStruct
Enum name: MyEnum
Enum value name: Value1
```

### Example 2: Template Types

```cpp
#include "raw_name.hpp"
#include <iostream>
#include <vector>

template<typename T>
struct TemplateStruct {};

int main() {
    std::cout << "Vector name: " << atom::meta::raw_name_of_template<std::vector<int>>() << std::endl;
    std::cout << "Template struct name: " << atom::meta::raw_name_of_template<TemplateStruct<double>>() << std::endl;

    return 0;
}
```

Expected output (may vary slightly depending on the compiler):

```
Vector name: vector<int>
Template struct name: TemplateStruct<double>
```

### Example 3: Compile-time Values

```cpp
#include "raw_name.hpp"
#include <iostream>

constexpr int CompileTimeValue = 42;

int main() {
    std::cout << "Compile-time value name: " << atom::meta::raw_name_of<CompileTimeValue>() << std::endl;

    return 0;
}
```

Expected output:

```
Compile-time value name: CompileTimeValue
```

### Example 4: Member Names (C++20 and above)

```cpp
#include "raw_name.hpp"
#include <iostream>

struct MyStruct {
    int member;
};

int main() {
    constexpr auto wrapper = atom::meta::Wrapper{&MyStruct::member};
    std::cout << "Member name: " << atom::meta::raw_name_of_member<wrapper>() << std::endl;

    return 0;
}
```

Expected output:

```
Member name: member
```

## Compiler Support

The `raw_name.hpp` file includes conditional compilation directives to support different compilers:

- GCC and Clang are supported using the `__GNUC__` and `__clang__` macros.
- MSVC is supported using the `_MSC_VER` macro.

If an unsupported compiler is detected, a static assertion will fail with the message "Unsupported compiler".

## Advanced Features

### `args_type_of`

```cpp
template <typename T>
using args_type_of = args_type_of<T>;
```

This alias template is provided for extracting argument types, but its implementation is not shown in the given code snippet. It's likely defined elsewhere in the library.

---
