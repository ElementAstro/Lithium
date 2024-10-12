# Exception Class Documentation

## Overview

The `Exception` class is part of the `atom::error` namespace and provides an enhanced exception handling mechanism. It extends the standard `std::exception` class with additional features such as file, line, and function information, as well as stack trace capture.

## Class Definition

```cpp
namespace atom::error {
class Exception : public std::exception {
public:
    template <typename... Args>
    Exception(const char *file, int line, const char *func, Args &&...args);

    template <typename... Args>
    static void rethrowNested(Args &&...args);

    auto what() const ATOM_NOEXCEPT -> const char * override;
    auto getFile() const -> std::string;
    auto getLine() const -> int;
    auto getFunction() const -> std::string;
    auto getMessage() const -> std::string;
    auto getThreadId() const -> std::thread::id;

private:
    std::string file_;
    int line_;
    std::string func_;
    std::string message_;
    mutable std::string full_message_;
    std::thread::id thread_id_;
    StackTrace stack_trace_;
};
}
```

## Constructor

### `Exception(const char *file, int line, const char *func, Args &&...args)`

Constructs an `Exception` object with detailed information about where and why the exception occurred.

#### Parameters

- `file`: The file where the exception occurred.
- `line`: The line number in the file where the exception occurred.
- `func`: The function where the exception occurred.
- `args`: Additional arguments to provide context for the exception (variadic template).

## Static Methods

### `static void rethrowNested(Args &&...args)`

Rethrows the current exception as a nested exception within a new `Exception` object.

#### Usage

This method is typically used in catch blocks to add more context to an existing exception before rethrowing it.

## Public Methods

### `auto what() const ATOM_NOEXCEPT -> const char *`

Returns a C-style string describing the exception. This method overrides the `std::exception::what()` method.

### `auto getFile() const -> std::string`

Returns the file where the exception occurred.

### `auto getLine() const -> int`

Returns the line number where the exception occurred.

### `auto getFunction() const -> std::string`

Returns the function where the exception occurred.

### `auto getMessage() const -> std::string`

Returns the message associated with the exception.

### `auto getThreadId() const -> std::thread::id`

Returns the ID of the thread where the exception occurred.

## Usage Examples

### Basic Usage

```cpp
#include "exception.hpp"
#include <iostream>

void riskyFunction() {
    throw atom::error::Exception(__FILE__, __LINE__, __FUNCTION__, "An error occurred");
}

int main() {
    try {
        riskyFunction();
    } catch (const atom::error::Exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        std::cerr << "File: " << e.getFile() << std::endl;
        std::cerr << "Line: " << e.getLine() << std::endl;
        std::cerr << "Function: " << e.getFunction() << std::endl;
        std::cerr << "Thread ID: " << e.getThreadId() << std::endl;
    }
    return 0;
}
```

### Using rethrowNested

```cpp
#include "exception.hpp"
#include <iostream>

void innerFunction() {
    throw std::runtime_error("Inner error");
}

void outerFunction() {
    try {
        innerFunction();
    } catch (...) {
        atom::error::Exception::rethrowNested(__FILE__, __LINE__, __FUNCTION__, "Outer error");
    }
}

int main() {
    try {
        outerFunction();
    } catch (const atom::error::Exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        try {
            std::rethrow_if_nested(e);
        } catch (const std::runtime_error& nested) {
            std::cerr << "Nested exception: " << nested.what() << std::endl;
        }
    }
    return 0;
}
```

## Best Practices

1. **Use Macros**: Consider defining macros for common exception throwing patterns to reduce boilerplate code:

   ```cpp
   #define THROW_EXCEPTION(...) \
       throw atom::error::Exception(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
   ```

2. **Catch by Reference**: Always catch exceptions by const reference to avoid slicing:

   ```cpp
   try {
       // risky code
   } catch (const atom::error::Exception& e) {
       // handle exception
   }
   ```

3. **Leverage Stack Trace**: The `Exception` class includes a `StackTrace` object. Make use of this for detailed debugging:

   ```cpp
   catch (const atom::error::Exception& e) {
       std::cerr << "Exception stack trace:\n" << e.getStackTrace() << std::endl;
   }
   ```

4. **Thread Safety**: Be aware that the `Exception` class captures the thread ID. This can be useful for debugging multi-threaded applications.

5. **Nested Exceptions**: Use `rethrowNested` when you want to add context to an exception while preserving the original error information.

## Notes

- The `Exception` class is designed to provide rich context for debugging and error handling.
- It automatically captures the stack trace at the point of exception creation.
- The `ATOM_NOEXCEPT` macro is used for the `what()` method, which should be defined in your project to handle noexcept specifications correctly.
