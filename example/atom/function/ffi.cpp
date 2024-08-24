/*!
 * \file ffi_examples.cpp
 * \brief Examples of using FFI functionality.
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/ffi.hpp"

#include <iostream>
#include <string>
#include <string_view>

// Example library with a simple function signature
extern "C" {
int add(int a, int b);
const char* greet(const char* name);
}

// Implementation of the example functions (for testing purposes)
int add(int a, int b) { return a + b; }

const char* greet(const char* name) {
    static std::string greeting;
    greeting = "Hello, " + std::string(name) + "!";
    return greeting.c_str();
}

// Function to demonstrate FFIWrapper
void demoFFIWrapper() {
    using namespace atom::meta;

    FFIWrapper<int, int, int> ffiAddWrapper;

    // Simulate a function pointer to `add`
    void* addFuncPtr = reinterpret_cast<void*>(&add);

    // Call `add` function using FFIWrapper
    int result = ffiAddWrapper.call(addFuncPtr, 3, 4);
    std::cout << "Result of add(3, 4): " << result << std::endl;
}

// Function to demonstrate DynamicLibrary usage
void demoDynamicLibrary() {
    using namespace atom::meta;

    // Create a dynamic library object (assuming the library has been built)
    DynamicLibrary library("./example_library.so");

    // Add functions to the library's function map
    library.addFunction<int(int, int)>("add");
    library.addFunction<const char*(const char*)>("greet");

    // Call functions using the dynamic library
    auto addResult = library.callFunction<int, int, int>("add", 5, 7);
    if (addResult) {
        std::cout << "Result of add(5, 7): " << *addResult << std::endl;
    } else {
        std::cout << "Failed to call add function." << std::endl;
    }

    auto greetResult =
        library.callFunction<const char*, const char*>("greet", "World");
    if (greetResult) {
        std::cout << "Greeting: " << *greetResult << std::endl;
    } else {
        std::cout << "Failed to call greet function." << std::endl;
    }
}

// Function to demonstrate LibraryObject usage
void demoLibraryObject() {
    using namespace atom::meta;

    // Create a dynamic library object
    DynamicLibrary library("./example_library.so");

    // Create a LibraryObject for a factory function
    LibraryObject<int> obj(library, "create_int");

    // Use the object
    int value = *obj;
    std::cout << "Value from LibraryObject: " << value << std::endl;
}

int main() {
    std::cout << "Demonstrating FFI Wrapper:" << std::endl;
    demoFFIWrapper();

    std::cout << "\nDemonstrating Dynamic Library:" << std::endl;
    demoDynamicLibrary();

    std::cout << "\nDemonstrating Library Object:" << std::endl;
    demoLibraryObject();

    return 0;
}
