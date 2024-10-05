# LoggerManager Documentation

This document provides a detailed explanation of the `lithium::LoggerManager` class, its methods, and usage examples.

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [LogEntry Struct](#logentry-struct)
4. [Constructor and Destructor](#constructor-and-destructor)
5. [Public Methods](#public-methods)
6. [Usage Examples](#usage-examples)

## Introduction

The `lithium::LoggerManager` class is part of the `lithium` namespace and provides functionality for managing log files. It includes methods for scanning log folders, searching logs, uploading files, and analyzing logs.

## Class Overview

```cpp
namespace lithium {

class LoggerManager {
public:
    LoggerManager();
    ~LoggerManager();

    void scanLogsFolder(const std::string &folderPath);
    std::vector<LogEntry> searchLogs(std::string_view keyword);
    void uploadFile(const std::string &filePath);
    void analyzeLogs();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace lithium
```

The class uses the Pimpl (Pointer to Implementation) design pattern, which helps in reducing compilation dependencies and provides better encapsulation.

## LogEntry Struct

```cpp
struct LogEntry {
    std::string fileName;
    int lineNumber;
    std::string message;
} ATOM_ALIGNAS(128);
```

The `LogEntry` struct represents a single log entry with the following fields:

- `fileName`: The name of the file containing the log entry.
- `lineNumber`: The line number of the log entry in the file.
- `message`: The content of the log message.

The struct is aligned to 128 bytes using the `ATOM_ALIGNAS(128)` macro, which can help improve memory access performance on some systems.

## Constructor and Destructor

```cpp
LoggerManager();
~LoggerManager();
```

The constructor and destructor are declared but not defined in the header. They are likely implemented in the corresponding `.cpp` file.

## Public Methods

### scanLogsFolder

```cpp
void scanLogsFolder(const std::string &folderPath);
```

This method scans a specified folder for log files.

Parameters:

- `folderPath`: The path to the folder containing log files.

### searchLogs

```cpp
std::vector<LogEntry> searchLogs(std::string_view keyword);
```

This method searches for log entries containing a specific keyword.

Parameters:

- `keyword`: The keyword to search for in the log entries.

Returns:

- A vector of `LogEntry` objects that match the search criteria.

### uploadFile

```cpp
void uploadFile(const std std::string &filePath);
```

This method uploads a specified file, presumably a log file.

Parameters:

- `filePath`: The path to the file to be uploaded.

### analyzeLogs

```cpp
void analyzeLogs();
```

This method performs analysis on the logs. The specific type of analysis is not detailed in the header and would be implemented in the `.cpp` file.

## Usage Examples

Here are some examples of how to use the `LoggerManager` class:

### Basic Usage

```cpp
#include "logger.hpp"
#include <iostream>

int main() {
    lithium::LoggerManager logManager;

    // Scan a logs folder
    logManager.scanLogsFolder("/path/to/logs");

    // Search for logs containing "error"
    auto errorLogs = logManager.searchLogs("error");
    for (const auto& log : errorLogs) {
        std::cout << "Error in " << log.fileName << " at line " << log.lineNumber << ": " << log.message << std::endl;
    }

    // Upload a specific log file
    logManager.uploadFile("/path/to/logs/important.log");

    // Analyze logs
    logManager.analyzeLogs();

    return 0;
}
```

### Advanced Usage

```cpp
#include "logger.hpp"
#include <iostream>
#include <stdexcept>

void processLogs(lithium::LoggerManager& manager, const std::string& folderPath) {
    try {
        manager.scanLogsFolder(folderPath);

        auto warningLogs = manager.searchLogs("warning");
        std::cout << "Found " << warningLogs.size() << " warnings." << std::endl;

        auto errorLogs = manager.searchLogs("error");
        std::cout << "Found " << errorLogs.size() << " errors." << std::endl;

        if (!errorLogs.empty()) {
            std::cout << "Uploading error logs..." << std::endl;
            for (const auto& log : errorLogs) {
                manager.uploadFile(log.fileName);
            }
        }

        manager.analyzeLogs();
    } catch (const std::exception& e) {
        std::cerr << "Error processing logs: " << e.what() << std::endl;
    }
}

int main() {
    lithium::LoggerManager logManager;
    processLogs(logManager, "/var/log/application");
    return 0;
}
```
