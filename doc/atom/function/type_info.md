# TypeInfo Class and Related Utilities Documentation

This document provides a comprehensive guide to the `TypeInfo` class and its associated utilities in the `atom::meta` namespace. These tools are designed to enhance type handling and provide rich type information at runtime in C++ applications.

## Table of Contents

1. [Introduction](#introduction)
2. [TypeInfo Class](#typeinfo-class)
3. [GetTypeInfo Struct](#gettypeinfo-struct)
4. [User Type Functions](#user-type-functions)
5. [Type Registration and Retrieval](#type-registration-and-retrieval)
6. [Usage Examples](#usage-examples)
7. [Best Practices](#best-practices)

## Introduction

The `TypeInfo` class and its related utilities provide a powerful system for handling and querying type information at runtime. This can be particularly useful for reflection-like capabilities, serialization, and other scenarios where detailed type information is needed.

## TypeInfo Class

The `TypeInfo` class encapsulates detailed information about a type, including its qualifiers, category, and other properties.

### Key Methods and Properties

- `fromType<T>()`: Static method to create a `TypeInfo` instance from a type.
- `fromInstance(const T& instance)`: Static method to create a `TypeInfo` instance from an object.
- `name()`: Returns the demangled name of the type.
- `bareName()`: Returns the demangled name of the bare type (without qualifiers).
- `isConst()`, `isReference()`, `isVoid()`, `isArithmetic()`, `isArray()`, `isEnum()`, `isClass()`, `isFunction()`, `isTrivial()`, `isStandardLayout()`, `isPod()`, `isPointer()`: Methods to query type properties.
- `bareEqual(const TypeInfo&)`: Checks if two `TypeInfo` instances represent the same bare type.
- `bareEqualTypeInfo(const std::type_info&)`: Compares the bare type with a `std::type_info` instance.

### Usage

```cpp
#include "type_info.hpp"

// Create TypeInfo for int
auto intInfo = atom::meta::TypeInfo::fromType<int>();

// Check properties
bool isArithmetic = intInfo.isArithmetic();  // true
bool isClass = intInfo.isClass();  // false

// Get type name
std::string name = intInfo.name();  // "int"
```

## GetTypeInfo Struct

The `GetTypeInfo` struct is a template utility for obtaining `TypeInfo` for different types, including specializations for smart pointers and reference wrappers.

### Usage

```cpp
#include "type_info.hpp"
#include <memory>

// Get TypeInfo for int
auto intInfo = atom::meta::GetTypeInfo<int>::get();

// Get TypeInfo for std::shared_ptr<double>
auto doubleSharedPtrInfo = atom::meta::GetTypeInfo<std::shared_ptr<double>>::get();
```

## User Type Functions

The `userType` functions provide a convenient way to obtain `TypeInfo` for types and instances.

### Usage

```cpp
#include "type_info.hpp"

class MyClass {};

// Get TypeInfo for a type
auto myClassInfo = atom::meta::userType<MyClass>();

// Get TypeInfo from an instance
MyClass instance;
auto instanceInfo = atom::meta::userType(instance);
```

## Type Registration and Retrieval

The library provides functions to register custom types and retrieve their `TypeInfo`.

### Key Functions

- `registerType(const std::string&, TypeInfo)`: Registers a type with a custom name.
- `registerType<T>(const std::string&)`: Registers a type `T` with a custom name.
- `getTypeInfo(const std::string&)`: Retrieves `TypeInfo` for a registered type.

### Usage

```cpp
#include "type_info.hpp"

// Register a custom type
class CustomType {};
atom::meta::registerType<CustomType>("MyCustomType");

// Retrieve TypeInfo for the registered type
auto customTypeInfo = atom::meta::getTypeInfo("MyCustomType");
if (customTypeInfo) {
    std::cout << "Custom type name: " << customTypeInfo->name() << std::endl;
}
```

## Usage Examples

Here are some more comprehensive examples demonstrating the use of `TypeInfo` and related utilities:

### Example 1: Type Property Checking

```cpp
#include "type_info.hpp"
#include <iostream>
#include <vector>

void printTypeProperties(const atom::meta::TypeInfo& info) {
    std::cout << "Type: " << info.name() << std::endl;
    std::cout << "Is const: " << std::boolalpha << info.isConst() << std::endl;
    std::cout << "Is reference: " << info.isReference() << std::endl;
    std::cout << "Is pointer: " << info.isPointer() << std::endl;
    std::cout << "Is arithmetic: " << info.isArithmetic() << std::endl;
    std::cout << "Is class: " << info.isClass() << std::endl;
    std::cout << "Is POD: " << info.isPod() << std::endl;
    std::cout << std::endl;
}

int main() {
    printTypeProperties(atom::meta::TypeInfo::fromType<int>());
    printTypeProperties(atom::meta::TypeInfo::fromType<const double&>());
    printTypeProperties(atom::meta::TypeInfo::fromType<std::vector<int>>());

    return 0;
}
```

### Example 2: Working with Custom Types

```cpp
#include "type_info.hpp"
#include <iostream>

class MyCustomClass {
public:
    int value;
    explicit MyCustomClass(int v) : value(v) {}
};

int main() {
    // Register custom type
    atom::meta::registerType<MyCustomClass>("MyCustomClass");

    // Create an instance
    MyCustomClass obj(42);

    // Get TypeInfo from instance
    auto instanceInfo = atom::meta::userType(obj);

    // Get TypeInfo from registered name
    auto registeredInfo = atom::meta::getTypeInfo("MyCustomClass");

    if (registeredInfo) {
        std::cout << "Registered type name: " << registeredInfo->name() << std::endl;
        std::cout << "Is same as instance type: " << std::boolalpha
                  << instanceInfo.bareEqual(*registeredInfo) << std::endl;
    }

    return 0;
}
```

### Example 3: Type Comparison and Hashing

```cpp
#include "type_info.hpp"
#include <iostream>
#include <unordered_set>

int main() {
    auto intInfo = atom::meta::TypeInfo::fromType<int>();
    auto doubleInfo = atom::meta::TypeInfo::fromType<double>();
    auto constIntInfo = atom::meta::TypeInfo::fromType<const int>();

    // Compare types
    std::cout << "int == double: " << std::boolalpha << (intInfo == doubleInfo) << std::endl;
    std::cout << "int bare == const int: " << intInfo.bareEqual(constIntInfo) << std::endl;

    // Use TypeInfo in standard containers
    std::unordered_set<atom::meta::TypeInfo> typeSet;
    typeSet.insert(intInfo);
    typeSet.insert(doubleInfo);
    typeSet.insert(constIntInfo);

    std::cout << "Number of unique types: " << typeSet.size() << std::endl;

    return 0;
}
```
