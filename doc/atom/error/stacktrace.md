# StackTrace Class Documentation

## Overview

The `StackTrace` class is part of the `atom::error` namespace and provides functionality to capture and represent the current stack trace. This class is particularly useful for debugging and error reporting, as it allows you to obtain detailed information about the current execution context, including file names, line numbers, and symbols (when available).

## Class Definition

```cpp
namespace atom::error {
class StackTrace {
public:
    StackTrace();
    [[nodiscard]] auto toString() const -> std::string;

private:
    void capture();

    // Platform-specific members
    #ifdef _WIN32
        std::vector<void*> frames_;
    #elif defined(__APPLE__) || defined(__linux__)
        std::unique_ptr<char*, decltype(&free)> symbols_;
        std::vector<void*> frames_;
        int num_frames_ = 0;
    #endif
};
}
```

## Constructor

### `StackTrace()`

The default constructor for the `StackTrace` class. When a `StackTrace` object is created, it automatically captures the current stack trace.

## Public Methods

### `[[nodiscard]] auto toString() const -> std::string`

Returns a string representation of the captured stack trace. This method is marked with `[[nodiscard]]`, indicating that the return value should not be ignored.

#### Return Value

- A `std::string` containing the formatted stack trace information.

## Private Methods

### `void capture()`

This private method is responsible for capturing the current stack trace. The implementation varies depending on the operating system:

- On Windows, it populates the `frames_` vector with stack frame pointers.
- On macOS and Linux, it captures both the raw frame pointers and the corresponding symbols.

## Platform-Specific Members

### Windows

- `std::vector<void*> frames_`: Stores the stack frame pointers.

### macOS and Linux

- `std::unique_ptr<char*, decltype(&free)> symbols_`: Stores the stack symbols.
- `std::vector<void*> frames_`: Stores the raw stack frame pointers.
- `int num_frames_`: Keeps track of the number of captured stack frames.

## Usage Example

Here's an example of how to use the `StackTrace` class:

```cpp
#include "stacktrace.hpp"
#include <iostream>

void function_c() {
    atom::error::StackTrace stackTrace;
    std::cout << "Stack Trace:\n" << stackTrace.toString() << std::endl;
}

void function_b() {
    function_c();
}

void function_a() {
    function_b();
}

int main() {
    try {
        function_a();
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
    return 0;
}
```

In this example:

1. We create a chain of function calls: `main()` -> `function_a()` -> `function_b()` -> `function_c()`.
2. In `function_c()`, we create a `StackTrace` object and print its string representation.
3. The output will show the call stack, including file names, line numbers, and function names (if available).

## Notes

- The `StackTrace` class uses conditional compilation (`#ifdef`, `#elif`) to provide different implementations for Windows, macOS, and Linux.
- On macOS and Linux, it uses a smart pointer (`std::unique_ptr`) with a custom deleter to manage the memory for symbols.
- The class is designed to be easy to use, with the stack trace capture happening automatically upon object creation.

## Best Practices

- Use `StackTrace` objects in exception handlers or error logging scenarios to provide detailed context for debugging.
- Remember that capturing a stack trace can have a performance impact, so use it judiciously in performance-critical sections of your code.
- The `toString()` method is marked `[[nodiscard]]` to encourage checking its return value, which is particularly important for error handling and logging scenarios.
