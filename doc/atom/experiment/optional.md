# Optional Class in C++

## Introduction

The `Optional` class provides a way to represent an optional value that may or may not contain a valid object of type `T`. It allows you to safely work with values that may be missing, eliminating the need for null pointers.

## Class Definition

```cpp
template <typename T>
class Optional {
private:
    alignas(T) unsigned char storage[sizeof(T)];
    bool hasValue;

public:
    // Constructors
    Optional();
    Optional(const T &value);
    Optional(T &&value);
    Optional(const Optional &other);
    Optional(Optional &&other);

    // Destructor
    ~Optional();

    // Assignment operators
    Optional &operator=(const Optional &other);
    Optional &operator=(Optional &&other);

    // Member functions
    T &operator*();
    const T &operator*() const;
    void reset();
    T &value();
    const T &value() const;
    explicit operator bool() const;
    bool operator==(const Optional &other) const;
    bool operator!=(const Optional &other) const;
    T value_or(const T &defaultValue) const;
};
```

## Usage Example

### Creating and Using Optional Objects

```cpp
#include <iostream>
#include "Optional.h"

int main() {
    Optional<int> optInt;
    std::cout << "Optional has value: " << static_cast<bool>(optInt) << std::endl; // Output: 0 (false)

    Optional<std::string> optStr("Hello");
    std::cout << "Optional has value: " << static_cast<bool>(optStr) << std::endl; // Output: 1 (true)

    try {
        std::cout << "Value in optStr: " << *optStr << std::endl; // Output: Hello
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    Optional<std::vector<int>> optVec(std::vector<int>{1, 2, 3});
    std::cout << "Value in optVec: ";
    for (const auto &num : optVec.value()) {
        std::cout << num << " ";
    }
    std::cout << std::endl; // Output: 1 2 3

    return 0;
}
```

## Functionality

- The `Optional` class allows you to store values of type `T` optionally.
- It provides safe access to the contained value with operators like `*` and `value()`.
- You can check if the `Optional` contains a value using the boolean conversion operator.
- The `reset()` function clears the stored value.
- Comparison operators `==` and `!=` are provided for comparing `Optional` objects.
- The `value_or()` function returns the contained value or a default value if no value is present.

## Conclusion

The `Optional` class in C++ provides a safer and more expressive way to handle optional values, avoiding the pitfalls of null pointers. By following the usage examples and understanding the class functions, you can leverage `Optional` to enhance the robustness of your code.
