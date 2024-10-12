# AsyncGlob Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor](#constructor)
4. [Public Methods](#public-methods)
5. [Usage Examples](#usage-examples)
6. [Best Practices](#best-practices)
7. [Advanced Topics](#advanced-topics)

## Introduction

The `AsyncGlob` class, part of the `atom::io` namespace, provides asynchronous file and directory pattern matching functionality similar to the glob module in Python. It allows for efficient file system traversal and pattern matching using the ASIO library for asynchronous operations.

## Class Overview

The `AsyncGlob` class is designed to perform glob operations asynchronously, which is particularly useful for searching large directory structures or when dealing with slow file systems.

## Constructor

```cpp
AsyncGlob(asio::io_context& io_context);
```

Creates an `AsyncGlob` object using the provided ASIO I/O context.

- `io_context`: Reference to an `asio::io_context` object for managing asynchronous operations.

## Public Methods

### glob

```cpp
void glob(const std::string& pathname,
          const std::function<void(std::vector<fs::path>)>& callback,
          bool recursive = false,
          bool dironly = false);
```

Performs an asynchronous glob operation.

- `pathname`: The pattern to match against file and directory names.
- `callback`: A function to be called with the results of the glob operation.
- `recursive`: If true, the glob operation will search recursively through subdirectories.
- `dironly`: If true, only directories will be matched.

## Usage Examples

### Basic Usage

```cpp
#include "async_glob.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;
    atom::io::AsyncGlob glob(io_context);

    glob.glob("*.txt", [](const std::vector<fs::path>& results) {
        for (const auto& path : results) {
            std::cout << "Found: " << path << std::endl;
        }
    });

    io_context.run();
    return 0;
}
```

This example searches for all `.txt` files in the current directory and prints their paths.

### Recursive Search

```cpp
glob.glob("**/*.cpp", [](const std::vector<fs::path>& results) {
    std::cout << "Found " << results.size() << " C++ files:" << std::endl;
    for (const auto& path : results) {
        std::cout << path << std::endl;
    }
}, true);  // Set recursive to true
```

This example recursively searches for all `.cpp` files in the current directory and its subdirectories.

### Directory-Only Search

```cpp
glob.glob("*", [](const std::vector<fs::path>& results) {
    std::cout << "Found directories:" << std::endl;
    for (const auto& path : results) {
        std::cout << path << std::endl;
    }
}, false, true);  // Set dironly to true
```

This example searches for all directories in the current directory.

### Complex Pattern Matching

```cpp
glob.glob("src/{lib,bin}/*.{h,cpp}", [](const std::vector<fs::path>& results) {
    std::cout << "Found source files:" << std::endl;
    for (const auto& path : results) {
        std::cout << path << std::endl;
    }
});
```

This example demonstrates a more complex pattern that matches `.h` and `.cpp` files in both `src/lib` and `src/bin` directories.

## Best Practices

1. **Error Handling**: Always implement proper error handling in your callback function to deal with potential issues during the glob operation.

2. **Performance Considerations**: For large directory structures, consider using the recursive option judiciously, as it may impact performance.

3. **Pattern Design**: Craft your glob patterns carefully to minimize unnecessary file system traversals.

4. **Callback Execution**: Remember that the callback function will be executed asynchronously. Ensure any shared resources accessed in the callback are properly synchronized.

5. **I/O Context Management**: Make sure the `asio::io_context` runs for the duration of the glob operation. You may need to use `io_context.run()` or `asio::io_context::work` to keep it active.

## Advanced Topics

### Custom Filtering

While the `AsyncGlob` class provides powerful pattern matching capabilities, you might need to implement additional filtering logic in your callback function for more complex requirements.

```cpp
glob.glob("*.log", [](const std::vector<fs::path>& results) {
    for (const auto& path : results) {
        if (fs::file_size(path) > 1024 * 1024) {  // Check if file is larger than 1MB
            std::cout << "Large log file: " << path << std::endl;
        }
    }
});
```

### Integration with Other Asynchronous Operations

The `AsyncGlob` class can be easily integrated with other asynchronous operations in your application:

```cpp
asio::io_context io_context;
atom::io::AsyncGlob glob(io_context);

// Start a timer
asio::steady_timer timer(io_context, asio::chrono::seconds(5));
timer.async_wait([&glob](const asio::error_code& ec) {
    if (!ec) {
        glob.glob("*.dat", [](const std::vector<fs::path>& results) {
            // Process results
        });
    }
});

io_context.run();
```
