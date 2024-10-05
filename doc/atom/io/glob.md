# atom::io Namespace Documentation

The `atom::io` namespace provides utilities for file system operations, particularly focusing on glob-style pattern matching for file and directory paths. This document covers the main functions and their usage.

## Table of Contents

1. [glob Functions](#glob-functions)
2. [rglob Functions](#rglob-functions)
3. [Helper Functions](#helper-functions)

## glob Functions

The `glob` functions are used to find pathnames matching a specified pattern.

### Single Pattern Glob

```cpp
static ATOM_INLINE auto glob(const std::string &pathname) -> std::vector<fs::path>
```

This function takes a single pathname pattern and returns a vector of matching filesystem paths.

#### Usage:

```cpp
auto matches = atom::io::glob("*.txt");
```

#### Example:

```cpp
#include <iostream>
#include "atom/io/glob.hpp"

int main() {
    for (const auto& match : atom::io::glob("*.cpp")) {
        std::cout << match << std::endl;
    }
    return 0;
}
```

This will print all `.cpp` files in the current directory.

### Multiple Pattern Glob

```cpp
static ATOM_INLINE auto glob(const std::vector<std::string> &pathnames) -> std::vector<fs::path>
```

This overload takes a vector of pathname patterns and returns a vector of matching filesystem paths.

#### Usage:

```cpp
auto matches = atom::io::glob({"*.txt", "*.cpp"});
```

#### Example:

```cpp
#include <iostream>
#include "atom/io/glob.hpp"

int main() {
    for (const auto& match : atom::io::glob({"*.txt", "*.cpp"})) {
        std::cout << match << std::endl;
    }
    return 0;
}
```

This will print all `.txt` and `.cpp` files in the current directory.

### Initializer List Glob

```cpp
static ATOM_INLINE auto glob(const std::initializer_list<std::string> &pathnames) -> std::vector<fs::path>
```

This overload allows you to pass an initializer list of pathname patterns.

#### Usage:

```cpp
auto matches = atom::io::glob({"*.txt", "*.cpp", "*.h"});
```

#### Example:

```cpp
#include <iostream>
#include "atom/io/glob.hpp"

int main() {
    for (const auto& match : atom::io::glob({"*.txt", "*.cpp", "*.h"})) {
        std::cout << match << std::endl;
    }
    return 0;
}
```

This will print all `.txt`, `.cpp`, and `.h` files in the current directory.

## rglob Functions

The `rglob` functions are recursive versions of the `glob` functions. They search for matching pathnames in the current directory and all subdirectories.

### Single Pattern Recursive Glob

```cpp
static ATOM_INLINE auto rglob(const std::string &pathname) -> std::vector<fs::path>
```

#### Usage:

```cpp
auto matches = atom::io::rglob("*.txt");
```

#### Example:

```cpp
#include <iostream>
#include "atom/io/glob.hpp"

int main() {
    for (const auto& match : atom::io::rglob("*.cpp")) {
        std::cout << match << std::endl;
    }
    return 0;
}
```

This will recursively print all `.cpp` files in the current directory and its subdirectories.

### Multiple Pattern Recursive Glob

```cpp
static ATOM_INLINE auto rglob(const std::vector<std::string> &pathnames) -> std::vector<fs::path>
```

#### Usage:

```cpp
auto matches = atom::io::rglob({"*.txt", "*.cpp"});
```

### Initializer List Recursive Glob

```cpp
static ATOM_INLINE auto rglob(const std::initializer_list<std::string> &pathnames) -> std::vector<fs::path>
```

#### Usage:

```cpp
auto matches = atom::io::rglob({"*.txt", "*.cpp", "*.h"});
```

## Helper Functions

These functions are used internally by the `glob` and `rglob` functions, but they can also be useful in certain situations.

### expandTilde

```cpp
ATOM_INLINE auto expandTilde(fs::path path) -> fs::path
```

Expands the tilde (`~`) in a path to the user's home directory.

### hasMagic

```cpp
ATOM_INLINE auto hasMagic(const std::string &pathname) -> bool
```

Checks if a pathname contains any glob magic characters (`*`, `?`, or `[`).

### isHidden

```cpp
ATOM_INLINE auto isHidden(const std::string &pathname) -> bool
```

Checks if a pathname represents a hidden file or directory (starts with a dot).

### isRecursive

```cpp
ATOM_INLINE auto isRecursive(const std::string &pattern) -> bool
```

Checks if a pattern is recursive (i.e., `**`).

These helper functions are not typically used directly but can be useful for advanced use cases or when implementing custom glob-like functionality.

---
