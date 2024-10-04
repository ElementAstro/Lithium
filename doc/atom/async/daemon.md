# Documentation for daemon.hpp

This document provides a detailed overview of the `daemon.hpp` header file, which contains classes and functions for managing daemon processes in C++.

## Table of Contents

1. [DaemonGuard Class](#daemonguard-class)
2. [Utility Functions](#utility-functions)
3. [Platform-Specific Considerations](#platform-specific-considerations)

## DaemonGuard Class

The `DaemonGuard` class is responsible for managing process information and controlling the execution of daemon processes.

### Methods

#### toString

```cpp
[[nodiscard]] auto toString() const -> std::string;
```

Converts process information to a string.

- **Return Value**: The process information as a string.

#### realStart

```cpp
auto realStart(int argc, char **argv, const std::function<int(int argc, char **argv)> &mainCb) -> int;
```

Starts a child process to execute the actual task.

- **Parameters**:
  - `argc`: The number of command line arguments.
  - `argv`: An array of command line arguments.
  - `mainCb`: The main callback function to be executed in the child process.
- **Return Value**: The return value of the main callback function.

#### realDaemon

```cpp
auto realDaemon(int argc, char **argv, const std::function<int(int argc, char **argv)> &mainCb) -> int;
```

Starts a child process to execute the actual task as a daemon.

- **Parameters**:
  - `argc`: The number of command line arguments.
  - `argv`: An array of command line arguments.
  - `mainCb`: The main callback function to be executed in the child process.
- **Return Value**: The return value of the main callback function.

#### startDaemon

```cpp
auto startDaemon(int argc, char **argv, const std::function<int(int argc, char **argv)> &mainCb, bool isDaemon) -> int;
```

Starts the process. If a daemon process needs to be created, it will create the daemon process first.

- **Parameters**:
  - `argc`: The number of command line arguments.
  - `argv`: An array of command line arguments.
  - `mainCb`: The main callback function to be executed.
  - `isDaemon`: Determines if a daemon process should be created.
- **Return Value**: The return value of the main callback function.

### Private Members

- `m_parentId`: The parent process ID.
- `m_mainId`: The child process ID.
- `m_parentStartTime`: The start time of the parent process.
- `m_mainStartTime`: The start time of the child process.
- `m_restartCount`: The number of restarts.

## Utility Functions

### signalHandler

```cpp
void signalHandler(int signum);
```

Signal handler function.

- **Parameters**:
  - `signum`: The signal number.

### writePidFile

```cpp
void writePidFile();
```

Writes the process ID to a file.

### checkPidFile

```cpp
auto checkPidFile() -> bool;
```

Checks if the process ID file exists.

- **Return Value**: True if the process ID file exists, false otherwise.

## Platform-Specific Considerations

The `daemon.hpp` file includes platform-specific headers and implementations:

- For Windows (`_WIN32`):
  - Includes `<windows.h>`
  - Uses `HANDLE` for process IDs
- For Unix-like systems:
  - Includes `<sys/stat.h>`, `<sys/wait.h>`, and `<unistd.h>`
  - Uses `pid_t` for process IDs

## Usage Example

Here's a basic example of how to use the `DaemonGuard` class to create a daemon process:

```cpp
#include "daemon.hpp"
#include <iostream>

int main(int argc, char** argv) {
    atom::async::DaemonGuard daemon;

    auto mainFunction = [](int argc, char** argv) -> int {
        // Your daemon process logic here
        std::cout << "Daemon process is running..." << std::endl;
        while (true) {
            // Perform daemon tasks
        }
        return 0;
    };

    bool runAsDaemon = true; // Set to false if you don't want to run as a daemon
    return daemon.startDaemon(argc, argv, mainFunction, runAsDaemon);
}
```

In this example:

1. We create a `DaemonGuard` object.
2. We define a lambda function `mainFunction` that contains the logic for our daemon process.
3. We call `startDaemon` with our command-line arguments, the main function, and a boolean indicating whether to run as a daemon.

Remember to handle signals appropriately and manage the PID file as needed in your actual implementation.

## Notes

- The `DaemonGuard` class provides a way to manage both regular processes and daemon processes.
- The class handles the forking and execution of child processes, as well as tracking process information.
- Signal handling and PID file management are important aspects of daemon processes that are supported by the provided utility functions.
- Platform-specific code ensures compatibility with both Windows and Unix-like systems.

When using this class and its associated functions, make sure to consider proper error handling, logging, and resource management in your daemon processes.
