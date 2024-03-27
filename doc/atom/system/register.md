# Registry Manipulation Functions

This document describes a set of functions for manipulating the Windows registry.

## getRegistrySubKeys

Get all subkey names under a specified registry key.

```cpp
bool getRegistrySubKeys(HKEY hRootKey, const std::string &subKey,
                        std::vector<std::string> &subKeys);
```

```cpp
std::vector<std::string> subKeys;
if (getRegistrySubKeys(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft", subKeys)) {
    for (const auto &key : subKeys) {
        std::cout << key << std::endl;
    }
}
```

## getRegistryValues

Get all value names and data under a specified registry key.

```cpp
bool getRegistryValues(
    HKEY hRootKey, const std::string &subKey,
    std::vector<std::pair<std::string, std::string>> &values);
```

```cpp
std::vector<std::pair<std::string, std::string>> values;
if (getRegistryValues(HKEY_CURRENT_USER, "Software\\Classes", values)) {
    for (const auto &value : values) {
        std::cout << value.first << ": " << value.second << std::endl;
    }
}
```

## modifyRegistryValue

Modify the data of a specified value under a registry key.

```cpp
bool modifyRegistryValue(HKEY hRootKey, const std::string &subKey,
                         const std::string &valueName,
                         const std::string &newValue);
```

```cpp
if (modifyRegistryValue(HKEY_CURRENT_USER, "Control Panel\\Desktop", "Wallpaper", "new_wallpaper.jpg")) {
    std::cout << "Value modified successfully." << std::endl;
}
```

## deleteRegistrySubKey

Delete a specified registry key and all its subkeys.

```cpp
bool deleteRegistrySubKey(HKEY hRootKey, const std::string &subKey);
```

```cpp
if (deleteRegistrySubKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\MyApp")) {
    std::cout << "Key and subkeys deleted successfully." << std::endl;
}
```

## deleteRegistryValue

Delete a specified value under a registry key.

```cpp
bool deleteRegistryValue(HKEY hRootKey, const std::string &subKey,
                         const std::string &valueName);
```

```cpp
if (deleteRegistryValue(HKEY_CURRENT_USER, "Software\\MyApp", "Setting")) {
    std::cout << "Value deleted successfully." << std::endl;
}
```

## recursivelyEnumerateRegistrySubKeys

Recursively enumerate all subkeys and values under a specified registry key.

```cpp
void recursivelyEnumerateRegistrySubKeys(HKEY hRootKey, const std::string &subKey);
```

### Special Note

This function will output all subkeys and values under the specified key.

## backupRegistry

Backup a specified registry key and all its subkeys and values.

```cpp
bool backupRegistry(HKEY hRootKey, const std::string &subKey,
                    const std::string &backupFilePath);
```

```cpp
if (backupRegistry(HKEY_LOCAL_MACHINE, "SOFTWARE\\MyApp", "C:\\backup\\myapp_backup.reg")) {
    std::cout << "Registry backed up successfully." << std::endl;
}
```

## findRegistryKey

Recursively search for subkey names containing a specified string under a registry key.

```cpp
void findRegistryKey(HKEY hRootKey, const std::string &subKey,
                     const std::string &searchKey);
```

```cpp
findRegistryKey(HKEY_CURRENT_USER, "Software", "Microsoft");
```

## findRegistryValue

Recursively search for value names and data containing a specified string under a registry key.

```cpp
void findRegistryValue(HKEY hRootKey, const std::string &subKey,
                       const std::string &searchValue);
```

```cpp
findRegistryValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft", "Version");
```

## exportRegistry

Export a specified registry key and all its subkeys and values to a .reg file.

```cpp
bool exportRegistry(HKEY hRootKey, const std::string &subKey,
                    const std::string &exportFilePath);
```

```cpp
if (exportRegistry(HKEY_CURRENT_USER, "Software\\MyApp", "C:\\exports\\myapp_export.reg")) {
    std::cout << "Registry exported successfully." << std::endl;
}
```
