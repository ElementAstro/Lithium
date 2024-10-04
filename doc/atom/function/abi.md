# DemangleHelper Class Documentation

## Overview

The `DemangleHelper` class is designed to provide functionality for demangling C++ type names and visualizing complex types. It is defined in the `atom::meta` namespace and is part of the `abi.hpp` header file. This class offers methods for demangling single and multiple type names, as well as visualizing the structure of complex types when debugging is enabled.

## Class Declaration

```cpp
namespace atom::meta {
class DemangleHelper {
public:
    // Public member functions
    // ... (detailed below)

private:
    // Private member functions
    // ... (detailed below)
};
}
```

## Public Member Functions

### demangleType

```cpp
template <typename T>
static auto demangleType() -> std::string;

template <typename T>
static auto demangleType(const T& instance) -> std::string;
```

Demangles the type name of a given type `T` or an instance of type `T`.

- **Returns:** A string containing the demangled type name.

### demangle

```cpp
static auto demangle(std::string_view mangled_name,
                     const std::optional<std::source_location>& location = std::nullopt) -> std::string;
```

Demangles a given mangled name and optionally includes source location information.

- **Parameters:**
  - `mangled_name`: The mangled name to be demangled.
  - `location`: Optional source location information.
- **Returns:** A string containing the demangled name and optional location information.

### demangleMany

```cpp
static auto demangleMany(const std::vector<std::string_view>& mangled_names,
                         const std::optional<std::source_location>& location = std::nullopt) -> std::vector<std::string>;
```

Demangles multiple mangled names and optionally includes source location information for each.

- **Parameters:**
  - `mangled_names`: A vector of mangled names to be demangled.
  - `location`: Optional source location information.
- **Returns:** A vector of strings containing the demangled names and optional location information.

### visualize (Debug mode only)

```cpp
static auto visualize(const std::string& demangled_name) -> std::string;
```

Visualizes the structure of a demangled type name. This function is only available in debug mode.

- **Parameters:**
  - `demangled_name`: The demangled type name to visualize.
- **Returns:** A string representation of the type's structure.

## Private Member Functions

### demangleInternal

```cpp
static auto demangleInternal(std::string_view mangled_name) -> std::string;
```

Internal function for demangling names. It uses platform-specific demangling functions.

### visualizeType, visualizeTemplateParams, visualizeFunctionParams (Debug mode only)

These are helper functions used by the `visualize` function to create a structured representation of complex types.

## Usage Examples

### Basic Demangling

```cpp
#include "abi.hpp"
#include <iostream>
#include <vector>

int main() {
    // Demangle a type
    std::cout << atom::meta::DemangleHelper::demangleType<std::vector<int>>() << std::endl;

    // Demangle an instance
    std::vector<double> vec;
    std::cout << atom::meta::DemangleHelper::demangleType(vec) << std::endl;

    // Demangle a mangled name
    std::cout << atom::meta::DemangleHelper::demangle("_ZNSt6vectorIiSaIiEE") << std::endl;

    return 0;
}
```

### Demangling Multiple Names

```cpp
#include "abi.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<std::string_view> mangled_names = {
        "_ZNSt6vectorIiSaIiEE",
        "_ZNSt6vectorIdSaIdEE",
        "_ZNSt3mapIiNSt6vectorIiSaIiEESt4lessIiESaISt4pairIKiS1_EEE"
    };

    auto demangled = atom::meta::DemangleHelper::demangleMany(mangled_names);
    for (const auto& name : demangled) {
        std::cout << name << std::endl;
    }

    return 0;
}
```

### Visualizing Types (Debug Mode Only)

```cpp
#include "abi.hpp"
#include <iostream>
#include <vector>
#include <map>

int main() {
    using ComplexType = std::map<int, std::vector<double>>;
    std::string demangled = atom::meta::DemangleHelper::demangleType<ComplexType>();
    std::cout << atom::meta::DemangleHelper::visualize(demangled) << std::endl;

    return 0;
}
```

## Notes

- The `DemangleHelper` class uses platform-specific demangling functions (`UnDecorateSymbolName` on MSVC, `__cxa_demangle` on other platforms).
- The visualization feature is only available when `ENABLE_DEBUG` is defined.
- The class uses C++17 features like `std::optional` and `std::string_view`.
- The `visualize` function provides a tree-like structure representation of complex types, which can be helpful for understanding template instantiations and nested types.
- When using `demangle` or `demangleMany` with source location information, make sure to compile with C++20 support to use `std::source_location`.
