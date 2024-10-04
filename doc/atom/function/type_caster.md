# TypeCaster Class Documentation

The `TypeCaster` class is part of the `atom::meta` namespace and provides advanced type casting functionality with features such as type inference, aliasing, multi-stage conversion, and logging. This document explains its usage and provides examples.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor](#constructor)
3. [Static Factory Method](#static-factory-method)
4. [Key Methods](#key-methods)
5. [Usage Examples](#usage-examples)

## Class Overview

The `TypeCaster` class is designed to handle complex type conversions in C++ applications. It allows for registering custom type conversions, creating type aliases, grouping types, and performing multi-stage conversions.

## Constructor

```cpp
TypeCaster()
```

The default constructor initializes a `TypeCaster` object and registers built-in types automatically.

## Static Factory Method

```cpp
static auto createShared() -> std::shared_ptr<TypeCaster>
```

This static method creates and returns a shared pointer to a new `TypeCaster` instance.

## Key Methods

### 1. Convert

```cpp
template <typename DestinationType>
auto convert(const std::any& input) const -> std::any
```

Converts an input of any type to the specified destination type.

- **Parameters:**
  - `input`: The input value to be converted (of type `std::any`)
- **Returns:** The converted value as `std::any`
- **Throws:** `std::invalid_argument` if the source type is not found

### 2. Register Conversion

```cpp
template <typename SourceType, typename DestinationType>
void registerConversion(ConvertFunc func)
```

Registers a conversion function between two types.

- **Parameters:**
  - `func`: The conversion function
- **Throws:** `std::invalid_argument` if the source and destination types are the same

### 3. Register Alias

```cpp
template <typename T>
void registerAlias(const std::string& alias)
```

Registers an alias for a type.

- **Parameters:**
  - `alias`: The alias name

### 4. Register Type Group

```cpp
void registerTypeGroup(const std::string& groupName, const std::vector<std::string>& types)
```

Registers a group of types under a common group name.

- **Parameters:**
  - `groupName`: The name of the group
  - `types`: The list of type names to group

### 5. Register Multi-Stage Conversion

```cpp
template <typename IntermediateType, typename SourceType, typename DestinationType>
void registerMultiStageConversion(ConvertFunc func1, ConvertFunc func2)
```

Registers a multi-stage conversion function.

- **Parameters:**
  - `func1`: The first stage conversion function
  - `func2`: The second stage conversion function

### 6. Has Conversion

```cpp
auto hasConversion(TypeInfo src, TypeInfo dst) const -> bool
```

Checks if a conversion exists between two types.

- **Parameters:**
  - `src`: The source type
  - `dst`: The destination type
- **Returns:** `true` if a conversion exists, `false` otherwise

### 7. Get Registered Types

```cpp
auto getRegisteredTypes() const -> std::vector<std::string>
```

Gets a list of registered types.

- **Returns:** A vector of registered type names

### 8. Register Type

```cpp
template <typename T>
void registerType(const std::string& name)
```

Registers a type with a given name.

- **Parameters:**
  - `name`: The name to register the type with

### 9. Register Enum Value

```cpp
template <typename EnumType>
void registerEnumValue(const std::string& enum_name, const std::string& string_value, EnumType enum_value)
```

Registers an enum value with its string representation.

- **Parameters:**
  - `enum_name`: The name of the enum type
  - `string_value`: The string representation of the enum value
  - `enum_value`: The actual enum value

### 10. Get Enum Map

```cpp
template <typename EnumType>
auto getEnumMap(const std::string& enum_name) const -> const std::unordered_map<std::string, EnumType>&
```

Retrieves the enum map for a given enum type.

- **Parameters:**
  - `enum_name`: The name of the enum type
- **Returns:** A const reference to the enum map

### 11. Enum to String

```cpp
template <typename EnumType>
auto enumToString(EnumType value, const std::string& enum_name) -> std::string
```

Converts an enum value to its string representation.

- **Parameters:**
  - `value`: The enum value to convert
  - `enum_name`: The name of the enum type
- **Returns:** The string representation of the enum value
- **Throws:** `std::invalid_argument` if the enum value is not found

### 12. String to Enum

```cpp
template <typename EnumType>
auto stringToEnum(const std::string& string_value, const std::string& enum_name) -> EnumType
```

Converts a string to its corresponding enum value.

- **Parameters:**
  - `string_value`: The string representation of the enum value
  - `enum_name`: The name of the enum type
- **Returns:** The corresponding enum value
- **Throws:** `std::invalid_argument` if the string value is not found

## Usage Examples

Here are some examples demonstrating how to use the `TypeCaster` class:

### Basic Usage

```cpp
#include "type_caster.hpp"
#include <iostream>

int main() {
    auto caster = atom::meta::TypeCaster::createShared();

    // Convert an integer to a string
    int intValue = 42;
    std::any result = caster->convert<std::string>(std::any(intValue));
    std::cout << std::any_cast<std::string>(result) << std::endl;  // Outputs: "42"

    return 0;
}
```

### Registering Custom Conversions

```cpp
#include "type_caster.hpp"
#include <iostream>

struct CustomType {
    int value;
    explicit CustomType(int v) : value(v) {}
};

int main() {
    auto caster = atom::meta::TypeCaster::createShared();

    // Register a custom conversion from CustomType to int
    caster->registerConversion<CustomType, int>([](const std::any& input) {
        return std::any(std::any_cast<CustomType>(input).value);
    });

    // Use the custom conversion
    CustomType customValue(123);
    std::any result = caster->convert<int>(std::any(customValue));
    std::cout << std::any_cast<int>(result) << std::endl;  // Outputs: 123

    return 0;
}
```

### Using Type Aliases

```cpp
#include "type_caster.hpp"
#include <iostream>

int main() {
    auto caster = atom::meta::TypeCaster::createShared();

    // Register an alias for int
    caster->registerAlias<int>("Integer");

    // Use the alias in a conversion
    double doubleValue = 3.14;
    std::any result = caster->convert<Integer>(std::any(doubleValue));
    std::cout << std::any_cast<int>(result) << std::endl;  // Outputs: 3

    return 0;
}
```

### Multi-Stage Conversion

```cpp
#include "type_caster.hpp"
#include <iostream>
#include <string>

int main() {
    auto caster = atom::meta::TypeCaster::createShared();

    // Register a multi-stage conversion from double to string via int
    caster->registerMultiStageConversion<int, double, std::string>(
        [](const std::any& input) {
            return std::any(static_cast<int>(std::any_cast<double>(input)));
        },
        [](const std::any& input) {
            return std::any(std::to_string(std::any_cast<int>(input)));
        }
    );

    // Use the multi-stage conversion
    double doubleValue = 3.14159;
    std::any result = caster->convert<std::string>(std::any(doubleValue));
    std::cout << std::any_cast<std::string>(result) << std::endl;  // Outputs: "3"

    return 0;
}
```

### Working with Enums

```cpp
#include "type_caster.hpp"
#include <iostream>

enum class Color { Red, Green, Blue };

int main() {
    auto caster = atom::meta::TypeCaster::createShared();

    // Register enum values
    caster->registerEnumValue<Color>("Color", "Red", Color::Red);
    caster->registerEnumValue<Color>("Color", "Green", Color::Green);
    caster->registerEnumValue<Color>("Color", "Blue", Color::Blue);

    // Convert enum to string
    Color color = Color::Green;
    std::string colorStr = caster->enumToString(color, "Color");
    std::cout << "Color enum to string: " << colorStr << std::endl;  // Outputs: "Green"

    // Convert string to enum
    Color convertedColor = caster->stringToEnum<Color>("Blue", "Color");
    std::cout << "String to Color enum: " << static_cast<int>(convertedColor) << std::endl;  // Outputs: 2

    return 0;
}
```

### Type Groups

```cpp
#include "type_caster.hpp"
#include <iostream>
#include <vector>

int main() {
    auto caster = atom::meta::TypeCaster::createShared();

    // Register a type group for numeric types
    caster->registerTypeGroup("Numeric", {"int", "float", "double"});

    // Check registered types
    std::vector<std::string> registeredTypes = caster->getRegisteredTypes();
    std::cout << "Registered types:" << std::endl;
    for (const auto& type : registeredTypes) {
        std::cout << "- " << type << std::endl;
    }

    return 0;
}
```

## Best Practices

1. **Thread Safety**: The `TypeCaster` class uses a mutex to ensure thread safety. However, be cautious when using the class in multi-threaded environments, especially when registering new conversions or aliases.

2. **Error Handling**: Always handle potential exceptions, especially when using `convert`, `enumToString`, and `stringToEnum` methods, as they can throw exceptions for invalid inputs.

3. **Performance Considerations**: The `TypeCaster` class caches conversion paths for better performance. However, frequent registration of new conversions will clear this cache, potentially impacting performance.

4. **Type Safety**: While `TypeCaster` provides flexibility in type conversions, it's important to use it judiciously. Overuse of dynamic type casting can lead to less type-safe code and potential runtime errors.

5. **Custom Types**: When working with custom types, always register them and their conversions before using them with the `TypeCaster`.

6. **Enum Handling**: For better type safety when working with enums, consider using strongly typed enums (`enum class`) and registering them with `TypeCaster`.

## Conclusion

The `TypeCaster` class provides a powerful and flexible system for handling complex type conversions in C++ applications. By leveraging its features such as custom conversions, type aliases, and multi-stage conversions, developers can create more robust and adaptable code, especially when dealing with diverse data types and formats.

Remember to always consider the trade-offs between flexibility and type safety when using dynamic type casting, and use the `TypeCaster` class judiciously in your applications.
