# BoxedValue Class Documentation

## Overview

The `BoxedValue` class is a versatile container for storing and managing values of various types. It is defined in the `atom::meta` namespace and is part of the `any.hpp` header file. This class provides enhanced functionality over `std::any`, including type information, attributes, and thread-safe operations.

## Class Declaration

```cpp
namespace atom::meta {
class BoxedValue {
    // ... (member functions and private members)
};
}
```

## Key Features

- Type-erased value storage using `std::any`
- Thread-safe operations with shared mutex
- Support for attributes
- Type information storage
- Reference and const-correctness support
- Void type representation

## Public Member Functions

### Constructors

```cpp
template <typename T>
    requires(!std::same_as<BoxedValue, std::decay_t<T>>)
explicit BoxedValue(T&& value, bool return_value = false, bool readonly = false);

BoxedValue();  // Default constructor for void type

explicit BoxedValue(std::shared_ptr<Data> data);

BoxedValue(const BoxedValue& other);  // Copy constructor

BoxedValue(BoxedValue&& other) noexcept;  // Move constructor

template <typename T>
explicit BoxedValue(const T& value);  // Const value constructor
```

### Assignment Operators

```cpp
auto operator=(const BoxedValue& other) -> BoxedValue&;  // Copy assignment

auto operator=(BoxedValue&& other) noexcept -> BoxedValue&;  // Move assignment

template <typename T>
    requires(!std::same_as<BoxedValue, std::decay_t<T>>)
auto operator=(T&& value) -> BoxedValue&;  // Value assignment
```

### Utility Functions

```cpp
void swap(BoxedValue& rhs) noexcept;

[[nodiscard]] auto isUndef() const noexcept -> bool;
[[nodiscard]] auto isConst() const noexcept -> bool;
[[nodiscard]] auto isType(const TypeInfo& type_info) const noexcept -> bool;
[[nodiscard]] auto isRef() const noexcept -> bool;
[[nodiscard]] auto isReturnValue() const noexcept -> bool;
[[nodiscard]] auto isReadonly() const noexcept -> bool;
[[nodiscard]] auto isConstDataPtr() const noexcept -> bool;
[[nodiscard]] auto isNull() const noexcept -> bool;

void resetReturnValue() noexcept;

[[nodiscard]] auto get() const noexcept -> const std::any&;
[[nodiscard]] auto getTypeInfo() const noexcept -> const TypeInfo&;
[[nodiscard]] auto getPtr() const noexcept -> void*;

template <typename T>
[[nodiscard]] auto tryCast() const noexcept -> std::optional<T>;

template <typename T>
[[nodiscard]] auto canCast() const noexcept -> bool;

[[nodiscard]] auto debugString() const -> std::string;
```

### Attribute Handling

```cpp
auto setAttr(const std::string& name, const BoxedValue& value) -> BoxedValue&;
[[nodiscard]] auto getAttr(const std::string& name) const -> BoxedValue;
[[nodiscard]] auto hasAttr(const std::string& name) const -> bool;
void removeAttr(const std::string& name);
[[nodiscard]] auto listAttrs() const -> std::vector<std::string>;
```

## Helper Functions

```cpp
template <typename T>
auto var(T&& value) -> BoxedValue;

template <typename T>
auto constVar(const T& value) -> BoxedValue;

inline auto voidVar() -> BoxedValue;

template <typename T>
auto makeBoxedValue(T&& value, bool is_return_value = false, bool readonly = false) -> BoxedValue;
```

## Usage Examples

### Basic Usage

```cpp
#include "any.hpp"
#include <iostream>

int main() {
    atom::meta::BoxedValue intValue(42);
    atom::meta::BoxedValue stringValue("Hello, BoxedValue!");

    std::cout << intValue.debugString() << std::endl;
    std::cout << stringValue.debugString() << std::endl;

    if (auto intPtr = intValue.tryCast<int>()) {
        std::cout << "Int value: " << *intPtr << std::endl;
    }

    if (auto stringPtr = stringValue.tryCast<std::string>()) {
        std::cout << "String value: " << *stringPtr << std::endl;
    }

    return 0;
}
```

### Using Attributes

```cpp
atom::meta::BoxedValue person("John Doe");
person.setAttr("age", atom::meta::BoxedValue(30));
person.setAttr("city", atom::meta::BoxedValue("New York"));

if (auto age = person.getAttr("age").tryCast<int>()) {
    std::cout << "Age: " << *age << std::endl;
}

auto attrs = person.listAttrs();
for (const auto& attr : attrs) {
    std::cout << "Attribute: " << attr << std::endl;
}
```

### Using Helper Functions

```cpp
auto intValue = atom::meta::var(42);
auto constIntValue = atom::meta::constVar(42);
auto emptyValue = atom::meta::voidVar();

auto boxedInt = atom::meta::makeBoxedValue(42, true, false);
```

## Notes

- The `BoxedValue` class uses a shared mutex for thread-safe operations.
- It supports both value semantics and reference semantics.
- The class provides type-safe casting through the `tryCast` and `canCast` methods.
- Attributes can be used to associate additional data with a `BoxedValue` instance.
- The `debugString` method provides a string representation for debugging purposes.
- Helper functions like `var`, `constVar`, and `voidVar` provide convenient ways to create `BoxedValue` instances.
- The `makeBoxedValue` function offers fine-grained control over the creation of `BoxedValue` instances.
