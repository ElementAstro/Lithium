# atom::log::Logger Documentation

This document provides a detailed explanation of the `atom::log::Logger` class, its methods, and usage examples.

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor](#constructor)
4. [Logging Methods](#logging-methods)
5. [Configuration Methods](#configuration-methods)
6. [Sink Management](#sink-management)
7. [Usage Examples](#usage-examples)

## Introduction

The `atom::log::Logger` class is part of the `atom::log` namespace and provides a flexible logging system for C++ applications. It supports multiple log levels, file rotation, and customizable logging patterns.

## Class Overview

```cpp
namespace atom::log {

enum class LogLevel {
    TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL, OFF
};

class Logger {
public:
    explicit Logger(const fs::path& file_name,
                    LogLevel min_level = LogLevel::TRACE,
                    size_t max_file_size = 1048576, int max_files = 10);
    ~Logger();

    // Logging methods
    template <typename... Args>
    void trace(const std::string& format, const Args&... args);
    template <typename... Args>
    void debug(const std::string& format, const Args&... args);
    template <typename... Args>
    void info(const std::string& format, const Args&... args);
    template <typename... Args>
    void warn(const std::string& format, const Args&... args);
    template <typename... Args>
    void error(const std::string& format, const Args&... args);
    template <typename... Args>
    void critical(const std::string& format, const Args&... args);

    // Configuration methods
    void setLevel(LogLevel level);
    void setPattern(const std::string& pattern);
    void setThreadName(const std::string& name);

    // Sink management
    void registerSink(const std::shared_ptr<Logger>& logger);
    void removeSink(const std::shared_ptr<Logger>& logger);
    void clearSinks();

    void enableSystemLogging(bool enable);

private:
    std::shared_ptr<LoggerImpl> impl_;
    void log(LogLevel level, const std::string& msg);
};

} // namespace atom::log
```

## Constructor

```cpp
explicit Logger(const fs::path& file_name,
                LogLevel min_level = LogLevel::TRACE,
                size_t max_file_size = 1048576, int max_files = 10);
```

Creates a new `Logger` instance.

- `file_name`: The path and name of the log file.
- `min_level`: The minimum log level to record (default: TRACE).
- `max_file_size`: Maximum size of a single log file in bytes (default: 1MB).
- `max_files`: Maximum number of log files to keep for rotation (default: 10).

Example:

```cpp
atom::log::Logger logger("app.log", atom::log::LogLevel::INFO, 5 * 1024 * 1024, 5);
```

## Logging Methods

The `Logger` class provides methods for each log level:

```cpp
template <typename... Args>
void trace(const std::string& format, const Args&... args);

template <typename... Args>
void debug(const std::string& format, const Args&... args);

template <typename... Args>
void info(const std::string& format, const Args&... args);

template <typename... Args>
void warn(const std::string& format, const Args&... args);

template <typename... Args>
void error(const std::string& format, const Args&... args);

template <typename... Args>
void critical(const std::string& format, const Args&... args);
```

These methods use C++20's `std::format` for string formatting.

Example:

```cpp
logger.info("Application started. Version: {}", app_version);
logger.error("Failed to open file: {}", filename);
```

## Configuration Methods

### setLevel

```cpp
void setLevel(LogLevel level);
```

Sets the minimum log level. Messages below this level will be ignored.

Example:

```cpp
logger.setLevel(atom::log::LogLevel::WARN);
```

### setPattern

```cpp
void setPattern(const std::string& pattern);
```

Sets the logging pattern. (Note: The pattern format is not specified in the header.)

Example:

```cpp
logger.setPattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
```

### setThreadName

```cpp
void setThreadName(const std::string& name);
```

Sets a name for the current thread in log messages.

Example:

```cpp
logger.setThreadName("WorkerThread");
```

## Sink Management

### registerSink

```cpp
void registerSink(const std::shared_ptr<Logger>& logger);
```

Registers another logger as a sink. Log messages will be forwarded to this logger.

Example:

```cpp
auto console_logger = std::make_shared<atom::log::Logger>("console");
file_logger.registerSink(console_logger);
```

### removeSink

```cpp
void removeSink(const std::shared_ptr<Logger>& logger);
```

Removes a previously registered sink logger.

Example:

```cpp
file_logger.removeSink(console_logger);
```

### clearSinks

```cpp
void clearSinks();
```

Removes all registered sink loggers.

Example:

```cpp
logger.clearSinks();
```

### enableSystemLogging

```cpp
void enableSystemLogging(bool enable);
```

Enables or disables system logging. (Note: The specifics of system logging are not detailed in the header.)

Example:

```cpp
logger.enableSystemLogging(true);
```

## Usage Examples

### Basic Logging

```cpp
#include "atom/log/atomlog.hpp"

int main() {
    atom::log::Logger logger("app.log");

    logger.info("Application started");
    logger.debug("Debug message: {}", 42);
    logger.warn("Warning: Low disk space ({}%)", 10);

    try {
        // Some risky operation
        throw std::runtime_error("Example error");
    } catch (const std::exception& e) {
        logger.error("Caught exception: {}", e.what());
    }

    logger.info("Application shutting down");
    return 0;
}
```

### Advanced Configuration

```cpp
#include "atom/log/atomlog.hpp"
#include <memory>

int main() {
    auto file_logger = std::make_shared<atom::log::Logger>("app.log", atom::log::LogLevel::DEBUG);
    auto console_logger = std::make_shared<atom::log::Logger>("console", atom::log::LogLevel::INFO);

    file_logger->setPattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    file_logger->registerSink(console_logger);

    file_logger->setThreadName("MainThread");

    file_logger->info("This message goes to both file and console");
    file_logger->debug("This only goes to the file");

    return 0;
}
```
