# AsyncFile and AsyncDirectory Classes Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [AsyncFile Class](#asyncfile-class)
   - [Constructor](#asyncfile-constructor)
   - [Methods](#asyncfile-methods)
   - [Usage Examples](#asyncfile-usage-examples)
3. [AsyncDirectory Class](#asyncdirectory-class)
   - [Constructor](#asyncdirectory-constructor)
   - [Methods](#asyncdirectory-methods)
   - [Usage Examples](#asyncdirectory-usage-examples)

## Introduction

The `AsyncFile` and `AsyncDirectory` classes, part of the `atom::async::io` namespace, provide asynchronous file and directory operations using the ASIO library. These classes allow for efficient I/O operations without blocking the main thread, which is particularly useful for applications that need to handle multiple I/O operations concurrently.

## AsyncFile Class

The `AsyncFile` class offers various asynchronous file operations such as reading, writing, deleting, and copying files.

### AsyncFile Constructor

```cpp
explicit AsyncFile(asio::io_context& io_context);
```

Creates an `AsyncFile` object using the provided ASIO I/O context.

- `io_context`: Reference to an `asio::io_context` object for managing asynchronous operations.

### AsyncFile Methods

1. `asyncRead`

   ```cpp
   void asyncRead(const std::string& filename,
                  const std::function<void(const std::string&)>& callback);
   ```

   Asynchronously reads the contents of a file.

2. `asyncWrite`

   ```cpp
   void asyncWrite(const std::string& filename, const std::string& content,
                   const std::function<void(bool)>& callback);
   ```

   Asynchronously writes content to a file.

3. `asyncDelete`

   ```cpp
   void asyncDelete(const std::string& filename,
                    const std::function<void(bool)>& callback);
   ```

   Asynchronously deletes a file.

4. `asyncCopy`

   ```cpp
   void asyncCopy(const std::string& src, const std::string& dest,
                  const std::function<void(bool)>& callback);
   ```

   Asynchronously copies a file.

5. `asyncReadWithTimeout`

   ```cpp
   void asyncReadWithTimeout(
       const std::string& filename, int timeoutMs,
       const std::function<void(const std::string&)>& callback);
   ```

   Asynchronously reads a file with a timeout.

6. `asyncBatchRead`

   ```cpp
   void asyncBatchRead(
       const std::vector<std::string>& files,
       const std::function<void(const std::vector<std::string>&)>& callback);
   ```

   Asynchronously reads multiple files.

7. `asyncStat`

   ```cpp
   void asyncStat(
       const std::string& filename,
       const std::function<void(bool, std::uintmax_t, std::time_t)>& callback);
   ```

   Asynchronously retrieves file statistics.

8. `asyncMove`

   ```cpp
   void asyncMove(const std::string& src, const std::string& dest,
                  const std::function<void(bool)>& callback);
   ```

   Asynchronously moves a file.

9. `asyncChangePermissions`

   ```cpp
   void asyncChangePermissions(const std::string& filename,
                               std::filesystem::perms perms,
                               const std::function<void(bool)>& callback);
   ```

   Asynchronously changes file permissions.

10. `asyncCreateDirectory`

    ```cpp
    void asyncCreateDirectory(const std::string& path,
                              const std::function<void(bool)>& callback);
    ```

    Asynchronously creates a directory.

11. `asyncExists`
    ```cpp
    void asyncExists(const std::string& filename,
                     const std::function<void(bool)>& callback);
    ```
    Asynchronously checks if a file exists.

### AsyncFile Usage Examples

```cpp
#include "async_io.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;
    atom::async::io::AsyncFile file(io_context);

    // Example 1: Reading a file
    file.asyncRead("example.txt", [](const std::string& content) {
        std::cout << "File content: " << content << std::endl;
    });

    // Example 2: Writing to a file
    file.asyncWrite("output.txt", "Hello, World!", [](bool success) {
        if (success) {
            std::cout << "File written successfully." << std::endl;
        } else {
            std::cout << "Failed to write file." << std::endl;
        }
    });

    // Example 3: Copying a file
    file.asyncCopy("source.txt", "destination.txt", [](bool success) {
        if (success) {
            std::cout << "File copied successfully." << std::endl;
        } else {
            std::cout << "Failed to copy file." << std::endl;
        }
    });

    io_context.run();
    return 0;
}
```

## AsyncDirectory Class

The `AsyncDirectory` class provides asynchronous operations for directory management.

### AsyncDirectory Constructor

```cpp
explicit AsyncDirectory(asio::io_context& io_context);
```

Creates an `AsyncDirectory` object using the provided ASIO I/O context.

- `io_context`: Reference to an `asio::io_context` object for managing asynchronous operations.

### AsyncDirectory Methods

1. `asyncCreate`

   ```cpp
   void asyncCreate(const std::string& path,
                    const std::function<void(bool)>& callback);
   ```

   Asynchronously creates a directory.

2. `asyncRemove`

   ```cpp
   void asyncRemove(const std::string& path,
                    const std::function<void(bool)>& callback);
   ```

   Asynchronously removes a directory.

3. `asyncListContents`

   ```cpp
   void asyncListContents(
       const std::string& path,
       const std::function<void(std::vector<std::string>)>& callback);
   ```

   Asynchronously lists the contents of a directory.

4. `asyncExists`
   ```cpp
   void asyncExists(const std::string& path,
                    const std::function<void(bool)>& callback);
   ```
   Asynchronously checks if a directory exists.

### AsyncDirectory Usage Examples

```cpp
#include "async_io.hpp"
#include <asio.hpp>
#include <iostream>

int main() {
    asio::io_context io_context;
    atom::async::io::AsyncDirectory dir(io_context);

    // Example 1: Creating a directory
    dir.asyncCreate("new_directory", [](bool success) {
        if (success) {
            std::cout << "Directory created successfully." << std::endl;
        } else {
            std::cout << "Failed to create directory." << std::endl;
        }
    });

    // Example 2: Listing directory contents
    dir.asyncListContents("existing_directory", [](const std::vector<std::string>& contents) {
        std::cout << "Directory contents:" << std::endl;
        for (const auto& item : contents) {
            std::cout << "- " << item << std::endl;
        }
    });

    io_context.run();
    return 0;
}
```
