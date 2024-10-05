# Extra Math Library Documentation

## Overview

The `math.hpp` file provides an extended set of mathematical operations and utility functions for 64-bit unsigned integers. These functions are designed to handle various mathematical operations safely and efficiently.

## Namespace

All functions are defined within the `atom::algorithm` namespace.

```cpp
namespace atom::algorithm {
    // ...
}
```

## Functions

### mulDiv64

```cpp
auto mulDiv64(uint64_t operant, uint64_t multiplier, uint64_t divider) -> uint64_t;
```

Performs a 64-bit multiplication followed by division.

- **Parameters:**
  - `operant`: The first operand for multiplication.
  - `multiplier`: The second operand for multiplication.
  - `divider`: The divisor for the division operation.
- **Returns:** The result of `(operant * multiplier) / divider`.
- **Usage:**

  ```cpp
  uint64_t result = atom::algorithm::mulDiv64(1000, 3, 4);  // result = 750
  ```

### safeAdd

```cpp
auto safeAdd(uint64_t a, uint64_t b) -> uint64_t;
```

Performs a safe addition operation, handling potential overflow.

- **Parameters:**
  - `a`: The first operand for addition.
  - `b`: The second operand for addition.
- **Returns:** The result of `a + b`, or 0 if there is an overflow.
- **Usage:**

  ```cpp
  uint64_t sum = atom::algorithm::safeAdd(UINT64_MAX, 1);  // sum = 0 (overflow)
  ```

### safeMul

```cpp
auto safeMul(uint64_t a, uint64_t b) -> uint64_t;
```

Performs a safe multiplication operation, handling potential overflow.

- **Parameters:**
  - `a`: The first operand for multiplication.
  - `b`: The second operand for multiplication.
- **Returns:** The result of `a * b`, or 0 if there is an overflow.
- **Usage:**

  ```cpp
  uint64_t product = atom::algorithm::safeMul(UINT64_MAX, 2);  // product = 0 (overflow)
  ```

### rotl64

```cpp
auto rotl64(uint64_t n, unsigned int c) -> uint64_t;
```

Rotates a 64-bit integer to the left by a specified number of bits.

- **Parameters:**
  - `n`: The 64-bit integer to rotate.
  - `c`: The number of bits to rotate.
- **Returns:** The rotated 64-bit integer.
- **Usage:**

  ```cpp
  uint64_t rotated = atom::algorithm::rotl64(0x1234567890ABCDEF, 4);
  ```

### rotr64

```cpp
auto rotr64(uint64_t n, unsigned int c) -> uint64_t;
```

Rotates a 64-bit integer to the right by a specified number of bits.

- **Parameters:**
  - `n`: The 64-bit integer to rotate.
  - `c`: The number of bits to rotate.
- **Returns:** The rotated 64-bit integer.
- **Usage:**

  ```cpp
  uint64_t rotated = atom::algorithm::rotr64(0x1234567890ABCDEF, 4);
  ```

### clz64

```cpp
auto clz64(uint64_t x) -> int;
```

Counts the leading zeros in a 64-bit integer.

- **Parameters:**
  - `x`: The 64-bit integer to count leading zeros in.
- **Returns:** The number of leading zeros in the 64-bit integer.
- **Usage:**

  ```cpp
  int leadingZeros = atom::algorithm::clz64(0x0000FFFFFFFFFFFF);  // leadingZeros = 16
  ```

### normalize

```cpp
auto normalize(uint64_t x) -> uint64_t;
```

Normalizes a 64-bit integer by shifting it to the right until the most significant bit is set.

- **Parameters:**
  - `x`: The 64-bit integer to normalize.
- **Returns:** The normalized 64-bit integer.
- **Usage:**

  ```cpp
  uint64_t normalized = atom::algorithm::normalize(0x0000FFFFFFFFFFFF);
  ```

### safeSub

```cpp
auto safeSub(uint64_t a, uint64_t b) -> uint64_t;
```

Performs a safe subtraction operation, handling potential underflow.

- **Parameters:**
  - `a`: The first operand for subtraction.
  - `b`: The second operand for subtraction.
- **Returns:** The result of `a - b`, or 0 if there is an underflow.
- **Usage:**

  ```cpp
  uint64_t diff = atom::algorithm::safeSub(10, 20);  // diff = 0 (underflow)
  ```

### safeDiv

```cpp
auto safeDiv(uint64_t a, uint64_t b) -> uint64_t;
```

Performs a safe division operation, handling potential division by zero.

- **Parameters:**
  - `a`: The numerator for division.
  - `b`: The denominator for division.
- **Returns:** The result of `a / b`, or 0 if there is a division by zero.
- **Usage:**

  ```cpp
  uint64_t quotient = atom::algorithm::safeDiv(10, 0);  // quotient = 0 (division by zero)
  ```

### bitReverse64

```cpp
auto bitReverse64(uint64_t n) -> uint64_t;
```

Calculates the bitwise reverse of a 64-bit integer.

- **Parameters:**
  - `n`: The 64-bit integer to reverse.
- **Returns:** The bitwise reverse of the 64-bit integer.
- **Usage:**

  ```cpp
  uint64_t reversed = atom::algorithm::bitReverse64(0x1234567890ABCDEF);
  ```

### approximateSqrt

```cpp
auto approximateSqrt(uint64_t n) -> uint64_t;
```

Approximates the square root of a 64-bit integer using a fast algorithm.

- **Parameters:**
  - `n`: The 64-bit integer for which to approximate the square root.
- **Returns:** The approximate square root of the 64-bit integer.
- **Usage:**

  ```cpp
  uint64_t sqrt = atom::algorithm::approximateSqrt(1000000);  // sqrt ≈ 1000
  ```

### gcd64

```cpp
auto gcd64(uint64_t a, uint64_t b) -> uint64_t;
```

Calculates the greatest common divisor (GCD) of two 64-bit integers.

- **Parameters:**
  - `a`: The first 64-bit integer.
  - `b`: The second 64-bit integer.
- **Returns:** The greatest common divisor of the two 64-bit integers.
- **Usage:**

  ```cpp
  uint64_t gcd = atom::algorithm::gcd64(48, 18);  // gcd = 6
  ```

### lcm64

```cpp
auto lcm64(uint64_t a, uint64_t b) -> uint64_t;
```

Calculates the least common multiple (LCM) of two 64-bit integers.

- **Parameters:**
  - `a`: The first 64-bit integer.
  - `b`: The second 64-bit integer.
- **Returns:** The least common multiple of the two 64-bit integers.
- **Usage:**

  ```cpp
  uint64_t lcm = atom::algorithm::lcm64(12, 18);  // lcm = 36
  ```

### isPowerOfTwo

```cpp
auto isPowerOfTwo(uint64_t n) -> bool;
```

Checks if a 64-bit integer is a power of two.

- **Parameters:**
  - `n`: The 64-bit integer to check.
- **Returns:** `true` if the 64-bit integer is a power of two, `false` otherwise.
- **Usage:**

  ```cpp
  bool isPower = atom::algorithm::isPowerOfTwo(64);  // isPower = true
  bool notPower = atom::algorithm::isPowerOfTwo(63); // notPower = false
  ```

### nextPowerOfTwo

```cpp
auto nextPowerOfTwo(uint64_t n) -> uint64_t;
```

Calculates the next power of two for a 64-bit integer.

- **Parameters:**
  - `n`: The 64-bit integer for which to calculate the next power of two.
- **Returns:** The next power of two for the 64-bit integer.
- **Usage:**

  ```cpp
  uint64_t nextPower = atom::algorithm::nextPowerOfTwo(63);  // nextPower = 64
  uint64_t sameValue = atom::algorithm::nextPowerOfTwo(64);  // sameValue = 64
  ```

## Best Practices

1. **Overflow and Underflow Handling**: Always use the safe arithmetic functions (`safeAdd`, `safeSub`, `safeMul`, `safeDiv`) when dealing with operations that might result in overflow or underflow.

2. **Bit Manipulation**: For bit manipulation operations, use `rotl64`, `rotr64`, and `bitReverse64` instead of implementing these operations manually to ensure correctness and efficiency.

3. **Integer Square Root**: When an approximate square root is sufficient, use `approximateSqrt` for better performance compared to floating-point operations.

4. **GCD and LCM**: Use `gcd64` and `lcm64` for efficient calculations of greatest common divisor and least common multiple, respectively.

5. **Power of Two Operations**: Utilize `isPowerOfTwo` and `nextPowerOfTwo` when working with power-of-two values, as these are optimized for such operations.

## Example Usage

Here's an example demonstrating the use of several functions from this library:

```cpp
#include "math.hpp"
#include <iostream>

int main() {
    uint64_t a = 1234567890;
    uint64_t b = 987654321;

    // Safe arithmetic operations
    uint64_t sum = atom::algorithm::safeAdd(a, b);
    uint64_t product = atom::algorithm::safeMul(a, b);

    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Product: " << product << std::endl;

    // Bit manipulation
    uint64_t rotated = atom::algorithm::rotl64(a, 8);
    uint64_t reversed = atom::algorithm::bitReverse64(a);

    std::cout << "Rotated left by 8: " << rotated << std::endl;
    std::cout << "Bit-reversed: " << reversed << std::endl;

    // Mathematical operations
    uint64_t sqrt = atom::algorithm::approximateSqrt(a);
    uint64_t gcd = atom::algorithm::gcd64(a, b);
    uint64_t lcm = atom::algorithm::lcm64(a, b);

    std::cout << "Approximate square root: " << sqrt << std::endl;
    std::cout << "GCD: " << gcd << std::endl;
    std::cout << "LCM: " << lcm << std::endl;

    // Power of two operations
    uint64_t n = 63;
    bool isPower = atom::algorithm::isPowerOfTwo(n);
    uint64_t nextPower = atom::algorithm::nextPowerOfTwo(n);

    std::cout << "Is " << n << " a power of two? " << (isPower ? "Yes" : "No") << std::endl;
    std::cout << "Next power of two after " << n << ": " << nextPower << std::endl;

    return 0;
}
```

This example demonstrates how to use various functions from the `math.hpp` library, including safe arithmetic operations, bit manipulation, mathematical calculations, and power-of-two operations.

## Notes

- All functions in this library are designed to work with 64-bit unsigned integers (`uint64_t`).
- The safe arithmetic functions return 0 in case of overflow, underflow, or division by zero. Always check the return value when using these functions in critical calculations.
- The `approximateSqrt` function provides a fast approximation and may not be suitable for applications requiring high precision.
- When working with very large numbers, be aware of the limitations of 64-bit integers and consider using arbitrary-precision arithmetic libraries for more demanding calculations.
