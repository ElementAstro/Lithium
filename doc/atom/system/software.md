# Atom System Software Module

## Overview

The `atom::system` namespace provides a set of functions for querying and managing software installations on a system. This module is part of the Atom library and is defined in the `ATOM_SYSTEM_SOFTWARE_HPP` header file.

## Functions

### `checkSoftwareInstalled`

```cpp
auto checkSoftwareInstalled(const std::string& software_name) -> bool;
```

#### Description

This function checks whether the specified software is installed on the system.

#### Parameters

- `software_name`: A string representing the name of the software to check.

#### Returns

- `true` if the software is installed.
- `false` if the software is not installed or an error occurred during the check.

#### Usage Example

```cpp
if (atom::system::checkSoftwareInstalled("Firefox")) {
    std::cout << "Firefox is installed on this system." << std::endl;
} else {
    std::cout << "Firefox is not installed or could not be detected." << std::endl;
}
```

### `getAppVersion`

```cpp
auto getAppVersion(const fs::path& app_path) -> std::string;
```

#### Description

This function retrieves the version of the specified application.

#### Parameters

- `app_path`: A filesystem path object representing the path to the application.

#### Returns

A string containing the version of the application.

#### Usage Example

```cpp
fs::path firefoxPath = "/usr/bin/firefox";
std::string version = atom::system::getAppVersion(firefoxPath);
std::cout << "Firefox version: " << version << std::endl;
```

### `getAppPath`

```cpp
auto getAppPath(const std::string& software_name) -> fs::path;
```

#### Description

This function retrieves the path to the specified application.

#### Parameters

- `software_name`: A string representing the name of the software.

#### Returns

A filesystem path object representing the path to the application.

#### Usage Example

```cpp
fs::path chromePath = atom::system::getAppPath("Google Chrome");
std::cout << "Chrome is installed at: " << chromePath << std::endl;
```

### `getAppPermissions`

```cpp
auto getAppPermissions(const fs::path& app_path) -> std::vector<std::string>;
```

#### Description

This function retrieves the permissions of the specified application.

#### Parameters

- `app_path`: A filesystem path object representing the path to the application.

#### Returns

A vector of strings, where each string represents a permission of the application.

#### Usage Example

```cpp
fs::path safariPath = "/Applications/Safari.app";
std::vector<std::string> permissions = atom::system::getAppPermissions(safariPath);
std::cout << "Safari permissions:" << std::endl;
for (const auto& perm : permissions) {
    std::cout << "- " << perm << std::endl;
}
```

## Notes

- This module uses the C++17 filesystem library, aliased as `fs`.
- All functions are part of the `atom::system` namespace.
- Error handling is not explicitly shown in the function signatures, so proper error checking should be implemented when using these functions in production code.
- The actual implementation of these functions is not provided in the header file, so they must be defined elsewhere in the project.

## Dependencies

- C++17 or later
- `<filesystem>` header
- `<string>` header
- `<vector>` header

## Compilation

When compiling a program that uses this module, ensure that C++17 features are enabled. For example, with GCC or Clang, you might use:

```
g++ -std=c++17 your_program.cpp -o your_program
```

or

```
clang++ -std=c++17 your_program.cpp -o your_program
```
