# Windows Registry Functions Documentation

## Overview

This documentation covers a set of functions designed to interact with the Windows Registry. These functions are part of the `atom::system` namespace and provide capabilities such as querying, modifying, and deleting registry keys and values, as well as more advanced operations like recursive enumeration, backup, and export.

## Important Note

These functions are specifically for Windows systems and are only available when compiling for Windows (i.e., when `_WIN32` is defined).

## Function Definitions

### Get Registry Subkeys

```cpp
[[nodiscard]] auto getRegistrySubKeys(HKEY hRootKey, std::string_view subKey,
                                      std::vector<std::string>& subKeys) -> bool;
```

Retrieves all subkey names under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.
  - `subKeys`: Vector of strings to store the subkey names.
- **Returns:** `true` if successful, `false` otherwise.

### Get Registry Values

```cpp
[[nodiscard]] auto getRegistryValues(
    HKEY hRootKey, std::string_view subKey,
    std::vector<std::pair<std::string, std::string>>& values) -> bool;
```

Retrieves all value names and data under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.
  - `values`: Vector of string pairs to store the value names and data.
- **Returns:** `true` if successful, `false` otherwise.

### Modify Registry Value

```cpp
[[nodiscard]] auto modifyRegistryValue(HKEY hRootKey, std::string_view subKey,
                                       std::string_view valueName,
                                       std::string_view newValue) -> bool;
```

Modifies the data of a specified value under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.
  - `valueName`: Name of the value to modify.
  - `newValue`: New data for the value.
- **Returns:** `true` if successful, `false` otherwise.

### Delete Registry Subkey

```cpp
[[nodiscard]] auto deleteRegistrySubKey(HKEY hRootKey,
                                        std::string_view subKey) -> bool;
```

Deletes the specified registry key and all its subkeys.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the key to delete, can include multiple nested keys separated by backslashes.
- **Returns:** `true` if successful, `false` otherwise.

### Delete Registry Value

```cpp
[[nodiscard]] auto deleteRegistryValue(HKEY hRootKey, std::string_view subKey,
                                       std::string_view valueName) -> bool;
```

Deletes a specified value under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.
  - `valueName`: Name of the value to delete.
- **Returns:** `true` if successful, `false` otherwise.

### Recursively Enumerate Registry Subkeys

```cpp
void recursivelyEnumerateRegistrySubKeys(HKEY hRootKey,
                                         std::string_view subKey);
```

Recursively enumerates all subkeys and values under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.

### Backup Registry

```cpp
[[nodiscard]] auto backupRegistry(HKEY hRootKey, std::string_view subKey,
                                  std::string_view backupFilePath) -> bool;
```

Backs up the specified registry key and all its subkeys and values.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the key to backup, can include multiple nested keys separated by backslashes.
  - `backupFilePath`: Full path of the backup file.
- **Returns:** `true` if successful, `false` otherwise.

### Find Registry Key

```cpp
void findRegistryKey(HKEY hRootKey, std::string_view subKey,
                     std::string_view searchKey);
```

Recursively searches for subkey names containing the specified string under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.
  - `searchKey`: String to search for in subkey names.

### Find Registry Value

```cpp
void findRegistryValue(HKEY hRootKey, std::string_view subKey,
                       std::string_view searchValue);
```

Recursively searches for value names and data containing the specified string under the specified registry key.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the specified key, can include multiple nested keys separated by backslashes.
  - `searchValue`: String to search for in value names and data.

### Export Registry

```cpp
[[nodiscard]] auto exportRegistry(HKEY hRootKey, std::string_view subKey,
                                  std::string_view exportFilePath) -> bool;
```

Exports the specified registry key and all its subkeys and values to a REG file.

- **Parameters:**
  - `hRootKey`: Handle to the root key.
  - `subKey`: Name of the key to export, can include multiple nested keys separated by backslashes.
  - `exportFilePath`: Full path of the export file.
- **Returns:** `true` if successful, `false` otherwise.

## Usage Examples

### Example 1: Retrieving and Modifying Registry Values

```cpp
#include "wregistry.hpp"
#include <iostream>
#include <vector>
#include <windows.h>

int main() {
    std::vector<std::pair<std::string, std::string>> values;

    if (atom::system::getRegistryValues(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", values)) {
        for (const auto& [name, data] : values) {
            std::cout << "Name: " << name << ", Data: " << data << std::endl;
        }
    }

    if (atom::system::modifyRegistryValue(HKEY_CURRENT_USER, "Software\\MyApp", "LastRun", "2023-06-17")) {
        std::cout << "Registry value modified successfully." << std::endl;
    }

    return 0;
}
```

### Example 2: Recursive Enumeration and Search

```cpp
#include "wregistry.hpp"
#include <windows.h>

int main() {
    atom::system::recursivelyEnumerateRegistrySubKeys(HKEY_LOCAL_MACHINE, "SOFTWARE");

    atom::system::findRegistryKey(HKEY_LOCAL_MACHINE, "SOFTWARE", "Microsoft");
    atom::system::findRegistryValue(HKEY_LOCAL_MACHINE, "SOFTWARE", "Version");

    return 0;
}
```

### Example 3: Backup and Export Registry

```cpp
#include "wregistry.hpp"
#include <iostream>
#include <windows.h>

int main() {
    if (atom::system::backupRegistry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft", "C:\\backup.reg")) {
        std::cout << "Registry backup created successfully." << std::endl;
    }

    if (atom::system::exportRegistry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft", "C:\\export.reg")) {
        std::cout << "Registry key exported successfully." << std::endl;
    }

    return 0;
}
```

## Important Considerations

1. **Administrator Rights**: Many registry operations, especially those involving HKEY_LOCAL_MACHINE, require administrator privileges. Ensure your application has the necessary permissions.

2. **Error Handling**: These functions return bool values to indicate success or failure. In a production environment, you should implement proper error handling and logging.

3. **String Encoding**: The functions use `std::string_view`, which implies UTF-8 encoding. Ensure your string literals are properly encoded when working with non-ASCII characters in registry keys or values.

4. **Performance**: Recursive operations like `recursivelyEnumerateRegistrySubKeys`, `findRegistryKey`, and `findRegistryValue` can be time-consuming for large registry hierarchies. Use them judiciously.

5. **Backup Before Modifying**: Always create a backup of the registry or the specific keys you're modifying before making changes. The `backupRegistry` function can be useful for this purpose.

6. **Security Implications**: Modifying the registry can have significant impacts on system behavior and stability. Be extremely careful when using functions that modify or delete registry keys and values.

## Best Practices

1. **Least Privilege**: When possible, use HKEY_CURRENT_USER instead of HKEY_LOCAL_MACHINE to minimize the need for elevated privileges.

2. **Error Checking**: Always check the return values of these functions and handle potential failures gracefully.

3. **Targeted Operations**: Be as specific as possible with the subkey paths to minimize the scope of your registry operations.

4. **Cleanup**: If your application creates temporary registry entries, ensure you have logic to clean them up when they're no longer needed.

5. **Validation**: When modifying registry values, validate the data you're writing to ensure it's in the correct format and within expected ranges.

6. **Documentation**: Keep clear documentation of what registry keys and values your application uses, modifies, or depends on.

## Conclusion

These Windows Registry functions provide a powerful interface for interacting with the Windows Registry using C++. They offer a range of capabilities from simple key-value operations to more complex tasks like recursive enumeration and registry backups.

When using these functions, always be mindful of the potential system-wide impact of registry modifications. Thoroughly test your application's registry interactions in a controlled environment before deploying to production systems.

Remember that these functions are Windows-specific and should be properly isolated in cross-platform code. Use conditional compilation (`#ifdef _WIN32`) to ensure these functions are only compiled and used on Windows systems.
