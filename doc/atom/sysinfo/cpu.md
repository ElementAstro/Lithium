# CPU Information Module Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Structures](#structures)
3. [Functions](#functions)
4. [Usage Examples](#usage-examples)
5. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The CPU Information Module, part of the `atom::system` namespace, provides a set of functions to retrieve various details about the system's CPU(s). This module is useful for system monitoring, performance tuning, and application optimization.

## Structures

### CacheSizes

```cpp
struct CacheSizes {
    size_t l1d;  // L1 data cache size in bytes
    size_t l1i;  // L1 instruction cache size in bytes
    size_t l2;   // L2 cache size in bytes
    size_t l3;   // L3 cache size in bytes
} ATOM_ALIGNAS(32);
```

This structure holds the sizes of different CPU caches. It's aligned to a 32-byte boundary for optimal memory access.

## Functions

### 1. getCurrentCpuUsage

```cpp
[[nodiscard]] auto getCurrentCpuUsage() -> float;
```

Retrieves the current CPU usage percentage.

- **Return Value**: A float representing the CPU usage (0.0 to 100.0).

### 2. getCurrentCpuTemperature

```cpp
[[nodiscard]] auto getCurrentCpuTemperature() -> float;
```

Retrieves the current CPU temperature.

- **Return Value**: A float representing the CPU temperature in degrees Celsius.

### 3. getCPUModel

```cpp
[[nodiscard]] auto getCPUModel() -> std::string;
```

Retrieves the CPU model name.

- **Return Value**: A string containing the CPU model name.

### 4. getProcessorIdentifier

```cpp
[[nodiscard]] auto getProcessorIdentifier() -> std::string;
```

Retrieves the CPU identifier.

- **Return Value**: A string representing the unique CPU identifier.

### 5. getProcessorFrequency

```cpp
[[nodiscard]] auto getProcessorFrequency() -> double;
```

Retrieves the current CPU frequency.

- **Return Value**: A double representing the CPU frequency in GHz.

### 6. getNumberOfPhysicalPackages

```cpp
[[nodiscard]] auto getNumberOfPhysicalPackages() -> int;
```

Retrieves the number of physical CPU packages (sockets) in the system.

- **Return Value**: An integer representing the number of physical CPU packages.

### 7. getNumberOfPhysicalCPUs

```cpp
[[nodiscard]] auto getNumberOfPhysicalCPUs() -> int;
```

Retrieves the number of logical CPUs (cores) in the system.

- **Return Value**: An integer representing the total number of logical CPUs.

### 8. getCacheSizes

```cpp
[[nodiscard]] auto getCacheSizes() -> CacheSizes;
```

Retrieves the sizes of the CPU caches.

- **Return Value**: A `CacheSizes` structure containing the sizes of L1, L2, and L3 caches in bytes.

## Usage Examples

Here's a comprehensive example demonstrating how to use the CPU Information Module:

```cpp
#include "cpu.hpp"
#include <iostream>
#include <iomanip>

void printCPUInfo() {
    using namespace atom::system;

    // Get and print CPU model
    std::cout << "CPU Model: " << getCPUModel() << std::endl;

    // Get and print CPU identifier
    std::cout << "CPU Identifier: " << getProcessorIdentifier() << std::endl;

    // Get and print current CPU usage
    std::cout << "Current CPU Usage: " << std::fixed << std::setprecision(2)
              << getCurrentCpuUsage() << "%" << std::endl;

    // Get and print current CPU temperature
    std::cout << "Current CPU Temperature: " << std::fixed << std::setprecision(2)
              << getCurrentCpuTemperature() << "°C" << std::endl;

    // Get and print CPU frequency
    std::cout << "CPU Frequency: " << std::fixed << std::setprecision(2)
              << getProcessorFrequency() << " GHz" << std::endl;

    // Get and print number of physical packages and CPUs
    std::cout << "Number of Physical Packages: " << getNumberOfPhysicalPackages() << std::endl;
    std::cout << "Number of Logical CPUs: " << getNumberOfPhysicalCPUs() << std::endl;

    // Get and print cache sizes
    CacheSizes cacheSizes = getCacheSizes();
    std::cout << "Cache Sizes:" << std::endl;
    std::cout << "  L1 Data Cache: " << cacheSizes.l1d / 1024 << " KB" << std::endl;
    std::cout << "  L1 Instruction Cache: " << cacheSizes.l1i / 1024 << " KB" << std::endl;
    std::cout << "  L2 Cache: " << cacheSizes.l2 / 1024 << " KB" << std::endl;
    std::cout << "  L3 Cache: " << cacheSizes.l3 / (1024 * 1024) << " MB" << std::endl;
}

int main() {
    printCPUInfo();
    return 0;
}
```

This example demonstrates how to use all the functions provided by the CPU Information Module to print a comprehensive overview of the system's CPU information.

## Best Practices and Considerations

1. **Error Handling**: The functions in this module don't have explicit error handling mechanisms. In a production environment, you might want to add error checking and handling for cases where the information might not be available.

2. **Performance Impact**: Some of these functions (especially `getCurrentCpuUsage()` and `getCurrentCpuTemperature()`) might have a small performance impact if called frequently. Consider caching the results if you need to access them often.

3. **Privileges**: Depending on the operating system, some of these functions might require elevated privileges to access certain system information. Ensure your application has the necessary permissions.

4. **Cross-Platform Considerations**: The implementation of these functions might vary across different operating systems. Ensure you test on all target platforms.

5. **Frequency of Updates**: CPU usage and temperature can change rapidly. If you're monitoring these values, consider how often you need to update them based on your application's requirements.

6. **Thread Safety**: The documentation doesn't specify thread safety for these functions. If you're using them in a multi-threaded environment, consider adding synchronization mechanisms.

7. **Cache Size Interpretation**: When interpreting cache sizes, remember that they are returned in bytes. You might want to convert them to more readable units (KB or MB) as shown in the example.

8. **CPU Frequency Variations**: Modern CPUs often have variable frequencies (e.g., due to power-saving features or turbo boost). The `getProcessorFrequency()` function might return an instantaneous or average value, which could vary over time.
