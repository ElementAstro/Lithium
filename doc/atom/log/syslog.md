# SyslogWrapper Class

The `SyslogWrapper` class encapsulates the functionality of system logging.

## Constructor

## `SyslogWrapper()`

- Default constructor sets log level to `Info` and outputs to "Event".

```cpp
SyslogWrapper logger;
logger.info("System initialized."); // Expected output: [Info] System initialized.
```

## `SyslogWrapper(LogLevel logLevel, const std::string &target = "")`

- Constructor with parameters to specify log level and output target.

```cpp
SyslogWrapper customLogger(LogLevel::Warning, "Console");
customLogger.warning("Memory low"); // Expected output: [Warning] Memory low
```

## Destructor

## `~SyslogWrapper() noexcept`

- Destructor to close handles or perform cleanup tasks.

## Logging Methods

## `void log(LogLevel level, const char *format, Args &&...args)`

- Log method supporting formatted strings with variable arguments.

```cpp
logger.log(LogLevel::Debug, "Value: %d", 42); // Expected output: [Debug] Value: 42
```

## `void log(LogLevel level, const char *format, Arg &&arg, Args &&...args)`

- Overloaded log method for a single argument followed by variable arguments.

```cpp
customLogger.log(LogLevel::Error, "Error code: %d", 404, "Not Found"); // Expected output: [Error] Error code: 404 Not Found
```

## Log Level Methods

## `void setLogLevel(LogLevel logLevel)`

- Set the log level.

```cpp
logger.setLogLevel(LogLevel::Error);
logger.info("This info message will not be logged."); // No output
```

## `LogLevel getLogLevel() const`

- Get the current log level.

## Logging Convenience Methods

## `void debug(const char *format, Args &&...args)`

- Log debug information.

```cpp
logger.debug("Debug message: %s", "Debugging in progress"); // Expected output: [Debug] Debug message: Debugging in progress
```

## `void info(const char *format, Args &&...args)`

- Log information.

```cpp
logger.info("Information: %s", "System is running smoothly"); // Expected output: [Info] Information: System is running smoothly
```

## `void warning(const char *format, Args &&...args)`

- Log warning information.

```cpp
logger.warning("Warning: %s", "Low disk space"); // Expected output: [Warning] Warning: Low disk space
```

## `void error(const char *format, Args &&...args)`

- Log error information.

```cpp
logger.error("Error: %s", "File not found"); // Expected output: [Error] Error: File not found
```

## Private Members

- `HANDLE m_eventHandle;` (Windows specific)
- `std::string m_target;`
- `LogLevel m_logLevel;`
- `std::mutex m_mutex;`

## Special Note:

- Ensure proper synchronization while accessing logging methods in a multithreaded environment by using the mutex `m_mutex`.
- Make sure to handle platform-specific considerations when using `m_eventHandle`.
