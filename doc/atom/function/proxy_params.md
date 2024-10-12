# FunctionParams Class Documentation

The `FunctionParams` class is designed to encapsulate function parameters using `std::any`. This class provides a flexible way to store and manage parameters of various types.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructors](#constructors)
3. [Member Functions](#member-functions)
4. [Usage Examples](#usage-examples)

## Class Overview

The `FunctionParams` class is defined in the `proxy_params.hpp` header file. It uses C++20 features and the Standard Template Library (STL).

```cpp
class FunctionParams {
    // ... (class implementation)
};
```

## Constructors

### Single Value Constructor

```cpp
explicit FunctionParams(const std::any& value);
```

Constructs `FunctionParams` with a single `std::any` value.

### Range Constructor

```cpp
template <std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::any>
explicit constexpr FunctionParams(const Range& range);
```

Constructs `FunctionParams` from any range of `std::any` values.

### Initializer List Constructor

```cpp
constexpr FunctionParams(std::initializer_list<std::any> ilist);
```

Constructs `FunctionParams` from an initializer list of `std::any` values.

## Member Functions

### operator[]

```cpp
[[nodiscard]] auto operator[](std::size_t t_i) const -> const std::any&;
```

Accesses the parameter at the given index. Throws `std::out_of_range` if the index is out of range.

### begin and end

```cpp
[[nodiscard]] auto begin() const noexcept;
[[nodiscard]] auto end() const noexcept;
```

Return iterators to the beginning and end of the parameters, respectively.

### front

```cpp
[[nodiscard]] auto front() const noexcept -> const std::any&;
```

Returns the first parameter.

### size

```cpp
[[nodiscard]] auto size() const noexcept -> std::size_t;
```

Returns the number of parameters.

### empty

```cpp
[[nodiscard]] auto empty() const noexcept -> bool;
```

Checks if there are no parameters.

### toVector

```cpp
[[nodiscard]] auto toVector() const -> std::vector<std::any>;
```

Converts the parameters to a vector of `std::any`.

### get

```cpp
template <typename T>
[[nodiscard]] auto get(std::size_t index) const -> std::optional<T>;
```

Gets the parameter at the given index as a specific type. Returns `std::nullopt` if the cast fails.

### slice

```cpp
[[nodiscard]] auto slice(std::size_t start, std::size_t end) const -> FunctionParams;
```

Slices the parameters from the given start index to the end index. Throws `std::out_of_range` if the slice range is invalid.

### filter

```cpp
template <typename Predicate>
[[nodiscard]] auto filter(Predicate pred) const -> FunctionParams;
```

Filters the parameters using a predicate.

### set

```cpp
void set(std::size_t index, const std::any& value);
```

Sets the parameter at the given index to a new value. Throws `std::out_of_range` if the index is out of range.

## Usage Examples

Here are some examples demonstrating how to use the `FunctionParams` class:

```cpp
#include "proxy_params.hpp"
#include <iostream>
#include <string>

int main() {
    // Creating FunctionParams with a single value
    FunctionParams single_param(std::any(42));
    std::cout << std::any_cast<int>(single_param[0]) << std::endl; // Output: 42

    // Creating FunctionParams with multiple values
    FunctionParams multi_params({std::any(10), std::any("Hello"), std::any(3.14)});

    // Accessing parameters
    std::cout << std::any_cast<int>(multi_params[0]) << std::endl; // Output: 10
    std::cout << std::any_cast<const char*>(multi_params[1]) << std::endl; // Output: Hello
    std::cout << std::any_cast<double>(multi_params[2]) << std::endl; // Output: 3.14

    // Using get() method
    auto int_value = multi_params.get<int>(0);
    if (int_value) {
        std::cout << "First parameter as int: " << *int_value << std::endl; // Output: First parameter as int: 10
    }

    // Using slice() method
    auto sliced_params = multi_params.slice(1, 3);
    std::cout << "Sliced params size: " << sliced_params.size() << std::endl; // Output: Sliced params size: 2

    // Using filter() method
    auto filtered_params = multi_params.filter([](const std::any& param) {
        return param.type() == typeid(int);
    });
    std::cout << "Filtered params size: " << filtered_params.size() << std::endl; // Output: Filtered params size: 1

    // Using set() method
    multi_params.set(1, std::any(std::string("World")));
    std::cout << std::any_cast<std::string>(multi_params[1]) << std::endl; // Output: World

    return 0;
}
```

This example demonstrates various operations using the `FunctionParams` class, including creation, accessing elements, type-safe retrieval, slicing, filtering, and updating values.

Remember to include proper error handling in your actual code, as some operations may throw exceptions if used incorrectly.
