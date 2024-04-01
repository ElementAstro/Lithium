# StringUtils.h

## Introduction

This C++ header file provides utility functions to convert various data types into strings for easy output and manipulation. The functions handle different types of containers, maps, key-value pairs, and basic data types.

### Functionality

1. Conversion of basic data types and strings to strings.
2. Conversion of containers (vectors, lists, etc.) to strings.
3. Conversion of key-value pairs to strings.
4. Joining key-value pairs with custom separators.
5. Joining command line arguments into a single string.
6. Generating string representation of arrays.

### Functions

#### `toString`

```cpp
template <typename T>
auto toString(const T &value) -> std::enable_if_t<!is_map<T>::value && !is_container<T>::value, std::string>

template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue)

template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue, const std::string &separator)

template <typename Container>
std::enable_if_t<is_map<Container>::value, std::string> toString(const Container &container)

template <typename Container>
auto toString(const Container &container) -> std::enable_if_t<is_container<Container>::value &&
                            !is_map<Container>::value &&
                            !is_string_type<typename Container::value_type>, std::string>

template <typename T>
std::string toString(const std::vector<T> &value)
```

#### `joinKeyValuePair`

```cpp
template <typename T>
std::enable_if_t<is_string_type<T>, std::string> joinKeyValuePair(const std::string &key, const T &value, const std::string &separator = "")

template <typename Key, typename Value>
std::string joinKeyValuePair(const std::pair<Key, Value> &keyValue, const std::string &separator = "")
```

#### `joinCommandLine`

```cpp
template <typename... Args>
[[nodiscard]] std::string joinCommandLine(const Args &...args)
```

#### `toStringArray`

```cpp
template <typename T>
auto toStringArray(const std::vector<T> &array)
```

### Example Usage

```cpp
#include <iostream>
#include "string_utils.h" // Include the header file

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::map<std::string, int> ages = {{"Alice", 30}, {"Bob", 25}, {"Charlie", 35}};

    // Convert basic types to strings
    std::string numberAsString = toString(42);
    std::string text = toString("Hello, World!");

    // Convert containers to strings
    std::string numbersString = toString(numbers);
    std::string agesString = toString(ages);

    // Join key-value pairs
    std::string keyValueString = joinKeyValuePair("Name", "Alice", ": ");
    std::string pairString = joinKeyValuePair(std::make_pair("Age", 30), " = ");

    // Join command line arguments
    std::string commandLine = joinCommandLine("ls", "-l", "-a");

    // Generate string representation of an array
    std::string arrayString = toStringArray(numbers);

    // Output the results
    std::cout << numberAsString << std::endl;
    std::cout << text << std::endl;
    std::cout << numbersString << std::endl;
    std::cout << agesString << std::endl;
    std::cout << keyValueString << std::endl;
    std::cout << pairString << std::endl;
    std::cout << commandLine << std::endl;
    std::cout << arrayString << std::endl;

    return 0;
}
```

### Function Explanation

- **`toString`**: Converts various types to strings.
- **`joinKeyValuePair`**: Joins key-value pairs with custom separators.
- **`joinCommandLine`**: Joins command line arguments into a single string.
- **`toStringArray`**: Generates a string representation of an array.
