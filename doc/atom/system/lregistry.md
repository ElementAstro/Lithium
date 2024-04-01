# Registry Class and Member Functions

The `Registry` class handles registry operations in Windows.

## Constructor

Creates a `Registry` object.

```cpp
Registry myRegistry;
```

## loadRegistryFromFile

Loads registry data from a file.

```cpp
myRegistry.loadRegistryFromFile();
```

## createKey

Creates a new key in the registry.

```cpp
myRegistry.createKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp");
```

Ensure proper permissions to create a new key.

## deleteKey

Deletes a key from the registry.

```cpp
myRegistry.deleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp");
```

## setValue

Sets a value for a key in the registry.

```cpp
myRegistry.setValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version", "1.0");
```

## getValue

Gets the value associated with a key and value name from the registry.

```cpp
std::string value = myRegistry.getValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version");
// Expected Output: "1.0"
```

## deleteValue

Deletes a value from a key in the registry.

```cpp
myRegistry.deleteValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version");
```

## backupRegistryData

Backs up the registry data.

```cpp
myRegistry.backupRegistryData();
```

## restoreRegistryData

Restores the registry data from a backup file.

```cpp
myRegistry.restoreRegistryData("C:\\backup\\myapp_backup.reg");
```

## keyExists

Checks if a key exists in the registry.

```cpp
bool exists = myRegistry.keyExists("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp");
// Expected Output: true or false
```

## valueExists

Checks if a value exists for a key in the registry.

```cpp
bool exists = myRegistry.valueExists("HKEY_CURRENT_USER\\Software\\MyApp", "Version");
// Expected Output: true or false
```

## getValueNames

Retrieves all value names for a given key from the registry.

```cpp
std::vector<std::string> valueNames = myRegistry.getValueNames("HKEY_CURRENT_USER\\Software\\MyApp");
```

## RegistryImpl Class

This is the implementation class for the `Registry` class.

### Usage Note

This class is used internally by the `Registry` class for managing registry data.

## saveRegistryToFile

Saves the registry data to a file.

## notifyEvent

Notifies an event related to registry operations.

This function is used for notifying events such as key creation or deletion.
