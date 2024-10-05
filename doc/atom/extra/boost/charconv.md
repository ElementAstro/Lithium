# BoostCharConv Class Documentation

## Overview

The `BoostCharConv` class is part of the `atom::extra::boost` namespace and provides a set of static methods for converting between strings and numeric types (integers and floating-point numbers). It utilizes the Boost library's `charconv` functionality for efficient and customizable conversions.

## Class Definition

```cpp
namespace atom::extra::boost {

class BoostCharConv {
public:
    static auto intToString(T value, int base = DEFAULT_BASE, const FormatOptions& options = {}) -> std::string;
    static auto floatToString(T value, const FormatOptions& options = {}) -> std::string;
    static auto stringToInt(const std::string& str, int base = DEFAULT_BASE) -> T;
    static auto stringToFloat(const std::string& str) -> T;
    static auto toString(T value, const FormatOptions& options = {}) -> std::string;
    static auto fromString(const std::string& str, int base = DEFAULT_BASE) -> T;
    static auto specialValueToString(T value) -> std::string;
};

}
```

## Constants

- `ALIGNMENT`: Set to 16 for memory alignment.
- `DEFAULT_BASE`: Set to 10 for default numeric base in conversions.
- `BUFFER_SIZE`: Set to 128, used for internal conversion buffers.

## Enums

### `NumberFormat`

Defines the format for number representation:

- `GENERAL`: General format (default)
- `SCIENTIFIC`: Scientific notation
- `FIXED`: Fixed-point notation
- `HEX`: Hexadecimal format

## Structs

### `FormatOptions`

Provides formatting options for number-to-string conversions:

```cpp
struct alignas(ALIGNMENT) FormatOptions {
    NumberFormat format = NumberFormat::GENERAL;
    std::optional<int> precision = std::nullopt;
    bool uppercase = false;
    char thousandsSeparator = '\0';
};
```

## Public Methods

### `intToString`

Converts an integer to a string.

```cpp
template <typename T>
static auto intToString(T value, int base = DEFAULT_BASE, const FormatOptions& options = {}) -> std::string;
```

### `floatToString`

Converts a floating-point number to a string.

```cpp
template <typename T>
static auto floatToString(T value, const FormatOptions& options = {}) -> std::string;
```

### `stringToInt`

Converts a string to an integer.

```cpp
template <typename T>
static auto stringToInt(const std::string& str, int base = DEFAULT_BASE) -> T;
```

### `stringToFloat`

Converts a string to a floating-point number.

```cpp
template <typename T>
static auto stringToFloat(const std::string& str) -> T;
```

### `toString`

Generic function to convert a number (integer or floating-point) to a string.

```cpp
template <typename T>
static auto toString(T value, const FormatOptions& options = {}) -> std::string;
```

### `fromString`

Generic function to convert a string to a number (integer or floating-point).

```cpp
template <typename T>
static auto fromString(const std::string& str, int base = DEFAULT_BASE) -> T;
```

### `specialValueToString`

Handles conversion of special floating-point values (NaN, Inf) to strings.

```cpp
template <typename T>
static auto specialValueToString(T value) -> std::string;
```

## Usage Examples

### Integer to String Conversion

```cpp
#include "boost_charconv.hpp"
#include <iostream>

int main() {
    int value = 12345;

    // Basic conversion
    std::string str = atom::extra::boost::BoostCharConv::intToString(value);
    std::cout << "Basic: " << str << std::endl;

    // With formatting options
    atom::extra::boost::FormatOptions options;
    options.thousandsSeparator = ',';
    str = atom::extra::boost::BoostCharConv::intToString(value, 10, options);
    std::cout << "With separator: " << str << std::endl;

    // Hexadecimal
    str = atom::extra::boost::BoostCharConv::intToString(value, 16);
    std::cout << "Hexadecimal: " << str << std::endl;

    return 0;
}
```

Output:

```
Basic: 12345
With separator: 12,345
Hexadecimal: 3039
```

### Float to String Conversion

```cpp
#include "boost_charconv.hpp"
#include <iostream>

int main() {
    double value = 3.14159265359;

    // Basic conversion
    std::string str = atom::extra::boost::BoostCharConv::floatToString(value);
    std::cout << "Basic: " << str << std::endl;

    // With precision
    atom::extra::boost::FormatOptions options;
    options.precision = 2;
    str = atom::extra::boost::BoostCharConv::floatToString(value, options);
    std::cout << "With precision: " << str << std::endl;

    // Scientific notation
    options.format = atom::extra::boost::NumberFormat::SCIENTIFIC;
    str = atom::extra::boost::BoostCharConv::floatToString(value, options);
    std::cout << "Scientific: " << str << std::endl;

    return 0;
}
```

Output:

```
Basic: 3.14159265359
With precision: 3.14
Scientific: 3.14e+00
```

### String to Number Conversion

```cpp
#include "boost_charconv.hpp"
#include <iostream>

int main() {
    std::string intStr = "12345";
    std::string floatStr = "3.14159";

    int intValue = atom::extra::boost::BoostCharConv::stringToInt<int>(intStr);
    double floatValue = atom::extra::boost::BoostCharConv::stringToFloat<double>(floatStr);

    std::cout << "Int value: " << intValue << std::endl;
    std::cout << "Float value: " << floatValue << std::endl;

    return 0;
}
```

Output:

```
Int value: 12345
Float value: 3.14159
```

### Generic Conversion

```cpp
#include "boost_charconv.hpp"
#include <iostream>

int main() {
    int intValue = 42;
    double floatValue = 3.14;

    std::string intStr = atom::extra::boost::BoostCharConv::toString(intValue);
    std::string floatStr = atom::extra::boost::BoostCharConv::toString(floatValue);

    std::cout << "Int to string: " << intStr << std::endl;
    std::cout << "Float to string: " << floatStr << std::endl;

    int newIntValue = atom::extra::boost::BoostCharConv::fromString<int>(intStr);
    double newFloatValue = atom::extra::boost::BoostCharConv::fromString<double>(floatStr);

    std::cout << "String to int: " << newIntValue << std::endl;
    std::cout << "String to float: " << newFloatValue << std::endl;

    return 0;
}
```

Output:

```
Int to string: 42
Float to string: 3.14
String to int: 42
String to float: 3.14
```

#### Best Practices

1. **Error Handling**: Always wrap conversions in try-catch blocks to handle potential exceptions.

```cpp
try {
    int value = atom::extra::boost::BoostCharConv::stringToInt<int>("123abc");
} catch (const std::runtime_error& e) {
    std::cerr << "Conversion error: " << e.what() << std::endl;
}
```

2. **Use Appropriate Types**: Ensure you're using the correct type for your conversions to avoid unexpected results or overflow.

```cpp
// Good practice
long long largeValue = atom::extra::boost::BoostCharConv::stringToInt<long long>("9223372036854775807");

// Potential overflow
int smallValue = atom::extra::boost::BoostCharConv::stringToInt<int>("9223372036854775807"); // This will throw an exception
```

3. **Locale-Aware Conversions**: The `BoostCharConv` class uses locale-independent conversions. If you need locale-aware conversions, consider using `std::locale` with `std::num_put` and `std::num_get`.

4. **Precision Control**: When converting floating-point numbers, use the `precision` option in `FormatOptions` to control the number of decimal places.

```cpp
atom::extra::boost::FormatOptions options;
options.precision = 3;
std::string str = atom::extra::boost::BoostCharConv::floatToString(3.14159265359, options);
// Result: "3.142"
```

5. **Base Specification**: When converting integers to strings or vice versa, always specify the base explicitly if it's not decimal to avoid ambiguity.

```cpp
std::string hexStr = atom::extra::boost::BoostCharConv::intToString(255, 16);
// Result: "ff"

int value = atom::extra::boost::BoostCharConv::stringToInt<int>("ff", 16);
// Result: 255
```

## Advanced Usage

### Handling Special Floating-Point Values

The `specialValueToString` method handles special floating-point values like NaN and Infinity:

```cpp
#include <limits>
#include <iostream>

int main() {
    double nan = std::numeric_limits<double>::quiet_NaN();
    double inf = std::numeric_limits<double>::infinity();

    std::cout << "NaN: " << atom::extra::boost::BoostCharConv::specialValueToString(nan) << std::endl;
    std::cout << "Inf: " << atom::extra::boost::BoostCharConv::specialValueToString(inf) << std::endl;
    std::cout << "-Inf: " << atom::extra::boost::BoostCharConv::specialValueToString(-inf) << std::endl;

    return 0;
}
```

Output:

```
NaN: NaN
Inf: Inf
-Inf: -Inf
```

### Custom Formatting with Thousands Separators

You can use the `thousandsSeparator` option to format large numbers with custom separators:

```cpp
atom::extra::boost::FormatOptions options;
options.thousandsSeparator = '_';
std::string str = atom::extra::boost::BoostCharConv::intToString(1234567890, 10, options);
std::cout << "Formatted large number: " << str << std::endl;
```

Output:

```
Formatted large number: 1_234_567_890
```

### Using Different Number Formats

The `NumberFormat` enum allows you to specify different representations for floating-point numbers:

```cpp
atom::extra::boost::FormatOptions options;
double value = 12345.6789;

options.format = atom::extra::boost::NumberFormat::GENERAL;
std::cout << "General: " << atom::extra::boost::BoostCharConv::floatToString(value, options) << std::endl;

options.format = atom::extra::boost::NumberFormat::SCIENTIFIC;
std::cout << "Scientific: " << atom::extra::boost::BoostCharConv::floatToString(value, options) << std::endl;

options.format = atom::extra::boost::NumberFormat::FIXED;
options.precision = 2;
std::cout << "Fixed: " << atom::extra::boost::BoostCharConv::floatToString(value, options) << std::endl;

options.format = atom::extra::boost::NumberFormat::HEX;
std::cout << "Hexadecimal: " << atom::extra::boost::BoostCharConv::floatToString(value, options) << std::endl;
```

Output:

```
General: 12345.6789
Scientific: 1.23456789e+04
Fixed: 12345.68
Hexadecimal: 1.81cd6b74c8b4396p+13
```

## Performance Considerations

1. **Buffer Size**: The `BUFFER_SIZE` constant (128 bytes) is used for internal conversions. For most use cases, this should be sufficient. However, if you're dealing with extremely large numbers or require high precision, you might need to increase this value.

2. **Alignment**: The `ALIGNMENT` constant (16 bytes) is used for struct alignment. This can affect performance on some architectures. Adjust if necessary based on your target platform.

3. **Reusing FormatOptions**: If you're performing many conversions with the same formatting, create a `FormatOptions` object once and reuse it to avoid unnecessary object creation.

```cpp
atom::extra::boost::FormatOptions options;
options.precision = 2;
options.format = atom::extra::boost::NumberFormat::FIXED;

for (double value : largeArrayOfDoubles) {
    std::string str = atom::extra::boost::BoostCharConv::floatToString(value, options);
    // Process str...
}
```

4. **Exception Handling**: While exceptions provide a robust way to handle errors, they can impact performance in critical loops. In performance-sensitive code, you might want to use error codes or other lightweight error handling mechanisms.

## Extending the Class

If you need to extend the functionality of `BoostCharConv`, consider the following approaches:

1. **Adding New Formats**: You can extend the `NumberFormat` enum and update the `getFloatFormat` and `getIntegerFormat` methods to support new formats.

2. **Custom Type Support**: If you need to support custom types, you can add new specializations of the `toString` and `fromString` methods.

3. **Locale Support**: If locale-aware conversions are needed, you could add methods that take a `std::locale` parameter and use it for conversions.

Remember to maintain the existing error handling and performance characteristics when extending the class.
