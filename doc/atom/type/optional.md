# Optional Class Template Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructors](#constructors)
4. [Operators](#operators)
5. [Methods](#methods)
6. [Advanced Operations](#advanced-operations)
7. [Usage Examples](#usage-examples)
8. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The `Optional` class template is a wrapper around `std::optional` that provides additional functionality and a more intuitive interface. It represents an optional value that may or may not be present, offering a safer alternative to nullable pointers or sentinel values.

## Class Overview

```cpp
namespace atom::type {

template <typename T>
class Optional {
public:
    // Constructors
    Optional() noexcept;
    Optional(std::nullopt_t) noexcept;
    Optional(const T& value);
    Optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>);
    Optional(const Optional& other);
    Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

    // Assignment operators
    Optional& operator=(std::nullopt_t) noexcept;
    Optional& operator=(const Optional& other);
    Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>);
    Optional& operator=(const T& value);
    Optional& operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>);

    // Methods
    template <typename... Args>
    T& emplace(Args&&... args);

    // ... (other methods listed in the following sections)

private:
    std::optional<T> storage_;
    void check_value() const;
};

} // namespace atom::type
```

## Constructors

1. Default constructor: `Optional() noexcept`
2. Nullopt constructor: `Optional(std::nullopt_t) noexcept`
3. Value copy constructor: `Optional(const T& value)`
4. Value move constructor: `Optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)`
5. Copy constructor: `Optional(const Optional& other)`
6. Move constructor: `Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)`

## Operators

1. Assignment operators:

   - `Optional& operator=(std::nullopt_t) noexcept`
   - `Optional& operator=(const Optional& other)`
   - `Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>)`
   - `Optional& operator=(const T& value)`
   - `Optional& operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>)`

2. Boolean conversion operator: `explicit operator bool() const noexcept`

3. Dereference operators:

   - `T& operator*() &`
   - `const T& operator*() const&`
   - `T&& operator*() &&`

4. Arrow operators:

   - `T* operator->()`
   - `const T* operator->() const`

5. Comparison operators:
   - `auto operator<=>(const Optional&) const = default`
   - `bool operator==(std::nullopt_t) const noexcept`
   - `auto operator<=>(std::nullopt_t) const noexcept`

## Methods

1. `template <typename... Args> T& emplace(Args&&... args)`: Constructs a new value in-place.

2. `T& value() &`: Accesses the contained value.
3. `const T& value() const&`: Accesses the contained value (const version).
4. `T&& value() &&`: Accesses the contained value and moves it.

5. `template <typename U> T value_or(U&& default_value) const&`: Returns the contained value or a default value.
6. `template <typename U> T value_or(U&& default_value) &&`: Returns the contained value or a default value (rvalue version).

7. `void reset() noexcept`: Resets the `Optional` object to an empty state.

## Advanced Operations

1. `template <typename F> auto map(F&& f) const -> Optional<std::invoke_result_t<F, T>>`: Applies a function to the contained value, if present.

2. `template <typename F> auto and_then(F&& f) const -> std::invoke_result_t<F, T>`: Applies a function to the contained value, if present.

3. `template <typename F> auto transform(F&& f) const -> Optional<std::invoke_result_t<F, T>>`: Alias for `map`.

4. `template <typename F> auto or_else(F&& f) const -> T`: Returns the contained value or invokes a function to generate a default value.

5. `template <typename F> auto transform_or(F&& f, const T& default_value) const -> Optional<T>`: Applies a function to the contained value and returns a new `Optional` with the result or a default value.

6. `template <typename F> auto flat_map(F&& f) const -> std::invoke_result_t<F, T>`: Applies a function to the contained value and returns the result.

## Usage Examples

### Basic Usage

```cpp
#include "optional.hpp"
#include <iostream>

int main() {
    atom::type::Optional<int> opt1;
    atom::type::Optional<int> opt2(42);

    std::cout << "opt1 has value: " << (opt1 ? "yes" : "no") << std::endl;
    std::cout << "opt2 has value: " << (opt2 ? "yes" : "no") << std::endl;

    if (opt2) {
        std::cout << "opt2 value: " << *opt2 << std::endl;
    }

    opt1 = 10;
    std::cout << "opt1 value: " << opt1.value() << std::endl;

    opt1.reset();
    std::cout << "opt1 has value after reset: " << (opt1 ? "yes" : "no") << std::endl;

    return 0;
}
```

### Using value_or

```cpp
#include "optional.hpp"
#include <iostream>
#include <string>

int main() {
    atom::type::Optional<std::string> name;
    std::cout << "Name: " << name.value_or("Unknown") << std::endl;

    name = "Alice";
    std::cout << "Name: " << name.value_or("Unknown") << std::endl;

    return 0;
}
```

### Using map and transform

```cpp
#include "optional.hpp"
#include <iostream>
#include <string>

int main() {
    atom::type::Optional<int> num(5);

    auto squared = num.map([](int x) { return x * x; });
    std::cout << "Squared: " << squared.value_or(0) << std::endl;

    auto as_string = num.transform([](int x) { return std::to_string(x); });
    std::cout << "As string: " << as_string.value_or("") << std::endl;

    return 0;
}
```

### Using and_then and flat_map

```cpp
#include "optional.hpp"
#include <iostream>
#include <string>

atom::type::Optional<std::string> findUser(int id) {
    if (id == 1) {
        return "Alice";
    } else if (id == 2) {
        return "Bob";
    }
    return std::nullopt;
}

atom::type::Optional<std::string> getUserEmail(const std::string& name) {
    if (name == "Alice") {
        return "alice@example.com";
    } else if (name == "Bob") {
        return "bob@example.com";
    }
    return std::nullopt;
}

int main() {
    atom::type::Optional<int> userId(2);

    auto userEmail = userId.and_then([](int id) {
        return findUser(id).and_then(getUserEmail);
    });

    std::cout << "User email: " << userEmail.value_or("Not found") << std::endl;

    // Using flat_map
    auto userEmailFlat = userId.flat_map([](int id) {
        return findUser(id).flat_map(getUserEmail);
    });

    std::cout << "User email (flat_map): " << userEmailFlat.value_or("Not found") << std::endl;

    return 0;
}
```

### Using or_else and transform_or

```cpp
#include "optional.hpp"
#include <iostream>
#include <string>

atom::type::Optional<int> getScore(const std::string& playerName) {
    if (playerName == "Alice") {
        return 100;
    } else if (playerName == "Bob") {
        return 85;
    }
    return std::nullopt;
}

int main() {
    atom::type::Optional<std::string> player1("Alice");
    atom::type::Optional<std::string> player2("Charlie");

    auto score1 = player1.and_then(getScore).or_else([]() { return 0; });
    std::cout << "Player 1 score: " << score1 << std::endl;

    auto score2 = player2.and_then(getScore).or_else([]() { return 0; });
    std::cout << "Player 2 score: " << score2 << std::endl;

    // Using transform_or
    auto gradePlayer = [](int score) { return score >= 90 ? 'A' : 'B'; };

    auto grade1 = player1.and_then(getScore).transform_or(gradePlayer, 'F');
    std::cout << "Player 1 grade: " << grade1.value() << std::endl;

    auto grade2 = player2.and_then(getScore).transform_or(gradePlayer, 'F');
    std::cout << "Player 2 grade: " << grade2.value() << std::endl;

    return 0;
}
```

### Exception Handling

```cpp
#include "optional.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    atom::type::Optional<int> opt;

    try {
        int value = opt.value();
    } catch (const std::runtime_error& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    // Using value_or to avoid exceptions
    int safeValue = opt.value_or(-1);
    std::cout << "Safe value: " << safeValue << std::endl;

    return 0;
}
```

### Comparison and Ordering

```cpp
#include "optional.hpp"
#include <iostream>

int main() {
    atom::type::Optional<int> a(5);
    atom::type::Optional<int> b(10);
    atom::type::Optional<int> c;

    std::cout << "a < b: " << (a < b) << std::endl;
    std::cout << "a > c: " << (a > c) << std::endl;
    std::cout << "b == 10: " << (b == atom::type::Optional<int>(10)) << std::endl;
    std::cout << "c == nullopt: " << (c == std::nullopt) << std::endl;

    return 0;
}
```

## Best Practices and Considerations

1. **Null Checking**: Always check if an `Optional` contains a value before accessing it to avoid exceptions.

   ```cpp
   atom::type::Optional<int> opt;
   if (opt) {
       std::cout << "Value: " << *opt << std::endl;
   }
   ```

2. **Use value_or for Default Values**: When you need a default value if the `Optional` is empty, use `value_or` instead of manually checking and providing a default.

   ```cpp
   atom::type::Optional<std::string> name;
   std::cout << "Name: " << name.value_or("Unknown") << std::endl;
   ```

3. **Chaining Operations**: Use methods like `map`, `and_then`, and `transform` to chain operations on `Optional` values without explicitly checking for emptiness at each step.

   ```cpp
   auto result = getValue()
       .map(processValue)
       .and_then(validateResult)
       .value_or(defaultValue);
   ```

4. **Avoid Overuse**: While `Optional` is useful for representing values that may or may not be present, overusing it can lead to complex and hard-to-read code. Use it judiciously.

5. **Consider Performance**: `Optional` adds a small overhead compared to raw values. In performance-critical code, consider alternatives if the "not present" state can be represented differently.

6. **Move Semantics**: Take advantage of move semantics when working with `Optional` containing move-only types or when performance is critical.

   ```cpp
   atom::type::Optional<std::unique_ptr<int>> opt = std::make_unique<int>(42);
   auto ptr = std::move(*opt);
   ```

7. **Exception Handling**: Be prepared to handle exceptions when calling `value()` on an empty `Optional`. Use `value_or()` or check for presence using the boolean conversion operator to avoid exceptions.

8. **Comparison with std::optional**: While this `Optional` class provides additional functionality, `std::optional` might be more appropriate in some contexts, especially when interfacing with standard library functions or third-party code.

9. **Custom Types**: When using custom types with `Optional`, ensure they properly support the operations required by `Optional`, such as move constructors and assignment operators.

10. **Const Correctness**: Pay attention to const correctness when working with `Optional`. Use the const versions of methods (e.g., `value() const&`) when appropriate.

## Conclusion

The `atom::type::Optional` class template provides a powerful tool for handling optional values in C++. By leveraging its methods and following best practices, you can write more expressive and safer code when dealing with values that may or may not be present. Remember to consider the trade-offs between using `Optional` and other approaches, and choose the most appropriate solution for your specific use case.
