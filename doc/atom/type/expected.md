# atom::type::expected Class Documentation

The `atom::type::expected` class is a C++ implementation of the Expected monad pattern, providing a way to handle operations that might fail without using exceptions. It is similar to `std::expected` from C++23, but with some additional features.

## Table of Contents

1. [Overview](#overview)
2. [Class Definitions](#class-definitions)
3. [Main Features](#main-features)
4. [Usage Examples](#usage-examples)
5. [Best Practices](#best-practices)

## Overview

The `expected` class template allows you to represent either a valid value of type `T` or an error of type `E`. This is particularly useful for error handling in situations where using exceptions might be undesirable or too costly.

## Class Definitions

### Error<E>

```cpp
template <typename E>
class Error {
public:
    explicit Error(E error);
    template <typename T> requires std::is_same_v<E, std::string>
    explicit Error(const T* error);
    [[nodiscard]] auto error() const -> const E&;
    auto operator==(const Error& other) const -> bool;
};
```

### unexpected<E>

```cpp
template <typename E>
class unexpected {
public:
    explicit unexpected(const E& error);
    explicit unexpected(E&& error);
    [[nodiscard]] auto error() const -> const E&;
};
```

### expected<T, E>

```cpp
template <typename T, typename E = std::string>
class expected {
public:
    expected(const T& value);
    expected(T&& value);
    expected(const Error<E>& error);
    expected(Error<E>&& error);
    expected(const unexpected<E>& unex);

    [[nodiscard]] auto has_value() const -> bool;
    auto value() -> T&;
    [[nodiscard]] auto value() const -> const T&;
    auto error() -> Error<E>&;
    [[nodiscard]] auto error() const -> const Error<E>&;
    template <typename U>
    auto value_or(U&& default_value) const -> T;

    template <typename Func>
    auto map(Func&& func) const -> expected<decltype(func(std::declval<T>())), E>;
    template <typename Func>
    auto and_then(Func&& func) const -> decltype(func(std::declval<T>()));
    template <typename Func>
    auto transform_error(Func&& func) const -> expected<T, decltype(func(std::declval<E>()))>;
    template <typename Func>
    auto or_else(Func&& func) const -> expected<T, E>;

    friend auto operator==(const expected& lhs, const expected& rhs) -> bool;
    friend auto operator!=(const expected& lhs, const expected& rhs) -> bool;
};
```

### expected<void, E>

A specialization of `expected` for `void` type, representing operations that might fail but don't return a value.

## Main Features

1. **Value and Error Handling**: Store either a value of type `T` or an error of type `E`.
2. **Type Safety**: Provides type-safe access to the contained value or error.
3. **Monadic Operations**: Supports operations like `map`, `and_then`, `transform_error`, and `or_else` for composing operations.
4. **Default Value Handling**: `value_or` method to provide a default value if an error is present.
5. **Equality Comparisons**: Supports equality and inequality comparisons.

## Usage Examples

Here are some examples demonstrating how to use the `expected` class:

```cpp
#include "expected.hpp"
#include <iostream>
#include <string>

// Function that might fail
atom::type::expected<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return atom::type::make_unexpected("Division by zero");
    }
    return atom::type::make_expected(a / b);
}

// Example usage
int main() {
    // Basic usage
    auto result = divide(10, 2);
    if (result.has_value()) {
        std::cout << "Result: " << result.value() << std::endl;
    } else {
        std::cout << "Error: " << result.error().error() << std::endl;
    }

    // Using value_or
    auto result_with_zero = divide(5, 0);
    int safe_result = result_with_zero.value_or(-1);
    std::cout << "Safe result: " << safe_result << std::endl;

    // Using map
    auto squared_result = divide(8, 2).map([](int x) { return x * x; });
    if (squared_result.has_value()) {
        std::cout << "Squared result: " << squared_result.value() << std::endl;
    }

    // Using and_then
    auto complex_operation = divide(10, 2).and_then([](int x) {
        return divide(x, 2);
    });
    if (complex_operation.has_value()) {
        std::cout << "Complex operation result: " << complex_operation.value() << std::endl;
    }

    // Using transform_error
    auto transformed_error = divide(5, 0).transform_error([](const std::string& error) {
        return "Transformed: " + error;
    });
    if (!transformed_error.has_value()) {
        std::cout << "Transformed error: " << transformed_error.error().error() << std::endl;
    }

    // Using or_else
    auto handled_error = divide(5, 0).or_else([](const std::string& error) {
        std::cout << "Handling error: " << error << std::endl;
        return atom::type::make_expected(0);
    });
    std::cout << "Handled result: " << handled_error.value() << std::endl;

    return 0;
}
```

This example demonstrates:

1. Basic usage of `expected` with a function that might fail.
2. Using `value_or` to provide a default value.
3. Using `map` to transform the contained value.
4. Using `and_then` to chain operations that might fail.
5. Using `transform_error` to modify the error message.
6. Using `or_else` to handle errors and provide an alternative result.

## Best Practices

1. **Use `make_expected` and `make_unexpected`**: These factory functions help create `expected` and `unexpected` objects with proper type deduction.

2. **Prefer `and_then` for chaining**: When chaining operations that might fail, prefer `and_then` over directly accessing the value to maintain proper error handling.

3. **Use `transform_error` for error context**: Use `transform_error` to add context or transform error messages as they propagate up the call stack.

4. **Leverage `value_or` for safe defaults**: When a default value is acceptable, use `value_or` to handle errors gracefully without explicit checking.

5. **Consider using `expected<void, E>` for operations without return values**: For functions that might fail but don't return a value, use the `void` specialization of `expected`.

6. **Be consistent with error types**: Try to use consistent error types throughout your codebase to make error handling more uniform.
