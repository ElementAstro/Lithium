# Atom System Registry Management Library Documentation

This document provides a detailed explanation of the `Registry` class available in the `atom::system` namespace for managing registry operations.

## Table of Contents

1. [Registry Class](#registry-class)
   - [Constructor](#constructor)
   - [Methods](#methods)
2. [Usage Examples](#usage-examples)
3. [Best Practices and Tips](#best-practices-and-tips)

## Registry Class

The `Registry` class provides functionality for managing registry operations, including creating and deleting keys, setting and getting values, and backing up and restoring registry data.

### Constructor

```cpp
Registry();
```

Creates a new `Registry` object.

### Methods

#### loadRegistryFromFile

```cpp
void loadRegistryFromFile();
```

Loads registry data from a file.

#### createKey

```cpp
void createKey(const std::string &keyName);
```

Creates a new key in the registry.

- `keyName`: The name of the key to create.

#### deleteKey

```cpp
void deleteKey(const std::string &keyName);
```

Deletes a key from the registry.

- `keyName`: The name of the key to delete.

#### setValue

```cpp
void setValue(const std::string &keyName, const std::string &valueName, const std::string &data);
```

Sets a value for a key in the registry.

- `keyName`: The name of the key.
- `valueName`: The name of the value to set.
- `data`: The data to set for the value.

#### getValue

```cpp
auto getValue(const std::string &keyName, const std::string &valueName) -> std::string;
```

Gets the value associated with a key and value name from the registry.

- `keyName`: The name of the key.
- `valueName`: The name of the value to retrieve.
- Returns: The value associated with the key and value name.

#### deleteValue

```cpp
void deleteValue(const std::string &keyName, const std::string &valueName);
```

Deletes a value from a key in the registry.

- `keyName`: The name of the key.
- `valueName`: The name of the value to delete.

#### backupRegistryData

```cpp
void backupRegistryData();
```

Backs up the registry data.

#### restoreRegistryData

```cpp
void restoreRegistryData(const std::string &backupFile);
```

Restores the registry data from a backup file.

- `backupFile`: The backup file to restore data from.

#### keyExists

```cpp
ATOM_NODISCARD auto keyExists(const std::string &keyName) const -> bool;
```

Checks if a key exists in the registry.

- `keyName`: The name of the key to check for existence.
- Returns: `true` if the key exists, `false` otherwise.

#### valueExists

```cpp
ATOM_NODISCARD auto valueExists(const std::string &keyName, const std::string &valueName) const -> bool;
```

Checks if a value exists for a key in the registry.

- `keyName`: The name of the key.
- `valueName`: The name of the value to check for existence.
- Returns: `true` if the value exists, `false` otherwise.

#### getValueNames

```cpp
ATOM_NODISCARD auto getValueNames(const std::string &keyName) const -> std::vector<std::string>;
```

Retrieves all value names for a given key from the registry.

- `keyName`: The name of the key.
- Returns: A vector of value names associated with the given key.

## Usage Examples

### Creating and Managing Registry Keys

```cpp
#include "atom/system/registry.hpp"
#include <iostream>

int main() {
    atom::system::Registry registry;

    // Create a new key
    registry.createKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp");

    // Check if the key exists
    if (registry.keyExists("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp")) {
        std::cout << "Key created successfully!" << std::endl;
    }

    // Delete the key
    registry.deleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp");

    if (!registry.keyExists("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp")) {
        std::cout << "Key deleted successfully!" << std::endl;
    }

    return 0;
}
```

### Setting and Getting Values

```cpp
#include "atom/system/registry.hpp"
#include <iostream>

int main() {
    atom::system::Registry registry;

    // Set a value
    registry.setValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version", "1.0.0");

    // Get the value
    std::string version = registry.getValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version");
    std::cout << "MyApp Version: " << version << std::endl;

    // Check if a value exists
    if (registry.valueExists("HKEY_CURRENT_USER\\Software\\MyApp", "Version")) {
        std::cout << "Version value exists!" << std::endl;
    }

    // Delete the value
    registry.deleteValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version");

    if (!registry.valueExists("HKEY_CURRENT_USER\\Software\\MyApp", "Version")) {
        std::cout << "Version value deleted successfully!" << std::endl;
    }

    return 0;
}
```

### Backing Up and Restoring Registry Data

```cpp
#include "atom/system/registry.hpp"
#include <iostream>

int main() {
    atom::system::Registry registry;

    // Backup registry data
    registry.backupRegistryData();
    std::cout << "Registry data backed up successfully!" << std::endl;

    // Restore registry data
    registry.restoreRegistryData("registry_backup.dat");
    std::cout << "Registry data restored successfully!" << std::endl;

    return 0;
}
```

### Retrieving Value Names for a Key

```cpp
#include "atom/system/registry.hpp"
#include <iostream>

int main() {
    atom::system::Registry registry;

    // Set some values
    registry.setValue("HKEY_CURRENT_USER\\Software\\MyApp", "Version", "1.0.0");
    registry.setValue("HKEY_CURRENT_USER\\Software\\MyApp", "InstallPath", "C:\\Program Files\\MyApp");
    registry.setValue("HKEY_CURRENT_USER\\Software\\MyApp", "LastRun", "2023-06-17");

    // Get all value names for the key
    std::vector<std::string> valueNames = registry.getValueNames("HKEY_CURRENT_USER\\Software\\MyApp");

    std::cout << "Values for HKEY_CURRENT_USER\\Software\\MyApp:" << std::endl;
    for (const auto& valueName : valueNames) {
        std::cout << "- " << valueName << std::endl;
    }

    return 0;
}
```

## Best Practices and Tips

1. **Error Handling**: Implement proper error handling mechanisms when using registry operations. Many operations can fail due to permissions or other system issues.

   ```cpp
   try {
       registry.setValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp", "Version", "1.0.0");
   } catch (const std::exception& e) {
       std::cerr << "Failed to set registry value: " << e.what() << std::endl;
   }
   ```

2. **Permissions**: Be aware of the permissions required for different registry operations. Some operations may require administrator privileges.

3. **Backup Before Modifications**: Always create a backup of the registry data before making significant changes.

   ```cpp
   registry.backupRegistryData();
   // Perform registry modifications
   ```

4. **Use Consistent Key Naming**: Establish a consistent naming convention for your registry keys and values. This makes it easier to manage and locate your application's registry entries.

   ```cpp
   const std::string APP_KEY = "HKEY_CURRENT_USER\\Software\\MyCompany\\MyApp";
   registry.setValue(APP_KEY, "Version", "1.0.0");
   registry.setValue(APP_KEY, "InstallPath", "C:\\Program Files\\MyApp");
   ```

5. **Check for Existence**: Always check if a key or value exists before attempting to access or modify it.

   ```cpp
   if (registry.keyExists(APP_KEY)) {
       if (registry.valueExists(APP_KEY, "Version")) {
           std::string version = registry.getValue(APP_KEY, "Version");
           // Use the version
       }
   }
   ```

6. **Clean Up**: When your application is uninstalled, make sure to remove all related registry entries to keep the registry clean.

   ```cpp
   void cleanupRegistry() {
       Registry registry;
       registry.deleteKey("HKEY_CURRENT_USER\\Software\\MyCompany\\MyApp");
   }
   ```

7. **Minimize Registry Usage**: While the registry is useful for storing application settings, consider using configuration files for larger amounts of data or frequently changing information.

8. **Use RAII for Key Management**: Consider using RAII (Resource Acquisition Is Initialization) pattern for managing registry keys to ensure proper cleanup.

   ```cpp
   class RegistryKey {
   public:
       RegistryKey(Registry& registry, const std::string& keyName)
           : m_registry(registry), m_keyName(keyName) {
           m_registry.createKey(m_keyName);
       }
       ~RegistryKey() {
           m_registry.deleteKey(m_keyName);
       }
   private:
       Registry& m_registry;
       std::string m_keyName;
   };
   ```

9. **Thread Safety**: If your application is multi-threaded and multiple threads might access the registry simultaneously, consider implementing thread-safe access to the Registry class.

10. **Logging**: Implement logging for important registry operations, especially in production environments, to help with debugging and auditing.

    ```cpp
    void logRegistryOperation(const std::string& operation, const std::string& key) {
        // Log the operation
        std::cout << "Registry operation: " << operation << " on key: " << key << std::endl;
    }

    // Usage
    logRegistryOperation("setValue", APP_KEY);
    registry.setValue(APP_KEY, "LastRun", getCurrentDateTime());
    ```

## Implementation Details

The `Registry` class uses the Pimpl (Pointer to Implementation) idiom, as evidenced by the private `RegistryImpl` class and the `std::unique_ptr<RegistryImpl> pImpl_` member. This design pattern is used to:

1. Hide implementation details from the public interface.
2. Reduce compilation dependencies.
3. Maintain binary compatibility when the implementation changes.

The actual implementation of the registry operations is likely platform-specific and hidden within the `RegistryImpl` class. This allows for easier porting to different platforms if needed.

## Potential Extensions

1. **Registry Monitoring**: Implement a mechanism to monitor specific registry keys for changes.

   ```cpp
   class RegistryMonitor {
   public:
       using Callback = std::function<void(const std::string&, const std::string&)>;

       void addWatch(const std::string& keyName, Callback callback);
       void removeWatch(const std::string& keyName);
       void startMonitoring();
       void stopMonitoring();
   };
   ```

2. **Registry Diff**: Implement a function to compare two registry states and report differences.

   ```cpp
   struct RegistryDiff {
       std::vector<std::string> addedKeys;
       std::vector<std::string> removedKeys;
       std::vector<std::pair<std::string, std::string>> modifiedValues;
   };

   RegistryDiff compareRegistry(const Registry& before, const Registry& after);
   ```

3. **Registry Serialization**: Add methods to serialize and deserialize registry data to/from various formats (e.g., JSON, XML).

   ```cpp
   std::string serializeToJson(const Registry& registry);
   void deserializeFromJson(Registry& registry, const std::string& json);
   ```

4. **Transactional Operations**: Implement transactional support for registry operations.

   ```cpp
   class RegistryTransaction {
   public:
       RegistryTransaction(Registry& registry);
       void setValue(const std::string& keyName, const std::string& valueName, const std::string& data);
       void deleteValue(const std::string& keyName, const std::string& valueName);
       void commit();
       void rollback();
   };
   ```

5. **Registry Permissions**: Add methods to manage and check registry permissions.

   ```cpp
   bool hasWriteAccess(const std::string& keyName);
   void setKeyPermissions(const std::string& keyName, const Permissions& permissions);
   ```

By considering these potential extensions, you can enhance the functionality of the `Registry` class to meet more advanced requirements and use cases.
