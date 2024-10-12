# DirectoryStack Usage Guide

## Introduction

The `DirectoryStack` class is a powerful utility for managing a stack of directory paths with asynchronous support using Asio. It provides functionality to push, pop, and perform various operations on the stack of directories.

## Class Overview

```cpp
class DirectoryStack {
public:
    explicit DirectoryStack(asio::io_context& io_context);
    ~DirectoryStack();

    // ... (other method declarations)
};
```

The `DirectoryStack` class is designed with the following features:

- Asynchronous operations using Asio
- Move semantics support
- Copy operations are deleted to prevent unintended copies

## Constructor

```cpp
explicit DirectoryStack(asio::io_context& io_context);
```

Creates a new `DirectoryStack` instance.

**Parameters:**

- `io_context`: The Asio `io_context` to use for asynchronous operations

**Example:**

```cpp
asio::io_context io_context;
DirectoryStack dirStack(io_context);
```

## Asynchronous Operations

### Push Directory

```cpp
void asyncPushd(const std::filesystem::path& new_dir,
                const std::function<void(const std::error_code&)>& handler);
```

Pushes the current directory onto the stack and changes to the specified directory asynchronously.

**Parameters:**

- `new_dir`: The directory to change to
- `handler`: The completion handler to be called when the operation completes

**Example:**

```cpp
dirStack.asyncPushd("/path/to/new/dir", [](const std::error_code& ec) {
    if (!ec) {
        std::cout << "Successfully changed directory" << std::endl;
    } else {
        std::cerr << "Error: " << ec.message() << std::endl;
    }
});
```

### Pop Directory

```cpp
void asyncPopd(const std::function<void(const std::error_code&)>& handler);
```

Pops the top directory from the stack and changes back to it asynchronously.

**Parameters:**

- `handler`: The completion handler to be called when the operation completes

**Example:**

```cpp
dirStack.asyncPopd([](const std::error_code& ec) {
    if (!ec) {
        std::cout << "Successfully returned to previous directory" << std::endl;
    } else {
        std::cerr << "Error: " << ec.message() << std::endl;
    }
});
```

## Synchronous Operations

### Peek

```cpp
[[nodiscard]] auto peek() const -> std::filesystem::path;
```

Views the top directory in the stack without changing to it.

**Returns:** The top directory in the stack

**Example:**

```cpp
std::filesystem::path topDir = dirStack.peek();
std::cout << "Top directory: " << topDir << std::endl;
```

### Get All Directories

```cpp
[[nodiscard]] auto dirs() const -> std::vector<std::filesystem::path>;
```

Displays the current stack of directories.

**Returns:** A vector of directory paths in the stack

**Example:**

```cpp
std::vector<std::filesystem::path> allDirs = dirStack.dirs();
for (const auto& dir : allDirs) {
    std::cout << dir << std::endl;
}
```

### Clear Stack

```cpp
void clear();
```

Clears the directory stack.

**Example:**

```cpp
dirStack.clear();
std::cout << "Directory stack cleared" << std::endl;
```

### Swap Directories

```cpp
void swap(size_t index1, size_t index2);
```

Swaps two directories in the stack given their indices.

**Parameters:**

- `index1`: The first index
- `index2`: The second index

**Example:**

```cpp
dirStack.swap(1, 2);
std::cout << "Swapped directories at indices 1 and 2" << std::endl;
```

### Remove Directory

```cpp
void remove(size_t index);
```

Removes a directory from the stack at the specified index.

**Parameters:**

- `index`: The index of the directory to remove

**Example:**

```cpp
dirStack.remove(1);
std::cout << "Removed directory at index 1" << std::endl;
```

## Advanced Operations

### Go to Index

```cpp
void asyncGotoIndex(size_t index,
                    const std::function<void(const std::error_code&)>& handler);
```

Changes to the directory at the specified index in the stack asynchronously.

**Parameters:**

- `index`: The index of the directory to change to
- `handler`: The completion handler to be called when the operation completes

**Example:**

```cpp
dirStack.asyncGotoIndex(2, [](const std::error_code& ec) {
    if (!ec) {
        std::cout << "Changed to directory at index 2" << std::endl;
    } else {
        std::cerr << "Error: " << ec.message() << std::endl;
    }
});
```

### Save Stack to File

```cpp
void asyncSaveStackToFile(const std::string& filename,
                          const std::function<void(const std::error_code&)>& handler);
```

Saves the directory stack to a file asynchronously.

**Parameters:**

- `filename`: The name of the file to save the stack to
- `handler`: The completion handler to be called when the operation completes

**Example:**

```cpp
dirStack.asyncSaveStackToFile("dir_stack.txt", [](const std::error_code& ec) {
    if (!ec) {
        std::cout << "Stack saved to file successfully" << std::endl;
    } else {
        std::cerr << "Error saving stack: " << ec.message() << std::endl;
    }
});
```

### Load Stack from File

```cpp
void asyncLoadStackFromFile(const std::string& filename,
                            const std::function<void(const std::error_code&)>& handler);
```

Loads the directory stack from a file asynchronously.

**Parameters:**

- `filename`: The name of the file to load the stack from
- `handler`: The completion handler to be called when the operation completes

**Example:**

```cpp
dirStack.asyncLoadStackFromFile("dir_stack.txt", [](const std::error_code& ec) {
    if (!ec) {
        std::cout << "Stack loaded from file successfully" << std::endl;
    } else {
        std::cerr << "Error loading stack: " << ec.message() << std::endl;
    }
});
```

## Utility Methods

### Get Stack Size

```cpp
[[nodiscard]] auto size() const -> size_t;
```

Gets the size of the directory stack.

**Returns:** The number of directories in the stack

**Example:**

```cpp
size_t stackSize = dirStack.size();
std::cout << "Stack size: " << stackSize << std::endl;
```

### Check if Stack is Empty

```cpp
[[nodiscard]] auto isEmpty() const -> bool;
```

Checks if the directory stack is empty.

**Returns:** True if the stack is empty, false otherwise

**Example:**

```cpp
if (dirStack.isEmpty()) {
    std::cout << "Stack is empty" << std::endl;
} else {
    std::cout << "Stack is not empty" << std::endl;
}
```

### Get Current Directory

```cpp
void asyncGetCurrentDirectory(const std::function<void(const std::filesystem::path&)>& handler) const;
```

Gets the current directory path asynchronously.

**Parameters:**

- `handler`: The completion handler to be called with the current directory path

**Example:**

```cpp
dirStack.asyncGetCurrentDirectory([](const std::filesystem::path& currentPath) {
    std::cout << "Current directory: " << currentPath << std::endl;
});
```

## Complete Usage Example

Here's a more comprehensive example demonstrating the usage of various `DirectoryStack` methods:

```cpp
#include <iostream>
#include <asio.hpp>
#include "DirectoryStack.h"

int main() {
    asio::io_context io_context;
    DirectoryStack dirStack(io_context);

    // Push a new directory
    dirStack.asyncPushd("/path/to/project", [&](const std::error_code& ec) {
        if (!ec) {
            std::cout << "Changed to project directory" << std::endl;

            // Push another directory
            dirStack.asyncPushd("src", [&](const std::error_code& ec) {
                if (!ec) {
                    std::cout << "Changed to src directory" << std::endl;

                    // Display current stack
                    auto dirs = dirStack.dirs();
                    std::cout << "Current stack:" << std::endl;
                    for (const auto& dir : dirs) {
                        std::cout << "  " << dir << std::endl;
                    }

                    // Pop back to previous directory
                    dirStack.asyncPopd([&](const std::error_code& ec) {
                        if (!ec) {
                            std::cout << "Returned to project directory" << std::endl;

                            // Save stack to file
                            dirStack.asyncSaveStackToFile("dir_stack.txt", [](const std::error_code& ec) {
                                if (!ec) {
                                    std::cout << "Stack saved to file" << std::endl;
                                }
                            });
                        }
                    });
                }
            });
        }
    });

    io_context.run();
    return 0;
}
```

## Best Practices

1. **Error Handling**: Always check the error code in completion handlers to ensure operations were successful.

2. **Asynchronous Operations**: Use the asynchronous methods (`asyncPushd`, `asyncPopd`, etc.) for better performance, especially when dealing with slow file systems or network drives.

3. **Stack Management**: Regularly clear the stack or remove unnecessary directories to prevent excessive memory usage.

4. **File Operations**: When saving or loading the stack from a file, ensure proper file permissions and handle potential I/O errors.

5. **Thread Safety**: The `DirectoryStack` class is not thread-safe by default. If you need to use it from multiple threads, implement proper synchronization mechanisms.

## Troubleshooting

1. **Operation Timeout**: If asynchronous operations take too long, ensure that the file system is responsive and consider implementing timeout mechanisms.

2. **Permission Errors**: When changing directories or saving/loading stack files, make sure the process has the necessary permissions.

3. **Stack Overflow**: If you're pushing too many directories, consider implementing a maximum stack size or regularly clearing unnecessary entries.

4. **File Format Issues**: When loading a stack from a file, ensure the file format matches what the `asyncLoadStackFromFile` method expects.

## Conclusion

The `DirectoryStack` class provides a powerful and flexible way to manage directory navigation in C++ applications. By leveraging asynchronous operations and the Asio library, it offers efficient directory management suitable for both small scripts and large-scale applications.

Remember to always handle errors appropriately and consider the specific requirements of your application when using this class. With proper usage, `DirectoryStack` can significantly simplify directory management tasks in your C++ projects.
