# Function Signature Parsing Utilities

This document describes the usage of the `FunctionSignature` class and the `parseFunctionDefinition` function from the `atom::meta` namespace, which provide utilities for parsing and representing function signatures.

## Table of Contents

1. [Introduction](#introduction)
2. [FunctionSignature Class](#functionsignature-class)
3. [parseFunctionDefinition Function](#parsefunctiondefinition-function)
4. [Usage Examples](#usage-examples)
5. [Limitations and Considerations](#limitations-and-considerations)

## Introduction

The `signature.hpp` file provides utilities for parsing and representing function signatures in a Python-like syntax. It includes the `FunctionSignature` class to store parsed signature information and the `parseFunctionDefinition` function to parse a function definition string.

## FunctionSignature Class

The `FunctionSignature` class represents a parsed function signature.

### Class Definition

```cpp
struct alignas(128) FunctionSignature {
public:
    constexpr FunctionSignature(
        std::string_view name,
        std::array<std::pair<std::string_view, std::string_view>, 2> parameters,
        std::optional<std::string_view> returnType);

    [[nodiscard]] auto getName() const -> std::string_view;
    [[nodiscard]] auto getParameters() const
        -> const std::array<std::pair<std::string_view, std::string_view>, 2>&;
    [[nodiscard]] auto getReturnType() const
        -> std::optional<std::string_view>;

private:
    std::string_view name_;
    std::array<std::pair<std::string_view, std::string_view>, 2> parameters_;
    std::optional<std::string_view> returnType_;
};
```

### Member Functions

- `getName()`: Returns the function name.
- `getParameters()`: Returns an array of parameter name-type pairs.
- `getReturnType()`: Returns the optional return type.

## parseFunctionDefinition Function

The `parseFunctionDefinition` function parses a function definition string and returns an optional `FunctionSignature`.

### Function Signature

```cpp
constexpr auto parseFunctionDefinition(
    const std::string_view DEFINITION) noexcept
    -> std::optional<FunctionSignature>;
```

### Parameters

- `DEFINITION`: A string view containing the function definition to parse.

### Return Value

Returns an `std::optional<FunctionSignature>`. If parsing is successful, it contains the parsed `FunctionSignature`. If parsing fails, it returns `std::nullopt`.

## Usage Examples

Here are some examples demonstrating how to use the `parseFunctionDefinition` function and work with `FunctionSignature`:

### Example 1: Basic Function Parsing

```cpp
#include "signature.hpp"
#include <iostream>

int main() {
    constexpr std::string_view functionDef = "def add(a: int, b: int) -> int";

    if (auto signature = atom::meta::parseFunctionDefinition(functionDef)) {
        std::cout << "Function Name: " << signature->getName() << std::endl;
        std::cout << "Parameters:" << std::endl;
        for (const auto& [name, type] : signature->getParameters()) {
            if (!name.empty()) {
                std::cout << "  " << name << ": " << type << std::endl;
            }
        }
        if (auto returnType = signature->getReturnType()) {
            std::cout << "Return Type: " << *returnType << std::endl;
        }
    } else {
        std::cout << "Failed to parse function definition." << std::endl;
    }

    return 0;
}
```

Expected output:

```
Function Name: add
Parameters:
  a: int
  b: int
Return Type: int
```

### Example 2: Function with Default Parameter Types

```cpp
#include "signature.hpp"
#include <iostream>

int main() {
    constexpr std::string_view functionDef = "def greet(name, greeting='Hello')";

    if (auto signature = atom::meta::parseFunctionDefinition(functionDef)) {
        std::cout << "Function Name: " << signature->getName() << std::endl;
        std::cout << "Parameters:" << std::endl;
        for (const auto& [name, type] : signature->getParameters()) {
            if (!name.empty()) {
                std::cout << "  " << name << ": " << type << std::endl;
            }
        }
        if (auto returnType = signature->getReturnType()) {
            std::cout << "Return Type: " << *returnType << std::endl;
        }
    } else {
        std::cout << "Failed to parse function definition." << std::endl;
    }

    return 0;
}
```

Expected output:

```
Function Name: greet
Parameters:
  name: any
  greeting: any
Return Type: none
```

### Example 3: Function with Complex Parameter Types

```cpp
#include "signature.hpp"
#include <iostream>

int main() {
    constexpr std::string_view functionDef = "def process(data: List[int], options: Dict[str, Any]) -> bool";

    if (auto signature = atom::meta::parseFunctionDefinition(functionDef)) {
        std::cout << "Function Name: " << signature->getName() << std::endl;
        std::cout << "Parameters:" << std::endl;
        for (const auto& [name, type] : signature->getParameters()) {
            if (!name.empty()) {
                std::cout << "  " << name << ": " << type << std::endl;
            }
        }
        if (auto returnType = signature->getReturnType()) {
            std::cout << "Return Type: " << *returnType << std::endl;
        }
    } else {
        std::cout << "Failed to parse function definition." << std::endl;
    }

    return 0;
}
```

Expected output:

```
Function Name: process
Parameters:
  data: List[int]
  options: Dict[str, Any]
Return Type: bool
```

## Limitations and Considerations

1. The `FunctionSignature` class is limited to storing up to two parameters. For functions with more parameters, only the first two will be stored.

2. The parsing is designed for Python-like function definitions. It may not work correctly for other syntaxes.

3. The parser assumes that the function definition starts with "def " and follows a specific format. It may fail to parse correctly if the input doesn't match this expected format.

4. The parsing is done at compile-time using `constexpr`, which allows for efficient runtime performance but may increase compile times for complex function definitions.

5. Error handling is done through the use of `std::optional`. If parsing fails, `std::nullopt` is returned instead of throwing an exception.

6. The parser uses a simple approach to handle nested brackets in parameter types, but it may not handle all complex cases correctly.

7. The `trim` function used in the parser is assumed to be defined in the `atom::utils` namespace. Make sure this utility function is available in your project.
