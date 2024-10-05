# Software-related Functions Documentation

## Overview

This documentation covers a set of functions designed to interact with software installations and applications on a system. These functions are part of the `atom::system` namespace and provide capabilities such as checking software installation status, retrieving application versions, paths, and permissions.

## Function Definitions

### Check Software Installation

```cpp
auto checkSoftwareInstalled(const std::string& software_name) -> bool;
```

Checks whether the specified software is installed on the system.

- **Parameters:**
  - `software_name`: The name of the software to check.
- **Returns:** `true` if the software is installed, `false` if it's not installed or an error occurred.

### Get Application Version

```cpp
auto getAppVersion(const fs::path& app_path) -> std::string;
```

Retrieves the version of the specified application.

- **Parameters:**
  - `app_path`: The path to the application.
- **Returns:** A string representing the version of the application.

### Get Application Path

```cpp
auto getAppPath(const std::string& software_name) -> fs::path;
```

Retrieves the path to the specified application.

- **Parameters:**
  - `software_name`: The name of the software.
- **Returns:** A `std::filesystem::path` object representing the path to the application.

### Get Application Permissions

```cpp
auto getAppPermissions(const fs::path& app_path) -> std::vector<std::string>;
```

Retrieves the permissions of the specified application.

- **Parameters:**
  - `app_path`: The path to the application.
- **Returns:** A vector of strings representing the permissions of the application.

## Usage Examples

### Example 1: Checking Software Installation and Getting Version

```cpp
#include "software.hpp"
#include <iostream>

int main() {
    const std::string software_name = "ExampleSoftware";

    if (atom::system::checkSoftwareInstalled(software_name)) {
        std::cout << software_name << " is installed." << std::endl;

        fs::path app_path = atom::system::getAppPath(software_name);
        std::string version = atom::system::getAppVersion(app_path);

        std::cout << "Version: " << version << std::endl;
        std::cout << "Path: " << app_path << std::endl;
    } else {
        std::cout << software_name << " is not installed." << std::endl;
    }

    return 0;
}
```

### Example 2: Getting Application Permissions

```cpp
#include "software.hpp"
#include <iostream>

int main() {
    const std::string software_name = "ExampleSoftware";
    fs::path app_path = atom::system::getAppPath(software_name);

    if (!app_path.empty()) {
        std::vector<std::string> permissions = atom::system::getAppPermissions(app_path);

        std::cout << "Permissions for " << software_name << ":" << std::endl;
        for (const auto& perm : permissions) {
            std::cout << "- " << perm << std::endl;
        }
    } else {
        std::cout << "Could not find path for " << software_name << std::endl;
    }

    return 0;
}
```

### Example 3: Checking Multiple Software Installations

```cpp
#include "software.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<std::string> software_list = {
        "ExampleSoftware1",
        "ExampleSoftware2",
        "ExampleSoftware3"
    };

    for (const auto& software : software_list) {
        if (atom::system::checkSoftwareInstalled(software)) {
            std::cout << software << " is installed." << std::endl;
            fs::path app_path = atom::system::getAppPath(software);
            std::cout << "Path: " << app_path << std::endl;
        } else {
            std::cout << software << " is not installed." << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
```

## Important Considerations

1. **Cross-platform Compatibility**: These functions may behave differently on different operating systems. Ensure you handle platform-specific cases appropriately.

2. **Error Handling**: The functions may return empty strings, paths, or vectors in case of errors. Implement proper error checking in your code.

3. **Performance**: Functions like `checkSoftwareInstalled` and `getAppPath` might involve system-wide searches, which could be time-consuming for large systems.

4. **Permissions**: Depending on the system configuration, some of these functions might require elevated privileges to access certain software information.

5. **Software Names**: The `software_name` parameter in functions like `checkSoftwareInstalled` and `getAppPath` might need to match the exact name used in system registries or package managers.

## Best Practices

1. **Cache Results**: If you need to check for the same software multiple times, consider caching the results to improve performance.

2. **Handle Empty Returns**: Always check if returned paths or strings are empty before using them.

3. **Use with System Administration Tools**: These functions can be particularly useful in system administration scripts or tools.

4. **Version Comparisons**: When using `getAppVersion`, remember that version strings might not always be directly comparable. You might need to implement a version comparison function.

5. **Permission Handling**: When using `getAppPermissions`, be prepared to handle a variety of permission formats that might differ across operating systems.

## Conclusion

These software-related functions provide a convenient interface for querying information about installed software and applications on a system. They can be particularly useful for system administration tasks, software inventory management, or as part of larger application deployment and management systems.

When using these functions, be mindful of potential performance implications and system-specific behaviors. Always implement proper error checking and handling in your code to ensure robustness across different system configurations.
