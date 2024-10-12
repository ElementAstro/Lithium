# Atom System Process Management Library Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Structures](#structures)
3. [ProcessManager Class](#processmanager-class)
4. [Global Functions](#global-functions)
5. [Usage Examples](#usage-examples)
6. [Best Practices](#best-practices)
7. [Platform-Specific Considerations](#platform-specific-considerations)

## Introduction

The Atom System Process Management Library provides a comprehensive set of tools for managing and monitoring system processes. It includes the `ProcessManager` class for handling multiple processes and several standalone functions for process-related operations.

## Structures

### Process

```cpp
struct Process {
    int pid;
    std::string name;
    std::string output;
    fs::path path;
    std::string status;
#if _WIN32
    void *handle;
#endif
};
```

Represents a system process with its ID, name, output, path, status, and handle (on Windows).

### NetworkConnection

```cpp
struct NetworkConnection {
    std::string protocol;
    std::string line;
} ATOM_ALIGNAS(64);
```

Represents a network connection with its protocol and connection details.

## ProcessManager Class

### Constructor

```cpp
explicit ProcessManager(int maxProcess = 10);
```

Creates a `ProcessManager` instance with a specified maximum number of processes to manage.

### Static Methods

```cpp
static auto createShared(int maxProcess = 10) -> std::shared_ptr<ProcessManager>;
```

Creates a shared pointer to a `ProcessManager` instance.

### Member Methods

```cpp
auto createProcess(const std::string &command, const std::string &identifier) -> bool;
auto terminateProcess(int pid, int signal = 15) -> bool;
auto terminateProcessByName(const std::string &name, int signal = 15) -> bool;
auto hasProcess(const std::string &identifier) -> bool;
[[nodiscard]] auto getRunningProcesses() const -> std::vector<Process>;
[[nodiscard]] auto getProcessOutput(const std::string &identifier) -> std::vector<std::string>;
void waitForCompletion();
auto runScript(const std::string &script, const std::string &identifier) -> bool;
auto monitorProcesses() -> bool;
```

These methods allow for creating, terminating, checking, and monitoring processes, as well as running scripts and retrieving process outputs.

## Global Functions

```cpp
auto getAllProcesses() -> std::vector<std::pair<int, std::string>>;
[[nodiscard]] auto getSelfProcessInfo() -> Process;
[[nodiscard]] auto ctermid() -> std::string;
auto getProcessPriorityByPid(int pid) -> std::optional<int>;
auto getProcessPriorityByName(const std::string &name) -> std::optional<int>;
auto isProcessRunning(const std::string &processName) -> bool;
auto getParentProcessId(int processId) -> int;
auto _CreateProcessAsUser(const std::string &command, const std::string &username,
                          const std::string &domain, const std::string &password) -> bool;
auto getNetworkConnections(int pid) -> std::vector<NetworkConnection>;
auto getProcessIdByName(const std::string &processName) -> std::vector<int>;
```

These functions provide various utilities for process management and information retrieval.

## Usage Examples

### Creating and Managing Processes

```cpp
#include "atom/system/process.hpp"
#include <iostream>

int main() {
    auto processManager = atom::system::ProcessManager::createShared(5);

    // Create a new process
    if (processManager->createProcess("ls -l", "list_files")) {
        std::cout << "Process created successfully." << std::endl;
    }

    // Check if the process exists
    if (processManager->hasProcess("list_files")) {
        std::cout << "Process 'list_files' is running." << std::endl;
    }

    // Get process output
    auto output = processManager->getProcessOutput("list_files");
    for (const auto& line : output) {
        std::cout << line << std::endl;
    }

    // Wait for all processes to complete
    processManager->waitForCompletion();

    return 0;
}
```

### Monitoring Processes

```cpp
#include "atom/system/process.hpp"
#include <iostream>

int main() {
    auto processManager = atom::system::ProcessManager::createShared();

    processManager->createProcess("long_running_task", "background_task");

    if (processManager->monitorProcesses()) {
        std::cout << "Monitoring processes..." << std::endl;

        auto runningProcesses = processManager->getRunningProcesses();
        for (const auto& process : runningProcesses) {
            std::cout << "PID: " << process.pid << ", Name: " << process.name << std::endl;
        }
    }

    return 0;
}
```

### Using Global Functions

```cpp
#include "atom/system/process.hpp"
#include <iostream>

int main() {
    // Get all processes
    auto allProcesses = atom::system::getAllProcesses();
    for (const auto& [pid, name] : allProcesses) {
        std::cout << "PID: " << pid << ", Name: " << name << std::endl;
    }

    // Check if a specific process is running
    if (atom::system::isProcessRunning("firefox")) {
        std::cout << "Firefox is running." << std::endl;
    }

    // Get process priority
    auto priority = atom::system::getProcessPriorityByName("chrome");
    if (priority) {
        std::cout << "Chrome priority: " << *priority << std::endl;
    }

    // Get network connections for a process
    auto connections = atom::system::getNetworkConnections(1234);  // Replace with actual PID
    for (const auto& conn : connections) {
        std::cout << "Protocol: " << conn.protocol << ", Details: " << conn.line << std::endl;
    }

    return 0;
}
```

## Best Practices

1. **Error Handling**: Always check the return values of functions that return boolean or optional values to ensure operations were successful.

2. **Resource Management**: Use the `waitForCompletion()` method to ensure all managed processes have completed before exiting your program.

3. **Security**: Be cautious when using functions like `_CreateProcessAsUser()` that involve user credentials. Ensure proper security measures are in place.

4. **Performance**: When dealing with a large number of processes, consider using the `monitorProcesses()` method instead of repeatedly calling individual functions.

5. **Cross-Platform Compatibility**: Be aware of platform-specific functions and use appropriate conditional compilation when necessary.

## Platform-Specific Considerations

- The `Process` struct includes a `handle` member only on Windows systems.
- The `_CreateProcessAsUser()` function is only available on Windows.
- On non-Windows systems, use the `getProcFilePath()` method to get process file paths.
