# Disk Information Module Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Functions](#functions)
3. [Usage Examples](#usage-examples)
4. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The Disk Information Module, part of the `atom::system` namespace, provides a set of functions to retrieve various details about the system's storage devices and their usage. This module is useful for system monitoring, storage management, and disk-related operations.

## Functions

### 1. getDiskUsage

```cpp
[[nodiscard]] auto getDiskUsage() -> std::vector<std::pair<std::string, float>>;
```

Retrieves the disk usage information for all available disks.

- **Return Value**: A vector of pairs, where each pair contains:
  - A string representing the disk name (e.g., "/dev/sda1").
  - A float representing the usage percentage of the disk.

### 2. getDriveModel

```cpp
[[nodiscard]] auto getDriveModel(const std::string& drivePath) -> std::string;
```

Retrieves the model of a specified drive.

- **Parameters**:
  - `drivePath`: A string representing the path of the drive (e.g., "/dev/sda" or "C:\\" on Windows).
- **Return Value**: A string containing the model name of the drive.

### 3. getStorageDeviceModels

```cpp
[[nodiscard]] auto getStorageDeviceModels() -> std::vector<std::pair<std::string, std::string>>;
```

Retrieves the models of all connected storage devices.

- **Return Value**: A vector of pairs, where each pair contains:
  - A string representing the storage device name (e.g., "/dev/sda" or "C:\\" on Windows).
  - A string representing the model name of the storage device.

### 4. getAvailableDrives

```cpp
[[nodiscard]] auto getAvailableDrives() -> std::vector<std::string>;
```

Retrieves a list of all available drives on the system.

- **Return Value**: A vector of strings, where each string represents an available drive.

### 5. calculateDiskUsagePercentage

```cpp
[[nodiscard]] auto calculateDiskUsagePercentage(unsigned long totalSpace, unsigned long freeSpace) -> double;
```

Calculates the disk usage percentage.

- **Parameters**:
  - `totalSpace`: The total space on the disk, in bytes.
  - `freeSpace`: The free (available) space on the disk, in bytes.
- **Return Value**: A double representing the disk usage percentage (between 0.0 and 100.0).

### 6. getFileSystemType

```cpp
[[nodiscard]] auto getFileSystemType(const std::string& path) -> std::string;
```

Retrieves the file system type for a specified path.

- **Parameters**:
  - `path`: A string representing the path to the disk or mount point (e.g., "/dev/sda1" or "C:\\").
- **Return Value**: A string containing the file system type (e.g., "ext4", "NTFS", "APFS").

## Usage Examples

Here's a comprehensive example demonstrating how to use the Disk Information Module:

```cpp
#include "disk.hpp"
#include <iostream>
#include <iomanip>

void printDiskInfo() {
    using namespace atom::system;

    // Get and print disk usage for all available disks
    std::cout << "Disk Usage:\n";
    for (const auto& [disk, usage] : getDiskUsage()) {
        std::cout << "  " << disk << ": " << std::fixed << std::setprecision(2) << usage << "%\n";
    }
    std::cout << "\n";

    // Get and print models of all storage devices
    std::cout << "Storage Device Models:\n";
    for (const auto& [device, model] : getStorageDeviceModels()) {
        std::cout << "  " << device << ": " << model << "\n";
    }
    std::cout << "\n";

    // Get and print available drives
    std::cout << "Available Drives:\n";
    for (const auto& drive : getAvailableDrives()) {
        std::cout << "  " << drive << "\n";
    }
    std::cout << "\n";

    // Calculate and print disk usage percentage for a specific drive
    const std::string exampleDrive = "/dev/sda1";  // Change this to an actual drive on your system
    unsigned long totalSpace = 1000000000;  // Example total space (1 GB)
    unsigned long freeSpace = 250000000;    // Example free space (250 MB)
    double usagePercentage = calculateDiskUsagePercentage(totalSpace, freeSpace);
    std::cout << "Disk Usage Percentage for " << exampleDrive << ": "
              << std::fixed << std::setprecision(2) << usagePercentage << "%\n";

    // Get and print file system type for a specific path
    const std::string examplePath = "/";  // Change this to a relevant path on your system
    std::cout << "File System Type for " << examplePath << ": "
              << getFileSystemType(examplePath) << "\n";
}

int main() {
    printDiskInfo();
    return 0;
}
```

This example demonstrates how to use all the functions provided by the Disk Information Module to print a comprehensive overview of the system's disk information.

## Best Practices and Considerations

1. **Error Handling**: The functions in this module don't have explicit error handling mechanisms. In a production environment, you might want to add error checking and handling for cases where the information might not be available or accessible.

2. **Performance Impact**: Some of these functions (especially `getDiskUsage()` and `getStorageDeviceModels()`) might have a performance impact if called frequently, as they may involve I/O operations. Consider caching the results if you need to access them often.

3. **Privileges**: Depending on the operating system, some of these functions might require elevated privileges to access certain system information. Ensure your application has the necessary permissions.

4. **Cross-Platform Considerations**: The implementation of these functions might vary across different operating systems. Ensure you test on all target platforms and handle platform-specific variations (e.g., drive naming conventions).

5. **Large Disk Sizes**: When working with `calculateDiskUsagePercentage()`, be aware of potential overflows with very large disk sizes. Consider using a larger integer type if necessary.

6. **File System Changes**: File system information can change dynamically. If your application relies on this information, consider how often you need to refresh it.

7. **Thread Safety**: The documentation doesn't specify thread safety for these functions. If you're using them in a multi-threaded environment, consider adding synchronization mechanisms.

8. **Path Handling**: When working with file paths, be mindful of different path formats across operating systems. Use appropriate path manipulation libraries or functions to ensure compatibility.

9. **Removable Drives**: Be aware that the list of available drives can change if removable drives are connected or disconnected. Your application should be able to handle such dynamic changes.

10. **Performance Metrics**: The disk usage percentage alone might not give a complete picture of disk performance. Consider incorporating other metrics like I/O operations per second (IOPS) or read/write speeds for a more comprehensive analysis.
