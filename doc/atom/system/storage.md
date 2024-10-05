# StorageMonitor Class Documentation

## Overview

The `StorageMonitor` class, part of the `atom::system` namespace, is designed to monitor storage space changes on all mounted devices. It provides functionality to register callbacks that are triggered when storage space changes occur, list all mounted storage devices, and check for newly inserted media.

## Class Definition

```cpp
namespace atom::system {
class StorageMonitor {
public:
    StorageMonitor() = default;
    ~StorageMonitor();

    void registerCallback(std::function<void(const std::string &)> callback);
    ATOM_NODISCARD auto startMonitoring() -> bool;
    void stopMonitoring();
    ATOM_NODISCARD auto isRunning() const -> bool;
    void triggerCallbacks(const std::string &path);
    ATOM_NODISCARD auto isNewMediaInserted(const std::string &path) -> bool;
    void listAllStorage();
    void listFiles(const std::string &path);

private:
    // Private member variables
};
}
```

## Constructor and Destructor

### `StorageMonitor()`

Default constructor.

### `~StorageMonitor()`

Destructor.

## Member Functions

### `void registerCallback(std::function<void(const std::string &)> callback)`

Registers a callback function to be triggered when storage space changes occur.

- **Parameters:**
  - `callback`: A function that takes a `const std::string&` parameter representing the path of the changed storage.

### `ATOM_NODISCARD auto startMonitoring() -> bool`

Starts the storage space monitoring process.

- **Returns:** `true` if monitoring started successfully, `false` otherwise.

### `void stopMonitoring()`

Stops the storage space monitoring process.

### `ATOM_NODISCARD auto isRunning() const -> bool`

Checks if the monitoring process is currently running.

- **Returns:** `true` if monitoring is active, `false` otherwise.

### `void triggerCallbacks(const std::string &path)`

Triggers all registered callback functions for a specific storage path.

- **Parameters:**
  - `path`: The path of the storage device that has changed.

### `ATOM_NODISCARD auto isNewMediaInserted(const std::string &path) -> bool`

Checks if new storage media has been inserted at the specified path.

- **Parameters:**
  - `path`: The path to check for new media.
- **Returns:** `true` if new media is detected, `false` otherwise.

### `void listAllStorage()`

Lists all mounted storage devices.

### `void listFiles(const std::string &path)`

Lists all files in the specified storage path.

- **Parameters:**
  - `path`: The path of the storage device to list files from.

## Usage Examples

### Example 1: Basic Monitoring Setup

```cpp
#include "storage.hpp"
#include <iostream>

void storageChangeCallback(const std::string& path) {
    std::cout << "Storage change detected at: " << path << std::endl;
}

int main() {
    atom::system::StorageMonitor monitor;

    // Register callback
    monitor.registerCallback(storageChangeCallback);

    // Start monitoring
    if (monitor.startMonitoring()) {
        std::cout << "Storage monitoring started successfully." << std::endl;
    } else {
        std::cerr << "Failed to start storage monitoring." << std::endl;
        return 1;
    }

    // Keep the program running
    while (true) {
        // Your main program logic here
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
```

### Example 2: Checking for New Media

```cpp
#include "storage.hpp"
#include <iostream>

int main() {
    atom::system::StorageMonitor monitor;

    // Check for new media in a specific path
    const std::string path = "/media/usb";
    if (monitor.isNewMediaInserted(path)) {
        std::cout << "New media detected at: " << path << std::endl;
        monitor.listFiles(path);
    } else {
        std::cout << "No new media detected at: " << path << std::endl;
    }

    return 0;
}
```

### Example 3: Listing All Storage and Files

```cpp
#include "storage.hpp"
#include <iostream>

int main() {
    atom::system::StorageMonitor monitor;

    std::cout << "Listing all mounted storage devices:" << std::endl;
    monitor.listAllStorage();

    std::cout << "\nListing files in root directory:" << std::endl;
    monitor.listFiles("/");

    return 0;
}
```

## Notes

- The `StorageMonitor` class uses platform-specific implementations for monitoring storage changes. The `monitorUdisk` function is defined differently for Windows and non-Windows systems.
- The `ATOM_NODISCARD` macro is used to indicate that the return value of certain functions should not be discarded.
- This class is designed to be thread-safe, using a mutex to protect shared data structures.
- The monitoring process runs asynchronously, allowing the main program to continue execution while storage changes are being monitored.

## See Also

- [std::function documentation](https://en.cppreference.com/w/cpp/utility/functional/function)
- [std::mutex documentation](https://en.cppreference.com/w/cpp/thread/mutex)
- Platform-specific documentation for storage monitoring (e.g., Windows API, Linux udev)
