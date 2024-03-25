# DaemonGuard Class Documentation

The `DaemonGuard` class is designed for managing process information and starting child processes to execute tasks. It provides methods for starting processes, handling daemon processes, and converting process information to a string.

## Class Declaration

```cpp
class DaemonGuard {
public:
    DaemonGuard() {}

    std::string ToString() const;

    int RealStart(int argc, char **argv,
                  std::function<int(int argc, char **argv)> mainCb);

    int RealDaemon(int argc, char **argv,
                   std::function<int(int argc, char **argv)> mainCb);

    int StartDaemon(int argc, char **argv,
                    std::function<int(int argc, char **argv)> mainCb,
                    bool isDaemon);

private:
#ifdef _WIN32
    HANDLE m_parentId = 0;
    HANDLE m_mainId = 0;
#else
    pid_t m_parentId = 0;
    pid_t m_mainId = 0;
#endif
    time_t m_parentStartTime = 0;
    time_t m_mainStartTime = 0;
    int m_restartCount = 0;
};
```

## Usage Examples

### 1. RealStart Method

```cpp
// Create an instance of DaemonGuard
DaemonGuard daemon;

// Define a callback function
std::function<int(int argc, char **argv)> callback = [](int argc, char **argv) {
    // Task implementation
    return 0;
};

// Start a child process using RealStart
int result = daemon.RealStart(argc, argv, callback);
```

### 2. RealDaemon Method

```cpp
// Create an instance of DaemonGuard
DaemonGuard daemon;

// Start a daemon process using RealDaemon
int result = daemon.RealDaemon(argc, argv, callback);
```

### 3. StartDaemon Method

```cpp
// Create an instance of DaemonGuard
DaemonGuard daemon;

// Start a process with or without creating a daemon
bool createDaemon = true;
int result = daemon.StartDaemon(argc, argv, callback, createDaemon);
```

### Additional Notes

- The `SignalHandler`, `WritePidFile`, and `CheckPidFile` functions are external utility functions related to process management but are not part of the `DaemonGuard` class.

## SignalHandler Function

```cpp
/**
 * @brief Signal handler function.
 *
 * @param signum The signal number.
 */
void SignalHandler(int signum) {
    // Signal handling logic
}
```

## WritePidFile Function

```cpp
/**
 * @brief Writes the process ID to a file.
 */
void WritePidFile() {
    // Implementation to write PID to a file
}
```

## CheckPidFile Function

```cpp
/**
 * @brief Checks if the process ID file exists.
 *
 * @return True if the process ID file exists, false otherwise.
 */
bool CheckPidFile() {
    // Implementation to check the existence of the PID file
    return false; // Placeholder return value
}
```
