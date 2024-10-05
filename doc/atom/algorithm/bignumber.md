# BigNumber Class Documentation

## Overview

The `BigNumber` class is part of the `atom::algorithm` namespace and provides functionality for handling and manipulating large numbers that exceed the capacity of built-in numeric types. This class allows for arithmetic operations, comparisons, and various utility functions for working with arbitrarily large integers.

## Table of Contents

1. [Constructors](#constructors)
2. [Arithmetic Operations](#arithmetic-operations)
3. [Comparison Operations](#comparison-operations)
4. [Utility Functions](#utility-functions)
5. [Operator Overloads](#operator-overloads)
6. [Usage Examples](#usage-examples)

## Constructors

### String Constructor

```cpp
BigNumber(std::string number)
```

Constructs a `BigNumber` from a string representation of the number.

### Long Long Constructor

```cpp
BigNumber(long long number)
```

Constructs a `BigNumber` from a `long long` integer.

## Arithmetic Operations

### Addition

```cpp
auto add(const BigNumber& other) const -> BigNumber
```

Adds two `BigNumber` objects and returns the result.

### Subtraction

```cpp
auto subtract(const BigNumber& other) const -> BigNumber
```

Subtracts one `BigNumber` from another and returns the result.

### Multiplication

```cpp
auto multiply(const BigNumber& other) const -> BigNumber
```

Multiplies two `BigNumber` objects and returns the result.

### Division

```cpp
auto divide(const BigNumber& other) const -> BigNumber
```

Divides one `BigNumber` by another and returns the result.

### Exponentiation

```cpp
auto pow(int exponent) const -> BigNumber
```

Raises the `BigNumber` to the power of the given exponent.

## Comparison Operations

### Equality

```cpp
auto equals(const BigNumber& other) const -> bool
auto equals(const long long& other) const -> bool
auto equals(const std::string& other) const -> bool
```

Checks if the `BigNumber` is equal to another `BigNumber`, a `long long`, or a string representation.

## Utility Functions

### Get String Representation

```cpp
auto getString() const -> std::string
```

Returns the string representation of the `BigNumber`.

### Set String Representation

```cpp
auto setString(const std::string& newStr) -> BigNumber&
```

Sets a new string representation for the `BigNumber`.

### Negation

```cpp
auto negate() const -> BigNumber
```

Returns the negated value of the `BigNumber`.

### Trim Leading Zeros

```cpp
auto trimLeadingZeros() const -> BigNumber
```

Removes leading zeros from the `BigNumber`.

### Number of Digits

```cpp
auto digits() const -> unsigned int
```

Returns the number of digits in the `BigNumber`.

### Sign Checks

```cpp
auto isNegative() const -> bool
auto isPositive() const -> bool
```

Checks if the `BigNumber` is negative or positive.

### Parity Checks

```cpp
auto isEven() const -> bool
auto isOdd() const -> bool
```

Checks if the `BigNumber` is even or odd.

### Absolute Value

```cpp
auto abs() const -> BigNumber
```

Returns the absolute value of the `BigNumber`.

## Operator Overloads

The `BigNumber` class overloads various operators for ease of use:

- Arithmetic operators: `+`, `-`, `*`, `/`, `^`
- Comparison operators: `==`, `>`, `<`, `>=`, `<=`
- Compound assignment operators: `+=`, `-=`, `*=`, `/=`
- Increment/Decrement operators: `++`, `--` (both prefix and postfix)
- Subscript operator: `[]`
- Stream insertion operator: `<<`

## Usage Examples

```cpp
#include "atom/algorithm/bignumber.hpp"
#include <iostream>

int main() {
    atom::algorithm::BigNumber a("123456789012345678901234567890");
    atom::algorithm::BigNumber b("987654321098765432109876543210");

    // Addition
    std::cout << "a + b = " << a + b << std::endl;

    // Multiplication
    std::cout << "a * b = " << a * b << std::endl;

    // Comparison
    if (a < b) {
        std::cout << "a is less than b" << std::endl;
    }

    // Exponentiation
    std::cout << "a^3 = " << (a ^ 3) << std::endl;

    // Increment
    a++;
    std::cout << "a after increment: " << a << std::endl;

    return 0;
}
```

This example demonstrates basic arithmetic operations, comparisons, and other functionalities of the `BigNumber` class. The class provides a powerful tool for handling large numbers in C++ applications, especially useful in cryptography, scientific computing, and other domains requiring high-precision arithmetic.
