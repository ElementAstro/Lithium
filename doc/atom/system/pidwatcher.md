# Atom System PID Watcher Library Documentation

This document provides a detailed explanation of the `PidWatcher` class available in the `atom::system` namespace for monitoring processes by their PID.

## Table of Contents

1. [PidWatcher Class](#pidwatcher-class)
   - [Constructor and Destructor](#constructor-and-destructor)
   - [Public Methods](#public-methods)
2. [Usage Examples](#usage-examples)
3. [Best Practices and Tips](#best-practices-and-tips)
4. [Implementation Details](#implementation-details)

## PidWatcher Class

The `PidWatcher` class provides functionality for monitoring processes by their PID, setting callbacks for process exit, running monitor functions at intervals, and managing the monitoring process.

### Constructor and Destructor

```cpp
PidWatcher();
~PidWatcher();
```

- The constructor initializes a new `PidWatcher` object.
- The destructor ensures proper cleanup of resources when the object is destroyed.

### Public Methods

#### setExitCallback

```cpp
void setExitCallback(Callback callback);
```

Sets the callback function to be executed when the monitored process exits.

- `callback`: A function object (e.g., lambda, function pointer) to be called on process exit.

#### setMonitorFunction

```cpp
void setMonitorFunction(Callback callback, std::chrono::milliseconds interval);
```

Sets a monitor function to be executed at specified intervals while monitoring a process.

- `callback`: A function object to be called periodically.
- `interval`: The time interval between calls to the monitor function.

#### getPidByName

```cpp
[[nodiscard]] auto getPidByName(const std::string &name) const -> pid_t;
```

Retrieves the PID of a process by its name.

- `name`: The name of the process to find.
- Returns: The PID of the process if found, or an appropriate error value if not found.

#### start

```cpp
auto start(const std::string &name) -> bool;
```

Starts monitoring the specified process by name.

- `name`: The name of the process to monitor.
- Returns: `true` if monitoring started successfully, `false` otherwise.

#### stop

```cpp
void stop();
```

Stops monitoring the currently monitored process.

#### Switch

```cpp
auto Switch(const std::string &name) -> bool;
```

Switches the target process to monitor.

- `name`: The name of the new process to monitor.
- Returns: `true` if the process was successfully switched, `false` otherwise.

## Usage Examples

### Basic Usage

```cpp
#include "atom/system/pidwatcher.hpp"
#include <iostream>
#include <chrono>

int main() {
    atom::system::PidWatcher watcher;

    // Set exit callback
    watcher.setExitCallback([]() {
        std::cout << "Monitored process has exited." << std::endl;
    });

    // Set monitor function
    watcher.setMonitorFunction([]() {
        std::cout << "Process is still running..." << std::endl;
    }, std::chrono::seconds(5));

    // Start monitoring a process
    if (watcher.start("example_process")) {
        std::cout << "Started monitoring example_process" << std::endl;

        // Wait for some time
        std::this_thread::sleep_for(std::chrono::minutes(1));

        // Stop monitoring
        watcher.stop();
        std::cout << "Stopped monitoring" << std::endl;
    } else {
        std::cout << "Failed to start monitoring example_process" << std::endl;
    }

    return 0;
}
```

### Switching Monitored Process

```cpp
atom::system::PidWatcher watcher;

// Start monitoring initial process
watcher.start("process1");

// ... Some time later ...

// Switch to monitoring a different process
if (watcher.Switch("process2")) {
    std::cout << "Switched to monitoring process2" << std::endl;
} else {
    std::cout << "Failed to switch to process2" << std::endl;
}
```

### Using getPidByName

```cpp
atom::system::PidWatcher watcher;

pid_t pid = watcher.getPidByName("example_process");
if (pid != -1) {
    std::cout << "PID of example_process: " << pid << std::endl;
} else {
    std::cout << "Failed to find example_process" << std::endl;
}
```

## Best Practices and Tips

1. **Error Handling**: Always check the return values of methods like `start()` and `Switch()` to ensure the operations were successful.

2. **Resource Management**: The `PidWatcher` class manages its own resources, but ensure you stop monitoring before destroying the object or switching processes.

3. **Thread Safety**: The class uses mutexes and condition variables internally, but be cautious when accessing the object from multiple threads.

4. **Callback Design**: Keep exit and monitor callbacks lightweight. For heavy operations, consider offloading work to a separate thread.

5. **Interval Tuning**: Choose an appropriate interval for the monitor function to balance between responsiveness and system load.

6. **Process Name Accuracy**: Ensure you use exact process names when calling methods like `start()` and `Switch()`.

7. **Permissions**: Be aware that monitoring certain processes might require elevated permissions on some systems.

## Implementation Details

- The class uses separate threads for monitoring and handling process exit.
- C++20 features like `std::jthread` are used when available, falling back to standard `std::thread` for earlier C++ versions.
- The implementation likely uses system-specific APIs to retrieve process information and monitor process status.
- Condition variables are used to efficiently wait for process exit or monitoring intervals.

### Note on C++ Version

The implementation adapts based on the C++ standard version:

```cpp
#if __cplusplus >= 202002L
    std::jthread monitor_thread_;
    std::jthread exit_thread_;
#else
    std::thread monitor_thread_;
    std::thread exit_thread_;
#endif
```

This ensures compatibility with both C++20 (using `std::jthread`) and earlier versions (using `std::thread`).
