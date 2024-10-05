# Memory Information Module Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Structures](#structures)
3. [Functions](#functions)
4. [Usage Examples](#usage-examples)
5. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The Memory Information Module, part of the `atom::system` namespace, provides a set of functions and structures to retrieve various details about the system's memory, including physical memory, virtual memory, and swap memory. This module is useful for system monitoring, performance analysis, and memory management tasks.

## Structures

### MemoryInfo

```cpp
struct MemoryInfo {
    struct MemorySlot {
        std::string capacity;
        std::string clockSpeed;
        std::string type;

        MemorySlot() = default;
        MemorySlot(std::string capacity, std::string clockSpeed, std::string type);
    } ATOM_ALIGNAS(128);

    std::vector<MemorySlot> slots;
    unsigned long long virtualMemoryMax;
    unsigned long long virtualMemoryUsed;
    unsigned long long swapMemoryTotal;
    unsigned long long swapMemoryUsed;
} ATOM_ALIGNAS(64);
```

This structure holds comprehensive information about the system's memory, including details about physical memory slots, virtual memory, and swap memory.

## Functions

### 1. getMemoryUsage

```cpp
auto getMemoryUsage() -> float;
```

Retrieves the current memory usage percentage.

- **Return Value**: A float representing the memory usage percentage.

### 2. getTotalMemorySize

```cpp
auto getTotalMemorySize() -> unsigned long long;
```

Retrieves the total physical memory size.

- **Return Value**: An unsigned long long representing the total memory size in bytes.

### 3. getAvailableMemorySize

```cpp
auto getAvailableMemorySize() -> unsigned long long;
```

Retrieves the available physical memory size.

- **Return Value**: An unsigned long long representing the available memory size in bytes.

### 4. getPhysicalMemoryInfo

```cpp
auto getPhysicalMemoryInfo() -> MemoryInfo::MemorySlot;
```

Retrieves information about the physical memory slot.

- **Return Value**: A `MemoryInfo::MemorySlot` structure containing information about the memory slot.

### 5. getVirtualMemoryMax

```cpp
auto getVirtualMemoryMax() -> unsigned long long;
```

Retrieves the maximum virtual memory size.

- **Return Value**: An unsigned long long representing the maximum virtual memory size in bytes.

### 6. getVirtualMemoryUsed

```cpp
auto getVirtualMemoryUsed() -> unsigned long long;
```

Retrieves the amount of virtual memory currently in use.

- **Return Value**: An unsigned long long representing the used virtual memory size in bytes.

### 7. getSwapMemoryTotal

```cpp
auto getSwapMemoryTotal() -> unsigned long long;
```

Retrieves the total swap memory size.

- **Return Value**: An unsigned long long representing the total swap memory size in bytes.

### 8. getSwapMemoryUsed

```cpp
auto getSwapMemoryUsed() -> unsigned long long;
```

Retrieves the amount of swap memory currently in use.

- **Return Value**: An unsigned long long representing the used swap memory size in bytes.

### 9. getCommittedMemory

```cpp
auto getCommittedMemory() -> size_t;
```

Retrieves the amount of committed memory.

- **Return Value**: A size_t representing the committed memory size in bytes.

### 10. getUncommittedMemory

```cpp
auto getUncommittedMemory() -> size_t;
```

Retrieves the amount of uncommitted memory.

- **Return Value**: A size_t representing the uncommitted memory size in bytes.

## Usage Examples

Here's a comprehensive example demonstrating how to use the Memory Information Module:

```cpp
#include "memory.hpp"
#include <iostream>
#include <iomanip>

void printMemoryInfo() {
    using namespace atom::system;

    // Get and print memory usage percentage
    std::cout << "Memory Usage: " << std::fixed << std::setprecision(2)
              << getMemoryUsage() << "%\n";

    // Get and print total and available memory
    auto totalMemory = getTotalMemorySize();
    auto availableMemory = getAvailableMemorySize();
    std::cout << "Total Memory: " << totalMemory / (1024 * 1024) << " MB\n";
    std::cout << "Available Memory: " << availableMemory / (1024 * 1024) << " MB\n";

    // Get and print physical memory slot info
    auto memorySlot = getPhysicalMemoryInfo();
    std::cout << "Physical Memory Slot:\n";
    std::cout << "  Capacity: " << memorySlot.capacity << "\n";
    std::cout << "  Clock Speed: " << memorySlot.clockSpeed << "\n";
    std::cout << "  Type: " << memorySlot.type << "\n";

    // Get and print virtual memory information
    auto virtualMemMax = getVirtualMemoryMax();
    auto virtualMemUsed = getVirtualMemoryUsed();
    std::cout << "Virtual Memory:\n";
    std::cout << "  Max: " << virtualMemMax / (1024 * 1024) << " MB\n";
    std::cout << "  Used: " << virtualMemUsed / (1024 * 1024) << " MB\n";

    // Get and print swap memory information
    auto swapMemTotal = getSwapMemoryTotal();
    auto swapMemUsed = getSwapMemoryUsed();
    std::cout << "Swap Memory:\n";
    std::cout << "  Total: " << swapMemTotal / (1024 * 1024) << " MB\n";
    std::cout << "  Used: " << swapMemUsed / (1024 * 1024) << " MB\n";

    // Get and print committed and uncommitted memory
    auto committedMem = getCommittedMemory();
    auto uncommittedMem = getUncommittedMemory();
    std::cout << "Committed Memory: " << committedMem / (1024 * 1024) << " MB\n";
    std::cout << "Uncommitted Memory: " << uncommittedMem / (1024 * 1024) << " MB\n";
}

int main() {
    printMemoryInfo();
    return 0;
}
```

This example demonstrates how to use all the functions provided by the Memory Information Module to print a comprehensive overview of the system's memory information.

## Best Practices and Considerations

1. **Error Handling**: The functions in this module don't have explicit error handling mechanisms. In a production environment, you might want to add error checking and handling for cases where the information might not be available or accessible.

2. **Performance Impact**: Some of these functions might have a performance impact if called frequently. Consider caching the results if you need to access them often, especially for values that don't change rapidly.

3. **Privileges**: Depending on the operating system, some of these functions might require elevated privileges to access certain system information. Ensure your application has the necessary permissions.

4. **Cross-Platform Considerations**: The implementation of these functions might vary across different operating systems. Ensure you test on all target platforms and handle platform-specific variations.

5. **Unit Conversions**: The memory sizes are typically returned in bytes. Consider converting to more human-readable units (MB, GB) when displaying to users, as shown in the example.

6. **Memory Pressure**: Be aware that high memory usage or low available memory can impact system performance. Consider implementing warnings or actions based on these values in your application.

7. **Thread Safety**: The documentation doesn't specify thread safety for these functions. If you're using them in a multi-threaded environment, consider adding synchronization mechanisms.

8. **Virtual Memory vs Physical Memory**: Understand the difference between virtual and physical memory when interpreting these values. High virtual memory usage doesn't necessarily indicate a problem if physical memory usage is reasonable.

9. **Swap Memory Usage**: High swap memory usage can indicate that the system is under memory pressure. Monitor this value to detect potential
