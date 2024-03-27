# C++ System Utility Functions Documentation

## Utsname Struct

Represents information about the operating system.

### Members

- `sysname`: Operating system name
- `nodename`: Network host name
- `release`: Operating system release version
- `version`: Operating system internal version
- `machine`: Hardware identifier

## walk Function

Recursively walks through a directory and its subdirectories.

```cpp
void walk(const fs::path &root);
```

```cpp
// Example Usage of walk function
walk("/path/to/directory");
```

## jwalk Function

Recursively walks through a directory and its subdirectories, returning file information as a JSON string.

```cpp
std::string jwalk(const std::string &root);
```

```cpp
// Example Usage of jwalk function
std::string jsonFiles = jwalk("/path/to/directory");
// Expected Output: JSON string containing file information.

```

## fwalk Function

Recursively walks through a directory and its subdirectories, applying a callback function to each file.

```cpp
void fwalk(const fs::path &root, const std::function<void(const fs::path &)> &callback);
```

```cpp
// Example Usage of fwalk function
fwalk("/path/to/directory", [](const fs::path &file) {
    // Custom callback function for each file
});
```

## Environ Function

Retrieves environment variables as a key-value map.

```cpp
std::unordered_map<std::string, std::string> Environ();
```

```cpp
// Example Usage of Environ function
auto envMap = Environ();
// Expected Output: Unordered map containing environment variables and values.
```

## ctermid Function

Returns the name of the controlling terminal.

```cpp
std::string ctermid();
```

```cpp
// Example Usage of ctermid function
std::string termName = ctermid();
// Expected Output: Name of the controlling terminal.
```

## getpriority Function

Retrieves the priority of the current process.

```cpp
int getpriority();
```

```cpp
// Example Usage of getpriority function
int priority = getpriority();
// Expected Output: Priority of the current process.
```

## getlogin Function

Retrieves the login name of the user.

```cpp
std::string getlogin();
```

```cpp
// Example Usage of getlogin function
std::string username = getlogin();
// Expected Output: Login name of the user associated with the process.
```

## uname Function

Retrieves the operating system name and related information.

```cpp
Utsname uname();
```

```cpp
// Example Usage of uname function
Utsname systemInfo = uname();
// Access individual fields like systemInfo.sysname, systemInfo.release, etc.
```

This documentation provides detailed information about various C++ system utility functions along with usage examples and expected outputs. Ensure to replace placeholder paths like `"/path/to/directory"` with actual paths when testing these functions in your implementation.
