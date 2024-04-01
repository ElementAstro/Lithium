# Process Management Library Documentation

## ProcessInfo Struct

- Stores information about a process.
- Attributes:
  - `processID` (int): The process ID.
  - `parentProcessID` (int): The parent process ID.
  - `basePriority` (int): The base priority of the process.
  - `executableFile` (std::string): The executable file associated with the process.

```cpp
ProcessInfo info;
info.processID = 12345;
info.parentProcessID = 6789;
info.basePriority = 10;
info.executableFile = "example.exe";
```

## Function: CheckSoftwareInstalled

- Checks if the specified software is installed.

- `software_name` (const std::string&): The name of the software.

- `true` if the software is installed, `false` otherwise.

```cpp
bool isInstalled = CheckSoftwareInstalled("Example Software");
// Expected output: false
```

## Function: checkExecutableFile

- Checks if the specified file exists.

- `fileName` (const std::string&): The name of the file.
- `fileExt` (const std::string&): The extension of the file.

- `true` if the file exists, `false` otherwise.

## Function: IsRoot

- Checks if the current user has root/administrator privileges.

- `true` if the user has root/administrator privileges, `false` otherwise.

## Function: GetCurrentUsername

- Retrieves the current username.

- The current username as a string.

## Function: Shutdown

- Shuts down the system.

- `true` if the system is successfully shutdown, `false` if an error occurs.

## Function: Reboot

- Reboots the system.

- `true` if the system is successfully rebooted, `false` if an error occurs.

## Function: GetProcessInfo

- Retrieves process information and file addresses.

- A vector of pairs containing process information and file addresses.

## Function: CheckDuplicateProcess

- Checks for duplicate processes with the same program name.

- `program_name` (const std::string&): The program name to check for duplicates.

## Function: isProcessRunning

- Checks if the specified process is running.

- `processName` (const std::string&): The name of the process to check.

- `true` if the process is running, `false` otherwise.

## Function: GetProcessDetails

- Retrieves detailed process information.

- A vector of ProcessInfo structs containing process details.

## Function: GetProcessInfoByID (Platform-specific)

- Retrieves process information by process ID.

- `_WIN32: DWORD processID`, `!_WIN32: int processID`: The process ID.
- `processInfo`: ProcessInfo struct to store the retrieved information.

- `true` if the process information is successfully retrieved, `false` if an error occurs.

## Function: GetProcessInfoByName

- Retrieves process information by process name.

- `processName` (const std::string&): The process name.
- `processInfo`: ProcessInfo struct to store the retrieved information.

- `true` if the process information is successfully retrieved, `false` if an error occurs.
