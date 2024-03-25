# LoggerManager Class

## Brief

The LoggerManager class is used for scanning, analyzing, and uploading log files.

## Constructor

```cpp
LoggerManager loggerManager;
```

## scanLogsFolder

Scans log files in a specified folder.

```cpp
loggerManager.scanLogsFolder("/path/to/folder");
```

## searchLogs

Searches for log entries containing a specific keyword.

```cpp
std::vector<LogEntry> foundLogs = loggerManager.searchLogs("error");
```

- Ensure that the `LogEntry` structure is defined and contains necessary log information.

## uploadFile

Uploads a specific file.

```cpp
loggerManager.uploadFile("/path/to/file");
```

## analyzeLogs

Analyzes the content of log files.

```cpp
loggerManager.analyzeLogs();
```

- This method performs various log analysis tasks.

## parseLog

Parses a log file.

```cpp
loggerManager.parseLog("/path/to/logfile.log");
```

## extractErrorMessages

Extracts error messages from log entries.

```cpp
std::vector<std::string> errors = loggerManager.extractErrorMessages();
```

## computeMd5Hash

Calculates the MD5 hash value of a file.

```cpp
std::string md5Hash = loggerManager.computeMd5Hash("/path/to/file");
```

## getErrorType

Gets the error type from an error message.

```cpp
std::string errorType = loggerManager.getErrorType("Error: Something went wrong");
```

## getMostCommonErrorMessage

Gets the most common error message from a collection of error messages.

```cpp
std::string commonError = loggerManager.getMostCommonErrorMessage(errors);
```

---

The `LoggerManager` class provides functionality for managing log files, including scanning, searching, analyzing, and uploading. It also offers methods for parsing logs, extracting error messages, computing MD5 hashes, and finding common error messages. Utilize these features to effectively manage and analyze log data in your application.
