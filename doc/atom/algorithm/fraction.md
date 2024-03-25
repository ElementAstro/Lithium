# Fraction Class

The Fraction class represents a fraction and provides basic fraction operations and functionalities.

## Member Functions

### Constructors

#### `Fraction(int num_value, int den_value)`

Constructs a fraction object with the given numerator and denominator.

#### `Fraction(int num_value)`

Constructs a fraction object with the given integer as the numerator, defaulting the denominator to 1.

#### `Fraction(const char *str)`

Constructs a fraction object from the given string.

#### `Fraction()`

Default constructor that constructs a fraction object with a value of 0.

### Member Methods

#### `int getNumerator() const`

Gets the numerator of the fraction.

#### `int getDenominator() const`

Gets the denominator of the fraction.

#### `void alterValue(int num_value, int den_value)`

Changes the value of the fraction to the specified numerator and denominator.

#### `void alterValue(const Fraction &f)`

Changes the value of the fraction to the value of another fraction object.

#### `Fraction inverse()`

Returns the reciprocal of the fraction.

### Operator Overloading

#### `+`, `-`, `*`, `/`

Implements addition, subtraction, multiplication, and division for fractions.

#### `==`, `!=`, `>`, `>=`, `<`, `<=`

Implements comparison operators for fractions.

#### `<<`, `>>`

Implements input and output stream operations for fraction objects.

#### Negation Operator

Negates a fraction to its opposite value.

### Friend Functions

#### `operator>>`

Reads a fraction object's value from the input stream.

#### `operator<<`

Writes a fraction object's value to the output stream.

## Examples

```cpp
Fraction f1(1, 2);
Fraction f2(3, 4);
Fraction result = f1 + f2; // result = 5/4
std::cout << result; // Output: 5/4
```

```cpp
Fraction f1(2, 3);
Fraction f2(1, 6);
bool isEqual = (f1 == f2); // isEqual = false
```

```cpp
Fraction f1(3, 5);
Fraction f2 = -f1; // f2 = -3/5
```
