# FFI (Foreign Function Interface) System Documentation

## Overview

This document covers the FFI (Foreign Function Interface) system implemented in the `atom::meta` namespace. The system provides a flexible way to interact with dynamic libraries and call functions from these libraries at runtime.

## Key Components

### getFFIType

```cpp
template <typename T>
constexpr auto getFFIType() -> ffi_type*;
```

This function template maps C++ types to their corresponding `ffi_type*`. It supports various integer types, floating-point types, pointers, and strings.

Usage:

```cpp
ffi_type* intType = getFFIType<int>();
ffi_type* doubleType = getFFIType<double>();
ffi_type* stringType = getFFIType<const char*>();
```

### FFIWrapper

```cpp
template <typename ReturnType, typename... Args>
class FFIWrapper;
```

This class wraps the FFI (Foreign Function Interface) call interface for a specific function signature.

Key methods:

- Constructor: Prepares the FFI call interface.
- `call`: Invokes the function with the given arguments.

Usage:

```cpp
FFIWrapper<int, double, const char*> wrapper;
int result = wrapper.call(functionPointer, 3.14, "Hello");
```

### DynamicLibrary

```cpp
class DynamicLibrary;
```

This class manages loading, unloading, and interacting with dynamic libraries.

Key methods:

- Constructor: Loads a dynamic library.
- `getFunction`: Retrieves a function from the library.
- `reload`: Reloads the library.
- `addFunction`: Adds a function to the internal function map.
- `callFunction`: Calls a function from the library.
- `hasFunction`: Checks if a function exists in the library.
- `getBoundFunction`: Gets a bound function from the library.
- `bindFunction`: Creates a std::function wrapper for a library function.

Usage:

```cpp
DynamicLibrary lib("mylib.so");
auto func = lib.getFunction<int(double)>("my_function");
int result = func(3.14);

lib.addFunction<int(double)>("another_function");
auto result = lib.callFunction<int>("another_function", 2.71);

auto boundFunc = lib.bindFunction<int, double>("bound_function");
int boundResult = boundFunc(1.41);
```

### LibraryObject

```cpp
template <typename T>
class LibraryObject;
```

This class template manages objects created by factory functions from a dynamic library.

Usage:

```cpp
DynamicLibrary lib("mylib.so");
LibraryObject<MyClass> obj(lib, "create_my_class");
obj->someMethod();
```

## Detailed Descriptions

### getFFIType

This function template uses compile-time type traits to determine the appropriate `ffi_type*` for a given C++ type. It supports:

- Integer types (signed and unsigned, 8 to 64 bits)
- Floating-point types (float and double)
- Pointers and strings

If an unsupported type is used, a static assertion will fail at compile-time.

### FFIWrapper

The `FFIWrapper` class encapsulates the setup and invocation of FFI calls:

1. The constructor prepares the FFI call interface (`ffi_cif`) based on the function signature.
2. The `call` method invokes the function using `ffi_call`, handling the conversion between C++ and C types.

This class is typically used internally by `DynamicLibrary`.

### DynamicLibrary

The `DynamicLibrary` class provides a high-level interface for working with shared libraries:

1. **Loading and Unloading**:

   - The constructor loads the library using platform-specific calls (`LoadLibraryA` on Windows, `dlopen` on POSIX).
   - The destructor and `unloadLibrary` method handle proper cleanup.

2. **Function Management**:

   - `getFunction`: Retrieves a function pointer and wraps it in a `std::function`.
   - `addFunction`: Adds a function to an internal map for later use.
   - `hasFunction`: Checks if a function has been added to the internal map.

3. **Function Invocation**:

   - `callFunction`: Uses `FFIWrapper` to call a function by name.
   - `getBoundFunction`: Retrieves a function pointer as a `std::function`.
   - `bindFunction`: Creates a `std::function` that wraps the library call, including error handling.

4. **Thread Safety**:

   - Uses a mutex to ensure thread-safe operations on the library handle and function map.

5. **Error Handling**:
   - Throws `FFIException` for various error conditions (e.g., library loading failure, symbol not found).

### LibraryObject

The `LibraryObject` template provides a way to manage C++ objects created by factory functions in the dynamic library:

1. The constructor calls a factory function from the library to create the object.
2. It uses a `std::unique_ptr` to manage the object's lifetime.
3. Provides pointer-like syntax for accessing the managed object.

## Error Handling

The system uses a custom `FFIException` class for error reporting. The `THROW_FFI_EXCEPTION` macro is used to throw these exceptions with detailed error messages, including file, line, and function information.

## Platform Compatibility

The system uses conditional compilation to support both Windows and POSIX-compliant systems:

- On Windows, it uses `LoadLibraryA`, `GetProcAddress`, and `FreeLibrary`.
- On POSIX systems, it uses `dlopen`, `dlsym`, and `dlclose`.

## Performance Considerations

- The system uses a hash map for storing function pointers, providing fast lookup times.
- An optional fast hash implementation (`emhash8::HashMap`) can be used by defining `ENABLE_FASTHASH`.
- The `FFIWrapper` class allows for efficient repeated calls to the same function by preparing the FFI call interface only once.

## Thread Safety

The `DynamicLibrary` class uses a mutex to ensure thread-safe operations on the library handle and function map.

## Notes

- This system requires linking against the `libffi` library.
- It heavily relies on C++17 features like `if constexpr` and `std::optional`.
- Care should be taken when using this system, as it involves raw pointer manipulation and can lead to undefined behavior if used incorrectly.
- The system does not provide automatic type conversion for complex types; users must ensure type compatibility between C++ and the foreign functions.
