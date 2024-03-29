# Super Enhanced String Class Documentation

The `String` class is a super enhanced string class with various operations and functionalities. Below is the detailed documentation for its features and functions.

## Constructors

### String()

- **Brief**: Default constructor.

### String(const char \*str)

- **Brief**: Constructor accepting a C-style string as input.

### String(const std::string &str)

- **Brief**: Constructor accepting a `std::string` as input.

### String(const String &other)

- **Brief**: Copy constructor.

## Member Functions

### operator=(const String &other)

- **Brief**: Copy assignment.

### operator==(const String &other) const

- **Brief**: Equality comparison.

### operator!=(const String &other) const

- **Brief**: Inequality comparison.

### operator bool() const

- **Brief**: Check if the string is not empty.

### operator<(const String &other) const

- **Brief**: Less than comparison.

### operator>(const String &other) const

- **Brief**: Greater than comparison.

### operator<=(const String &other) const

- **Brief**: Less than or equal comparison.

### operator>=(const String &other) const

- **Brief**: Greater than or equal comparison.

### operator+=(const String &other)

- **Brief**: Concatenation with another `String`.

### operator+=(const char \*str)

- **Brief**: Concatenation with a C-style string.

### operator+=(char c)

- **Brief**: Concatenation with a single character.

### toCharArray() const

- **Brief**: Get C-style string.

### length() const

- **Brief**: Get the length of the string.

### substring(size_t pos, size_t len) const

- **Brief**: Get a substring from the string.

### find(const String &str, size_t pos) const

- **Brief**: Find a sub-string within the string.

### replace(const String &oldStr, const String &newStr)

- **Brief**: Replace occurrences of a sub-string with another sub-string.

### toUpperCase() const

- **Brief**: Convert the string to uppercase.

### toLowerCase() const

- **Brief**: Convert the string to lowercase.

### split(const String &delimiter) const

- **Brief**: Split the string by a delimiter.

### join(const std::vector<`String`> &strings, const String &separator)

- **Brief**: Join a vector of strings using a separator.

### replaceAll(const String &oldStr, const String &newStr)

- **Brief**: Replace all occurrences of a sub-string with another sub-string.

### insertChar(size_t pos, char c)

- **Brief**: Insert a character at a specified position.

### deleteChar(size_t pos)

- **Brief**: Delete a character at a specified position.

### reverse() const

- **Brief**: Get the reverse of the string.

### equalsIgnoreCase(const String &other) const

- **Brief**: Case-insensitive equality comparison.

### indexOf(const String &subStr, size_t startPos) const

- **Brief**: Get the index of a sub-string starting from a specified position.

### trim()

- **Brief**: Remove leading and trailing whitespaces from the string.

### startsWith(const String &prefix) const

- **Brief**: Check if the string starts with a specific prefix.

### endsWith(const String &suffix) const

- **Brief**: Check if the string ends with a specific suffix.

### escape() const

- **Brief**: Escape special characters in the string.

### unescape() const

- **Brief**: Unescape special characters in the string.

### toInt() const

- **Brief**: Convert the string to an integer.

### toFloat() const

- **Brief**: Convert the string to a floating-point number.

### format(const char \*format, ...)

- **Brief**: Format a string using printf-style formatting.

## Non-member Function

### operator+(const String &lhs, const String &rhs)

- **Brief**: Concatenate two strings.

## Static Member

### npos

- **Brief**: Constant representing 'not found' or 'invalid' position.

## Usage Example

```cpp
#include "String.h"

int main() {
    String s1("Hello");
    String s2("World");

    // Concatenation
    String result = s1 + " " + s2;
    std::cout << result.toCharArray() << std::endl;

    // Conversion to uppercase
    String upper = result.toUpperCase();
    std::cout << upper.toCharArray() << std::endl;

    // Find operation
    size_t pos = result.find("World", 0);
    std::cout << "Position of 'World': " << pos << std::endl;

    return 0;
}
```
