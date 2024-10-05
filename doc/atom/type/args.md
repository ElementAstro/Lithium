# Args Class Documentation

## Introduction

`Args` is a universal container class for storing key-value pairs of any type. It's a simplified version of `ArgumentContainer`, offering better performance with fewer features.

## Header File

```cpp
#include "args.hpp"
```

## Class Definition

```cpp
class Args {
    // ... (member function definitions)
};
```

## Key Features

1. Set key-value pairs
2. Retrieve key-value pairs
3. Check if a key exists
4. Remove key-value pairs
5. Clear the container
6. Get container size and check if it's empty

## Member Functions

### Setting Key-Value Pairs

#### `set`

```cpp
template <typename T>
void set(std::string_view key, T &&value);
```

Set a single key-value pair.

```cpp
template <typename T>
void set(std::span<const std::pair<std::string_view, T>> pairs);
```

Set multiple key-value pairs in batch.

### Retrieving Key-Value Pairs

#### `get`

```cpp
template <typename T>
auto get(std::string_view key) const -> T;
```

Get the value for a specified key. Throws an exception if the key doesn't exist.

#### `getOr`

```cpp
template <typename T>
auto getOr(std::string_view key, T &&default_value) const -> T;
```

Get the value for a specified key, returning a default value if the key doesn't exist.

#### `getOptional`

```cpp
template <typename T>
auto getOptional(std::string_view key) const -> std::optional<T>;
```

Get the value for a specified key, returning `std::nullopt` if the key doesn't exist.

#### Batch Retrieval

```cpp
template <typename T>
auto get(std::span<const std::string_view> keys) const -> std::vector<std::optional<T>>;
```

Retrieve values for multiple keys in batch.

### Other Operations

#### `contains`

```cpp
auto contains(std::string_view key) const noexcept -> bool;
```

Check if a specified key exists.

#### `remove`

```cpp
void remove(std::string_view key);
```

Remove a specified key-value pair.

#### `clear`

```cpp
void clear() noexcept;
```

Clear the entire container.

#### `size`

```cpp
auto size() const noexcept -> size_t;
```

Return the number of key-value pairs in the container.

#### `empty`

```cpp
auto empty() const noexcept -> bool;
```

Check if the container is empty.

### Operator Overloads

```cpp
template <typename T>
auto operator[](std::string_view key) -> T &;

template <typename T>
auto operator[](std::string_view key) const -> const T &;

auto operator[](std::string_view key) -> std::any &;

auto operator[](std::string_view key) const -> const std::any &;
```

Access key-value pairs using the `[]` operator.

## Macro Definitions

For convenience, the `Args` class provides the following macro definitions:

```cpp
#define SET_ARGUMENT(container, name, value) container.set(#name, value)
#define GET_ARGUMENT(container, name, type) container.get<type>(#name).value_or(type{})
#define HAS_ARGUMENT(container, name) container.contains(#name)
#define REMOVE_ARGUMENT(container, name) container.remove(#name)
```

## Usage Examples

### Basic Usage

```cpp
#include "args.hpp"
#include <iostream>

int main() {
    Args args;

    // Set key-value pairs
    args.set("name", "Alice");
    args.set("age", 30);

    // Get values
    std::string name = args.get<std::string>("name");
    int age = args.get<int>("age");

    std::cout << "Name: " << name << ", Age: " << age << std::endl;

    // Use default value
    int height = args.getOr("height", 170);
    std::cout << "Height: " << height << std::endl;

    // Check if key exists
    if (args.contains("name")) {
        std::cout << "Name exists in the container." << std::endl;
    }

    // Remove key-value pair
    args.remove("age");

    // Use macros
    SET_ARGUMENT(args, score, 95.5);
    double score = GET_ARGUMENT(args, score, double);
    std::cout << "Score: " << score << std::endl;

    return 0;
}
```

### Batch Operations

```cpp
#include "args.hpp"
#include <iostream>
#include <vector>

int main() {
    Args args;

    // Set key-value pairs in batch
    std::vector<std::pair<std::string_view, int>> pairs = {
        {"a", 1}, {"b", 2}, {"c", 3}
    };
    args.set(pairs);

    // Get values in batch
    std::vector<std::string_view> keys = {"a", "b", "c", "d"};
    auto values = args.get<int>(keys);

    for (size_t i = 0; i < keys.size(); ++i) {
        if (values[i].has_value()) {
            std::cout << keys[i] << ": " << values[i].value() << std::endl;
        } else {
            std::cout << keys[i] << ": Not found" << std::endl;
        }
    }

    return 0;
}
```

### Using Operators

```cpp
#include "args.hpp"
#include <iostream>

int main() {
    Args args;

    args["x"] = 10;
    args["y"] = "Hello";

    int x = args["x"].get<int>();
    std::string y = args["y"].get<std::string>();

    std::cout << "x: " << x << ", y: " << y << std::endl;

    return 0;
}
```

## Important Notes

1. When using the `get` method, an exception will be thrown if the key doesn't exist. Consider using `getOr` or `getOptional` to avoid exceptions.
2. Using the `[]` operator will create a new key-value pair if the key doesn't exist.
3. When using template methods, ensure you specify the correct type to avoid runtime errors.
4. Batch operations can improve efficiency, especially when dealing with large amounts of data.

## Conclusion

The `Args` class provides a flexible and efficient way to manage key-value pair data. It's suitable for scenarios where you need to store data of different types and offers an intuitive API for manipulating this data. By using the provided macro definitions, you can further simplify your code and improve readability.
