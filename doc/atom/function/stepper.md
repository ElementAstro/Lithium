# FunctionSequence Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Member Functions](#member-functions)
4. [Usage Examples](#usage-examples)
5. [Error Handling](#error-handling)
6. [Best Practices](#best-practices)

## Introduction

The `FunctionSequence` class, defined in the `atom::meta` namespace, provides a mechanism to register and execute a sequence of functions. This class is particularly useful for creating pipelines of operations where each function can be executed either independently or as part of a sequence.

## Class Overview

```cpp
namespace atom::meta {
class FunctionSequence {
public:
    using FunctionType = std::function<std::any(const std::vector<std::any>&)>;

    void registerFunction(FunctionType func);
    auto run(const std::vector<std::vector<std::any>>& argsBatch) -> std::vector<std::any>;
    auto runAll(const std::vector<std::vector<std::any>>& argsBatch) -> std::vector<std::vector<std::any>>;

private:
    std::vector<FunctionType> functions_;
};
}
```

## Member Functions

### `registerFunction`

```cpp
void registerFunction(FunctionType func)
```

Registers a function to be part of the sequence.

- **Parameters:**
  - `func`: A function object that takes a `const std::vector<std::any>&` and returns `std::any`.

### `run`

```cpp
auto run(const std::vector<std::vector<std::any>>& argsBatch) -> std::vector<std::any>
```

Runs the last registered function with each set of arguments provided.

- **Parameters:**
  - `argsBatch`: A vector of argument vectors, where each inner vector represents a set of arguments for a single function call.
- **Returns:**
  - A vector of `std::any`, containing the results of applying the last function to each set of arguments.

### `runAll`

```cpp
auto runAll(const std::vector<std::vector<std::any>>& argsBatch) -> std::vector<std::vector<std::any>>
```

Runs all registered functions with each set of arguments provided.

- **Parameters:**
  - `argsBatch`: A vector of argument vectors, where each inner vector represents a set of arguments for a single function call.
- **Returns:**
  - A vector of vectors of `std::any`, where each inner vector contains the results of applying all functions to a single set of arguments.

## Usage Examples

### Example 1: Basic Usage

```cpp
#include "stepper.hpp"
#include <iostream>

int main() {
    atom::meta::FunctionSequence sequence;

    // Register functions
    sequence.registerFunction([](const std::vector<std::any>& args) {
        int x = std::any_cast<int>(args[0]);
        return std::any(x * 2);
    });

    sequence.registerFunction([](const std::vector<std::any>& args) {
        int x = std::any_cast<int>(args[0]);
        return std::any(x + 10);
    });

    // Prepare arguments
    std::vector<std::vector<std::any>> argsBatch = {{5}, {10}};

    // Run the last function
    auto results = sequence.run(argsBatch);

    for (const auto& result : results) {
        std::cout << std::any_cast<int>(result) << std::endl;
    }

    return 0;
}
```

Output:

```
15
20
```

### Example 2: Running All Functions

```cpp
#include "stepper.hpp"
#include <iostream>

int main() {
    atom::meta::FunctionSequence sequence;

    // Register functions
    sequence.registerFunction([](const std::vector<std::any>& args) {
        int x = std::any_cast<int>(args[0]);
        return std::any(x * 2);
    });

    sequence.registerFunction([](const std::vector<std::any>& args) {
        int x = std::any_cast<int>(args[0]);
        return std::any(x + 10);
    });

    // Prepare arguments
    std::vector<std::vector<std::any>> argsBatch = {{5}, {10}};

    // Run all functions
    auto resultsBatch = sequence.runAll(argsBatch);

    for (const auto& results : resultsBatch) {
        for (const auto& result : results) {
            std::cout << std::any_cast<int>(result) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
```

Output:

```
10 15
20 20
```

## Error Handling

The `FunctionSequence` class uses exception handling to manage errors. If an exception is thrown during the execution of a function, it will be caught and re-thrown with additional context.

```cpp
try {
    auto results = sequence.run(argsBatch);
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Best Practices

1. **Type Safety**: Since `std::any` is used, be cautious about type casting. Always ensure that the types match when casting from `std::any`.

2. **Error Handling**: Wrap calls to `run` and `runAll` in try-catch blocks to handle potential exceptions.

3. **Function Design**: Design your functions to handle `std::any` inputs and outputs appropriately. Consider using helper functions for type checking and casting.

4. **Performance**: Be aware that using `std::any` and function objects may have some performance overhead. For performance-critical applications, consider alternative designs.

5. **Flexibility**: The `FunctionSequence` class allows for great flexibility in creating function pipelines. Use this to your advantage to create modular and reusable code.
