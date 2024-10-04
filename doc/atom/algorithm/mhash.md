# MinHash Implementation Documentation

## Overview

The `mhash.hpp` file provides an implementation of the MinHash algorithm for estimating Jaccard similarity between sets. It also includes utility functions for converting between hexadecimal strings and binary data. The implementation supports both CPU and OpenCL-based GPU computation.

## Namespace

All classes and functions are defined within the `atom::algorithm` namespace.

```cpp
namespace atom::algorithm {
    // ...
}
```

## Utility Functions

### hexstringFromData

```cpp
ATOM_NODISCARD auto hexstringFromData(const std::string& data) -> std::string;
```

Converts a string to its hexadecimal string representation.

- **Parameters:**
  - `data`: The input string to convert.
- **Returns:** The hexadecimal string representation of the input data.
- **Usage:**

  ```cpp
  std::string data = "Hello, World!";
  std::string hex = atom::algorithm::hexstringFromData(data);
  // hex will be "48656C6C6F2C20576F726C6421"
  ```

### dataFromHexstring

```cpp
ATOM_NODISCARD auto dataFromHexstring(const std::string& data) -> std::string;
```

Converts a hexadecimal string representation back to binary data.

- **Parameters:**
  - `data`: The input hexadecimal string to convert.
- **Returns:** The binary data represented by the input hexadecimal string.
- **Throws:** `std::invalid_argument` if the input is not a valid hexadecimal string.
- **Usage:**

  ```cpp
  std::string hex = "48656C6C6F2C20576F726C6421";
  std::string data = atom::algorithm::dataFromHexstring(hex);
  // data will be "Hello, World!"
  ```

## MinHash Class

The `MinHash` class implements the MinHash algorithm for estimating Jaccard similarity between sets.

### Class Definition

```cpp
class MinHash {
public:
    using HashFunction = std::function<size_t(size_t)>;

    explicit MinHash(size_t num_hashes);
    ~MinHash();

    template <std::ranges::range Range>
    auto computeSignature(const Range& set) const -> std::vector<size_t>;

    static auto jaccardIndex(const std::vector<size_t>& sig1,
                             const std::vector<size_t>& sig2) -> double;

private:
    // ... (private members and methods)
};
```

### Constructor

```cpp
explicit MinHash(size_t num_hashes);
```

Constructs a MinHash object with a specified number of hash functions.

- **Parameters:**
  - `num_hashes`: The number of hash functions to use for MinHash.
- **Usage:**

  ```cpp
  atom::algorithm::MinHash minhash(100); // Create MinHash with 100 hash functions
  ```

### computeSignature

```cpp
template <std::ranges::range Range>
auto computeSignature(const Range& set) const -> std::vector<size_t>;
```

Computes the MinHash signature (hash values) for a given set.

- **Template Parameters:**
  - `Range`: Type of the range representing the set elements.
- **Parameters:**
  - `set`: The set for which to compute the MinHash signature.
- **Returns:** A vector of size_t representing the MinHash signature for the set.
- **Usage:**

  ```cpp
  std::vector<std::string> set = {"apple", "banana", "cherry"};
  atom::algorithm::MinHash minhash(100);
  std::vector<size_t> signature = minhash.computeSignature(set);
  ```

### jaccardIndex

```cpp
static auto jaccardIndex(const std::vector<size_t>& sig1,
                         const std::vector<size_t>& sig2) -> double;
```

Computes the Jaccard index between two sets based on their MinHash signatures.

- **Parameters:**
  - `sig1`: MinHash signature of the first set.
  - `sig2`: MinHash signature of the second set.
- **Returns:** Estimated Jaccard index between the two sets as a double.
- **Usage:**

  ```cpp
  std::vector<size_t> sig1 = minhash.computeSignature(set1);
  std::vector<size_t> sig2 = minhash.computeSignature(set2);
  double similarity = atom::algorithm::MinHash::jaccardIndex(sig1, sig2);
  ```

## OpenCL Support

The MinHash implementation includes support for OpenCL-based GPU computation when the `USE_OPENCL` macro is defined. When OpenCL is available, the `computeSignature` method will automatically use GPU acceleration for improved performance.

### OpenCL-specific Methods

These methods are only available when `USE_OPENCL` is defined:

- `initializeOpenCL()`: Initializes OpenCL context and resources.
- `cleanupOpenCL()`: Cleans up OpenCL resources.
- `computeSignatureOpenCL()`: Computes the MinHash signature using OpenCL.

## Example Usage

Here's an example demonstrating how to use the MinHash class to estimate the Jaccard similarity between two sets:

```cpp
#include "mhash.hpp"
#include <iostream>
#include <vector>
#include <string>

int main() {
    // Create two sample sets
    std::vector<std::string> set1 = {"apple", "banana", "cherry", "date"};
    std::vector<std::string> set2 = {"banana", "cherry", "elderberry", "fig"};

    // Create a MinHash object with 100 hash functions
    atom::algorithm::MinHash minhash(100);

    // Compute MinHash signatures for both sets
    auto sig1 = minhash.computeSignature(set1);
    auto sig2 = minhash.computeSignature(set2);

    // Estimate Jaccard similarity
    double similarity = atom::algorithm::MinHash::jaccardIndex(sig1, sig2);

    std::cout << "Estimated Jaccard similarity: " << similarity << std::endl;

    // Convert a string to hexadecimal and back
    std::string original = "Hello, World!";
    std::string hex = atom::algorithm::hexstringFromData(original);
    std::string restored = atom::algorithm::dataFromHexstring(hex);

    std::cout << "Original: " << original << std::endl;
    std::cout << "Hex: " << hex << std::endl;
    std::cout << "Restored: " << restored << std::endl;

    return 0;
}
```

This example demonstrates:

1. Creating MinHash objects
2. Computing MinHash signatures for sets
3. Estimating Jaccard similarity between sets
4. Using utility functions for hexadecimal string conversion
