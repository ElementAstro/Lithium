# Global Shared Pointer Manager Documentation

## Overview

The `global_ptr.hpp` file defines a `GlobalSharedPtrManager` class, which provides a centralized management system for shared pointers and weak pointers in C++. This class is designed as a singleton and offers thread-safe operations for adding, retrieving, and removing pointers.

## Table of Contents

1. [Key Components](#key-components)
2. [Macros](#macros)
3. [GlobalSharedPtrManager Class](#globalsharedptrmanager-class)
4. [Usage Examples](#usage-examples)
5. [Notes and Considerations](#notes-and-considerations)

## Key Components

### GlobalSharedPtrManager

The core of this library is the `GlobalSharedPtrManager` class, which manages a collection of shared pointers and weak pointers.

```cpp
class GlobalSharedPtrManager : public NonCopyable {
    // ... (methods and members)
};
```

### Shared Pointer Map

The class uses either a fast hash map (if `ENABLE_FASTHASH` is defined) or a standard `unordered_map` to store pointers:

```cpp
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> shared_ptr_map_;
#else
    std::unordered_map<std::string, std::any> shared_ptr_map_;
#endif
```

## Macros

The file defines several convenience macros for common operations:

- `GetPtr`: Retrieves a shared pointer
- `GetWeakPtr`: Retrieves a weak pointer from a shared pointer
- `AddPtr`: Adds a shared pointer
- `RemovePtr`: Removes a shared pointer
- `GetPtrOrCreate`: Retrieves or creates a shared pointer
- `GET_OR_CREATE_PTR`: Creates and assigns a shared pointer
- `GET_OR_CREATE_PTR_THIS`: Creates and assigns a shared pointer with `this` context
- `GET_OR_CREATE_WEAK_PTR`: Creates and assigns a weak pointer
- `GET_OR_CREATE_PTR_WITH_DELETER`: Creates and assigns a shared pointer with a custom deleter

## GlobalSharedPtrManager Class

### Key Methods

1. `getInstance()`: Returns the singleton instance of the manager.
2. `getSharedPtr<T>(const std::string& key)`: Retrieves a shared pointer by key.
3. `getOrCreateSharedPtr<T, CreatorFunc>(const std::string& key, CreatorFunc creator)`: Retrieves or creates a shared pointer.
4. `getWeakPtr<T>(const std::string& key)`: Retrieves a weak pointer by key.
5. `addSharedPtr<T>(const std::string& key, std::shared_ptr<T> sharedPtr)`: Adds a shared pointer.
6. `removeSharedPtr(const std::string& key)`: Removes a shared pointer.
7. `addWeakPtr<T>(const std::string& key, const std::weak_ptr<T>& weakPtr)`: Adds a weak pointer.
8. `getSharedPtrFromWeakPtr<T>(const std::string& key)`: Retrieves a shared pointer from a weak pointer.
9. `getWeakPtrFromSharedPtr<T>(const std::string& key)`: Retrieves a weak pointer from a shared pointer.
10. `removeExpiredWeakPtrs()`: Removes all expired weak pointers.
11. `addDeleter<T>(const std::string& key, const std::function<void(T*)>& deleter)`: Adds a custom deleter for a shared object.
12. `deleteObject<T>(const std::string& key, T* ptr)`: Deletes a shared object using a custom deleter if available.
13. `clearAll()`: Removes all pointers from the manager.
14. `size()`: Returns the number of managed pointers.
15. `printSharedPtrMap()`: Prints the contents of the shared pointer map.

## Usage Examples

### Basic Usage

```cpp
#include "global_ptr.hpp"
#include <iostream>

class MyClass {
public:
    void doSomething() { std::cout << "Doing something" << std::endl; }
};

int main() {
    // Create and add a shared pointer
    auto myClassPtr = std::make_shared<MyClass>();
    AddPtr("myClass", myClassPtr);

    // Retrieve and use the shared pointer
    if (auto ptr = GetPtr<MyClass>("myClass")) {
        ptr->doSomething();
    }

    // Remove the shared pointer
    RemovePtr("myClass");

    return 0;
}
```

### Using GET_OR_CREATE_PTR

```cpp
#include "global_ptr.hpp"
#include <iostream>

class MyClass {
public:
    MyClass(int value) : value_(value) {}
    void print() { std::cout << "Value: " << value_ << std::endl; }

private:
    int value_;
};

int main() {
    std::shared_ptr<MyClass> myClassPtr;

    GET_OR_CREATE_PTR(myClassPtr, MyClass, "myClass", 42);

    if (myClassPtr) {
        myClassPtr->print();
    }

    return 0;
}
```

### Using Custom Deleter

```cpp
#include "global_ptr.hpp"
#include <iostream>

class Resource {
public:
    Resource() { std::cout << "Resource acquired" << std::endl; }
    ~Resource() { std::cout << "Resource released" << std::endl; }
};

int main() {
    auto customDeleter = [](Resource* ptr) {
        std::cout << "Custom deleter called" << std::endl;
        delete ptr;
    };

    GET_OR_CREATE_PTR_WITH_DELETER(resourcePtr, Resource, "resource", customDeleter);

    // Use the resource...

    // The custom deleter will be called when the shared_ptr is destroyed
    RemovePtr("resource");

    return 0;
}
```

## Notes and Considerations

1. Thread Safety: The `GlobalSharedPtrManager` uses a `std::shared_mutex` to ensure thread-safe operations.
2. Type Safety: The manager uses `std::any` to store different pointer types, which may lead to type-related issues if not used carefully.
3. Performance: The use of `std::any` and type casting might introduce some performance overhead.
4. Memory Management: Be cautious when using weak pointers, as they do not prevent the pointed object from being deleted.
5. Custom Deleters: The manager supports custom deleters, which can be useful for managing resources that require special cleanup.
6. Singleton Pattern: The `GlobalSharedPtrManager` is implemented as a singleton, which might not be suitable for all use cases.
7. Debugging: The `printSharedPtrMap()` method can be useful for debugging and tracking managed pointers.
8. Flexibility: The manager supports both shared and weak pointers, providing flexibility in pointer management.
