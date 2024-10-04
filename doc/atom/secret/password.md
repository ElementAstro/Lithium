# PasswordManager Class Documentation

## Overview

The `PasswordManager` class provides a secure way to store, retrieve, and delete passwords across different platforms (Windows, macOS, and Linux). It uses AES encryption and platform-specific secure storage mechanisms to ensure the safety of sensitive information.

## Table of Contents

1. [AESCipher Class](#aescipher-class)
2. [PasswordManager Class](#passwordmanager-class)
3. [Usage Examples](#usage-examples)
4. [Platform-Specific Implementation Details](#platform-specific-implementation-details)
5. [Best Practices](#best-practices)

## AESCipher Class

The `AESCipher` class provides static methods for encrypting and decrypting strings using AES encryption.

```cpp
class AESCipher {
public:
    static std::string encrypt(const std::string& plaintext, const unsigned char* key);
    static std::string decrypt(const std::string& ciphertext, const unsigned char* key);
};
```

### Methods

1. `encrypt`: Encrypts a plaintext string using the provided key.
2. `decrypt`: Decrypts a ciphertext string using the provided key.

## PasswordManager Class

The `PasswordManager` class manages the storage, retrieval, and deletion of passwords using platform-specific secure storage mechanisms.

```cpp
class PasswordManager {
public:
    PasswordManager();
    void storePassword(const std::string& platformKey, const std::string& password);
    std::string retrievePassword(const std::string& platformKey);
    void deletePassword(const std::string& platformKey);
};
```

### Constructor

```cpp
PasswordManager();
```

Initializes a new `PasswordManager` instance, generating a secure encryption key.

### Methods

1. `storePassword`: Stores an encrypted password for a given platform key.
2. `retrievePassword`: Retrieves and decrypts a password for a given platform key.
3. `deletePassword`: Deletes a stored password for a given platform key.

## Usage Examples

Here's an example of how to use the `PasswordManager` class:

```cpp
#include "password.h"
#include <iostream>

int main() {
    PasswordManager pm;

    // Storing a password
    pm.storePassword("example.com", "mySecurePassword123");
    std::cout << "Password stored for example.com" << std::endl;

    // Retrieving a password
    std::string retrievedPassword = pm.retrievePassword("example.com");
    std::cout << "Retrieved password for example.com: " << retrievedPassword << std::endl;

    // Deleting a password
    pm.deletePassword("example.com");
    std::cout << "Password deleted for example.com" << std::endl;

    // Trying to retrieve a deleted password
    try {
        std::string deletedPassword = pm.retrievePassword("example.com");
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

## Platform-Specific Implementation Details

The `PasswordManager` class uses different secure storage mechanisms depending on the operating system:

### Windows

On Windows, the class uses the Windows Credential Manager:

```cpp
void storeToWindowsCredentialManager(const std::string& target, const std::string& encryptedPassword);
std::string retrieveFromWindowsCredentialManager(const std::string& target);
void deleteFromWindowsCredentialManager(const std::string& target);
```

### macOS

On macOS, the class uses the Keychain:

```cpp
void storeToMacKeychain(const std::string& service, const std::string& account, const std::string& encryptedPassword);
std::string retrieveFromMacKeychain(const std::string& service, const std::string& account);
void deleteFromMacKeychain(const std::string& service, const std::string& account);
```

### Linux

On Linux, the class uses the GNOME Keyring:

```cpp
void storeToLinuxKeyring(const std::string& schema_name, const std::string& attribute_name, const std::string& encryptedPassword);
std::string retrieveFromLinuxKeyring(const std::string& schema_name, const std::string& attribute_name);
void deleteFromLinuxKeyring(const std::string& schema_name, const std::string& attribute_name);
```

## Best Practices

1. **Secure Key Management**: The `PasswordManager` class uses a 16-byte key for AES encryption. Ensure that this key is securely generated and stored.

2. **Platform-Specific Considerations**: Be aware of the platform-specific implementations when deploying your application across different operating systems.

3. **Error Handling**: Implement proper error handling for cases where password storage, retrieval, or deletion might fail.

4. **Memory Management**: Be cautious when handling sensitive information in memory. Consider using secure memory allocation and wiping techniques when dealing with passwords.

5. **Regular Updates**: Keep the `PasswordManager` class updated with the latest security practices and any changes in platform-specific APIs.

6. **User Authentication**: Consider implementing user authentication before allowing access to stored passwords.

7. **Logging**: Implement secure logging practices, avoiding logging of sensitive information such as passwords or encryption keys.

8. **Testing**: Thoroughly test the `PasswordManager` class on all supported platforms to ensure consistent behavior.

9. **Compliance**: Ensure that your password management practices comply with relevant data protection regulations and industry standards.

10. **Backup and Recovery**: Implement secure backup and recovery mechanisms for stored passwords in case of data loss or corruption.
