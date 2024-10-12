# Priority Manager Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Enumerations](#enumerations)
4. [Public Methods](#public-methods)
5. [Usage Examples](#usage-examples)

## Introduction

The `PriorityManager` class is a utility for managing process and thread priorities and affinities across different platforms (Windows and POSIX-compliant systems). It provides a unified interface for adjusting and monitoring priorities, scheduling policies, and CPU affinities.

## Class Overview

```cpp
class PriorityManager {
public:
    enum class PriorityLevel { /* ... */ };
    enum class SchedulingPolicy { /* ... */ };

    static void setProcessPriority(PriorityLevel level, int pid = 0);
    static auto getProcessPriority(int pid = 0) -> PriorityLevel;
    static void setThreadPriority(PriorityLevel level, std::thread::native_handle_type thread = 0);
    static auto getThreadPriority(std::thread::native_handle_type thread = 0) -> PriorityLevel;
    static void setThreadSchedulingPolicy(SchedulingPolicy policy, std::thread::native_handle_type thread = 0);
    static void setProcessAffinity(const std::vector<int>& cpus, int pid = 0);
    static auto getProcessAffinity(int pid = 0) -> std::vector<int>;
    static void startPriorityMonitor(int pid, const std::function<void(PriorityLevel)>& callback,
                                     std::chrono::milliseconds interval = std::chrono::seconds(1));

private:
    // Private helper methods
};
```

## Enumerations

### PriorityLevel

Defines various priority levels for processes and threads:

```cpp
enum class PriorityLevel {
    LOWEST,
    BELOW_NORMAL,
    NORMAL,
    ABOVE_NORMAL,
    HIGHEST,
    REALTIME
};
```

### SchedulingPolicy

Defines various scheduling policies for threads:

```cpp
enum class SchedulingPolicy {
    NORMAL,
    FIFO,
    ROUND_ROBIN
};
```

## Public Methods

### setProcessPriority

```cpp
static void setProcessPriority(PriorityLevel level, int pid = 0);
```

Sets the priority of a process. If `pid` is 0, it sets the priority for the current process.

### getProcessPriority

```cpp
static auto getProcessPriority(int pid = 0) -> PriorityLevel;
```

Gets the priority of a process. If `pid` is 0, it gets the priority of the current process.

### setThreadPriority

```cpp
static void setThreadPriority(PriorityLevel level, std::thread::native_handle_type thread = 0);
```

Sets the priority of a thread. If `thread` is 0, it sets the priority for the current thread.

### getThreadPriority

```cpp
static auto getThreadPriority(std::thread::native_handle_type thread = 0) -> PriorityLevel;
```

Gets the priority of a thread. If `thread` is 0, it gets the priority of the current thread.

### setThreadSchedulingPolicy

```cpp
static void setThreadSchedulingPolicy(SchedulingPolicy policy, std::thread::native_handle_type thread = 0);
```

Sets the scheduling policy of a thread. If `thread` is 0, it sets the policy for the current thread.

### setProcessAffinity

```cpp
static void setProcessAffinity(const std::vector<int>& cpus, int pid = 0);
```

Sets the CPU affinity of a process. If `pid` is 0, it sets the affinity for the current process.

### getProcessAffinity

```cpp
static auto getProcessAffinity(int pid = 0) -> std::vector<int>;
```

Gets the CPU affinity of a process. If `pid` is 0, it gets the affinity of the current process.

### startPriorityMonitor

```cpp
static void startPriorityMonitor(int pid, const std::function<void(PriorityLevel)>& callback,
                                 std::chrono::milliseconds interval = std::chrono::seconds(1));
```

Starts monitoring the priority of a process and calls the provided callback function when the priority changes.

## Usage Examples

### Setting and Getting Process Priority

```cpp
#include "PriorityManager.h"
#include <iostream>

int main() {
    // Set the priority of the current process to ABOVE_NORMAL
    PriorityManager::setProcessPriority(PriorityManager::PriorityLevel::ABOVE_NORMAL);

    // Get and print the current process priority
    PriorityManager::PriorityLevel currentPriority = PriorityManager::getProcessPriority();
    std::cout << "Current process priority: " << static_cast<int>(currentPriority) << std::endl;

    return 0;
}
```

### Setting Thread Priority and Scheduling Policy

```cpp
#include "PriorityManager.h"
#include <thread>
#include <iostream>

void worker() {
    // Set the priority of the current thread to HIGHEST
    PriorityManager::setThreadPriority(PriorityManager::PriorityLevel::HIGHEST);

    // Set the scheduling policy of the current thread to FIFO
    PriorityManager::setThreadSchedulingPolicy(PriorityManager::SchedulingPolicy::FIFO);

    // Perform high-priority work here
    std::cout << "Performing high-priority work..." << std::endl;
}

int main() {
    std::thread t(worker);
    t.join();
    return 0;
}
```

### Setting and Getting Process Affinity

```cpp
#include "PriorityManager.h"
#include <iostream>

int main() {
    // Set the process affinity to CPUs 0 and 2
    std::vector<int> cpus = {0, 2};
    PriorityManager::setProcessAffinity(cpus);

    // Get and print the current process affinity
    std::vector<int> currentAffinity = PriorityManager::getProcessAffinity();
    std::cout << "Current process affinity: ";
    for (int cpu : currentAffinity) {
        std::cout << cpu << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

### Monitoring Process Priority

```cpp
#include "PriorityManager.h"
#include <iostream>
#include <chrono>
#include <thread>

void priorityChangedCallback(PriorityManager::PriorityLevel newPriority) {
    std::cout << "Process priority changed to: " << static_cast<int>(newPriority) << std::endl;
}

int main() {
    int pid = 1234; // Replace with the actual process ID you want to monitor
    PriorityManager::startPriorityMonitor(pid, priorityChangedCallback);

    // Keep the program running for a while to monitor priority changes
    std::this_thread::sleep_for(std::chrono::minutes(5));

    return 0;
}
```
