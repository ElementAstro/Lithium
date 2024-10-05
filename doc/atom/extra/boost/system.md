# Boost System Wrapper Documentation

## Overview

This C++ header file provides a wrapper around Boost.System's error handling mechanisms. It defines custom `Error`, `Exception`, and `Result` classes in the `atom::extra::boost` namespace. These classes aim to simplify error handling and provide a more modern C++ interface for dealing with errors and results.

## Table of Contents

1. [Error Class](#error-class)
2. [Exception Class](#exception-class)
3. [Result Class](#result-class)
4. [makeResult Function](#makeresult-function)

## Error Class

The `Error` class encapsulates a Boost system error code.

### Methods

- `Error()`: Default constructor.
- `Error(const ::boost::system::error_code& error_code)`: Constructs an Error from a Boost error code.
- `Error(int error_value, const ::boost::system::error_category& error_category)`: Constructs an Error from an error value and category.
- `value()`: Returns the error value.
- `category()`: Returns the error category.
- `message()`: Returns the error message.
- `operator bool()`: Returns true if there is an error.
- `toBoostErrorCode()`: Converts back to a Boost error code.
- `operator==` and `operator!=`: Comparison operators.

### Usage Example

```cpp
atom::extra::boost::Error error(boost::system::errc::not_supported, boost::system::generic_category());
if (error) {
    std::cout << "Error: " << error.message() << std::endl;
}
```

## Exception Class

The `Exception` class is derived from `std::system_error` and wraps an `Error` object.

### Methods

- `Exception(const Error& error)`: Constructs an Exception from an Error.
- `error()`: Returns the wrapped Error object.

### Usage Example

```cpp
try {
    throw atom::extra::boost::Exception(atom::extra::boost::Error(boost::system::errc::not_supported, boost::system::generic_category()));
} catch (const atom::extra::boost::Exception& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
}
```

## Result Class

The `Result` class is a template class that represents either a successful value or an error. It has two specializations: one for non-void types and one for void.

### Methods (non-void specialization)

- `Result(T value)`: Constructs a Result with a success value.
- `Result(Error error)`: Constructs a Result with an error.
- `hasValue()`: Returns true if the Result contains a value.
- `value()`: Returns the contained value or throws an Exception if there's an error.
- `error()`: Returns the Error object if there's an error.
- `operator bool()`: Returns true if the Result contains a value.
- `valueOr(U&& default_value)`: Returns the value or a default value if there's an error.
- `map(F&& func)`: Applies a function to the contained value if present.
- `andThen(F&& func)`: Chains Result-returning functions.

### Methods (void specialization)

- `Result()`: Default constructor for successful void Result.
- `Result(Error error)`: Constructs a void Result with an error.
- `hasValue()`: Returns true if the Result represents success.
- `error()`: Returns the Error object if there's an error.
- `operator bool()`: Returns true if the Result represents success.

### Usage Example

```cpp
atom::extra::boost::Result<int> divide(int a, int b) {
    if (b == 0) {
        return atom::extra::boost::Result<int>(atom::extra::boost::Error(boost::system::errc::invalid_argument, boost::system::generic_category()));
    }
    return atom::extra::boost::Result<int>(a / b);
}

auto result = divide(10, 2);
if (result) {
    std::cout << "Result: " << result.value() << std::endl;
} else {
    std::cout << "Error: " << result.error().message() << std::endl;
}

// Using map and andThen
auto result2 = divide(10, 2)
    .map([](int value) { return value * 2; })
    .andThen([](int value) { return divide(value, 0); });

if (!result2) {
    std::cout << "Error in chain: " << result2.error().message() << std::endl;
}
```

## makeResult Function

The `makeResult` function is a utility that wraps a function call in a Result object, catching any exceptions and converting them to errors.

### Usage Example

```cpp
auto result = atom::extra::boost::makeResult([]() {
    // Some operation that might throw
    if (/* some condition */) {
        throw atom::extra::boost::Exception(atom::extra::boost::Error(boost::system::errc::not_supported, boost::system::generic_category()));
    }
    return 42;
});

if (result) {
    std::cout << "Result: " << result.value() << std::endl;
} else {
    std::cout << "Error: " << result.error().message() << std::endl;
}
```
