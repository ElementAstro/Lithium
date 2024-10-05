# Stat Class Documentation

## Overview

The `Stat` class, part of the `atom::system` namespace, provides a convenient interface for retrieving file statistics on both Windows and Linux systems. It offers functionality similar to Python's `os.stat()`, allowing users to access various attributes of a file such as its type, size, access times, and permissions.

## Class Definition

```cpp
namespace atom::system {
    class Stat {
    public:
        explicit Stat(const fs::path& path);
        void update();
        [[nodiscard]] auto type() const -> fs::file_type;
        [[nodiscard]] auto size() const -> std::uintmax_t;
        [[nodiscard]] auto atime() const -> std::time_t;
        [[nodiscard]] auto mtime() const -> std::time_t;
        [[nodiscard]] auto ctime() const -> std::time_t;
        [[nodiscard]] auto mode() const -> int;
        [[nodiscard]] auto uid() const -> int;
        [[nodiscard]] auto gid() const -> int;
        [[nodiscard]] auto path() const -> fs::path;

    private:
        fs::path path_;
        std::error_code ec_;
    };
}
```

## Constructor

### `Stat(const fs::path& path)`

Constructs a `Stat` object for the specified file path.

- **Parameters:**
  - `path`: The path to the file whose statistics are to be retrieved.

## Member Functions

### `void update()`

Updates the file statistics. This method refreshes the statistics for the file specified in the constructor.

### `[[nodiscard]] auto type() const -> fs::file_type`

Gets the type of the file.

- **Returns:** The type of the file as an `fs::file_type` enum value.

### `[[nodiscard]] auto size() const -> std::uintmax_t`

Gets the size of the file.

- **Returns:** The size of the file in bytes.

### `[[nodiscard]] auto atime() const -> std::time_t`

Gets the last access time of the file.

- **Returns:** The last access time of the file as a `std::time_t` value.

### `[[nodiscard]] auto mtime() const -> std::time_t`

Gets the last modification time of the file.

- **Returns:** The last modification time of the file as a `std::time_t` value.

### `[[nodiscard]] auto ctime() const -> std::time_t`

Gets the creation time of the file.

- **Returns:** The creation time of the file as a `std::time_t` value.

### `[[nodiscard]] auto mode() const -> int`

Gets the file mode/permissions.

- **Returns:** The file mode/permissions as an integer value.

### `[[nodiscard]] auto uid() const -> int`

Gets the user ID of the file owner.

- **Returns:** The user ID of the file owner as an integer value.

### `[[nodiscard]] auto gid() const -> int`

Gets the group ID of the file owner.

- **Returns:** The group ID of the file owner as an integer value.

### `[[nodiscard]] auto path() const -> fs::path`

Gets the path of the file.

- **Returns:** The path of the file as an `fs::path` object.

## Usage Examples

### Example 1: Basic Usage

```cpp
#include "stat.hpp"
#include <iostream>

int main() {
    atom::system::Stat fileStat("/path/to/your/file.txt");

    std::cout << "File type: " << static_cast<int>(fileStat.type()) << std::endl;
    std::cout << "File size: " << fileStat.size() << " bytes" << std::endl;
    std::cout << "Last access time: " << fileStat.atime() << std::endl;
    std::cout << "Last modification time: " << fileStat.mtime() << std::endl;
    std::cout << "Creation time: " << fileStat.ctime() << std::endl;

    return 0;
}
```

### Example 2: Checking File Permissions

```cpp
#include "stat.hpp"
#include <iostream>

int main() {
    atom::system::Stat fileStat("/path/to/your/file.txt");

    int mode = fileStat.mode();
    std::cout << "File permissions: " << std::oct << mode << std::dec << std::endl;

    // Check if the file is readable by the owner
    if (mode & S_IRUSR) {
        std::cout << "The file is readable by the owner." << std::endl;
    }

    // Check if the file is writable by the owner
    if (mode & S_IWUSR) {
        std::cout << "The file is writable by the owner." << std::endl;
    }

    return 0;
}
```

### Example 3: Updating File Statistics

```cpp
#include "stat.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    atom::system::Stat fileStat("/path/to/your/file.txt");

    std::cout << "Initial size: " << fileStat.size() << " bytes" << std::endl;

    // Simulate file modification (you would actually modify the file here)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Update the file statistics
    fileStat.update();

    std::cout << "Updated size: " << fileStat.size() << " bytes" << std::endl;

    return 0;
}
```

## Notes

- The `Stat` class uses `std::filesystem` for file operations, which requires C++17 or later.
- The class handles errors internally using `std::error_code`. You may want to add error checking methods if needed for your use case.
- The behavior of some methods (like `uid()` and `gid()`) may differ between Windows and Linux systems.

## See Also

- [std::filesystem documentation](https://en.cppreference.com/w/cpp/filesystem)
- [Python os.stat() documentation](https://docs.python.org/3/library/os.html#os.stat) for comparison with the Python equivalent
