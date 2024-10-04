# Fraction Class Documentation

## Overview

The `Fraction` class, defined in `fraction.hpp`, is part of the `atom::algorithm` namespace. It provides a robust implementation of fractions, supporting various arithmetic operations, comparisons, and conversions.

## Class Definition

```cpp
namespace atom::algorithm {
class Fraction {
    // ...
};
}
```

## Member Variables

- `int numerator`: The numerator of the fraction.
- `int denominator`: The denominator of the fraction.

## Constructor

```cpp
explicit Fraction(int n = 0, int d = 1);
```

Constructs a new `Fraction` object.

- Parameters:

  - `n`: The numerator (default is 0)
  - `d`: The denominator (default is 1)

- Usage:
  ```cpp
  Fraction f1;           // Creates fraction 0/1
  Fraction f2(3);        // Creates fraction 3/1
  Fraction f3(3, 4);     // Creates fraction 3/4
  ```

## Arithmetic Operations

### Addition

```cpp
auto operator+=(const Fraction& other) -> Fraction&;
auto operator+(const Fraction& other) const -> Fraction;
```

- Usage:
  ```cpp
  Fraction f1(1, 2);
  Fraction f2(1, 3);
  Fraction result = f1 + f2;  // result is 5/6
  f1 += f2;                   // f1 is now 5/6
  ```

### Subtraction

```cpp
auto operator-=(const Fraction& other) -> Fraction&;
auto operator-(const Fraction& other) const -> Fraction;
```

- Usage:
  ```cpp
  Fraction f1(3, 4);
  Fraction f2(1, 4);
  Fraction result = f1 - f2;  // result is 1/2
  f1 -= f2;                   // f1 is now 1/2
  ```

### Multiplication

```cpp
auto operator*=(const Fraction& other) -> Fraction&;
auto operator*(const Fraction& other) const -> Fraction;
```

- Usage:
  ```cpp
  Fraction f1(2, 3);
  Fraction f2(3, 4);
  Fraction result = f1 * f2;  // result is 1/2
  f1 *= f2;                   // f1 is now 1/2
  ```

### Division

```cpp
auto operator/=(const Fraction& other) -> Fraction&;
auto operator/(const Fraction& other) const -> Fraction;
```

- Usage:
  ```cpp
  Fraction f1(2, 3);
  Fraction f2(3, 4);
  Fraction result = f1 / f2;  // result is 8/9
  f1 /= f2;                   // f1 is now 8/9
  ```

## Comparison Operations

### Equality

```cpp
auto operator==(const Fraction& other) const -> bool;
```

- Usage:
  ```cpp
  Fraction f1(1, 2);
  Fraction f2(2, 4);
  bool areEqual = (f1 == f2);  // areEqual is true
  ```

### Ordering (C++20 and later)

```cpp
auto operator<=>(const Fraction& other) const;
```

This implements the three-way comparison operator, allowing for all comparison operations (`<`, `<=`, `>`, `>=`).

- Usage:
  ```cpp
  Fraction f1(1, 2);
  Fraction f2(3, 4);
  bool isLess = (f1 < f2);     // isLess is true
  bool isGreater = (f1 > f2);  // isGreater is false
  ```

## Conversion Methods

### To Double

```cpp
explicit operator double() const;
[[nodiscard]] auto toDouble() const -> double;
```

- Usage:
  ```cpp
  Fraction f(3, 4);
  double d1 = static_cast<double>(f);  // d1 is 0.75
  double d2 = f.toDouble();            // d2 is 0.75
  ```

### To Float

```cpp
explicit operator float() const;
```

- Usage:
  ```cpp
  Fraction f(1, 2);
  float fl = static_cast<float>(f);  // fl is 0.5f
  ```

### To Integer

```cpp
explicit operator int() const;
```

- Usage:
  ```cpp
  Fraction f(7, 3);
  int i = static_cast<int>(f);  // i is 2 (rounded down)
  ```

### To String

```cpp
[[nodiscard]] auto toString() const -> std::string;
```

- Usage:
  ```cpp
  Fraction f(3, 4);
  std::string s = f.toString();  // s is "3/4"
  ```

## Stream Operations

### Output Stream

```cpp
friend auto operator<<(std::ostream& os, const Fraction& f) -> std::ostream&;
```

- Usage:
  ```cpp
  Fraction f(3, 4);
  std::cout << f;  // Outputs: 3/4
  ```

### Input Stream

```cpp
friend auto operator>>(std::istream& is, Fraction& f) -> std::istream&;
```

- Usage:
  ```cpp
  Fraction f;
  std::cin >> f;  // Input format: numerator/denominator (e.g., 3/4)
  ```

## Private Helper Methods

### Greatest Common Divisor (GCD)

```cpp
static int gcd(int a, int b);
```

Computes the greatest common divisor of two numbers.

### Reduce

```cpp
void reduce();
```

Reduces the fraction to its simplest form.

## Best Practices

1. Always check for division by zero when creating fractions or performing division operations.
2. Use the `toDouble()` method or explicit casting when you need to perform floating-point operations with fractions.
3. Remember that integer division occurs when converting a fraction to an integer, which may lead to loss of precision.
4. Utilize the comparison operators for accurate comparisons between fractions instead of converting to floating-point numbers.
5. When inputting fractions from a stream, ensure the input is in the correct format (numerator/denominator) to avoid parsing errors.

## Notes

- The class uses C++20 features like the spaceship operator (`<=>`) for comparisons when available.
- The implementation automatically reduces fractions to their simplest form after operations.
- Error handling for division by zero is not explicitly shown in the header and should be implemented in the class methods.
