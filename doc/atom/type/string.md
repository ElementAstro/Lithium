# Enhanced String Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructors](#constructors)
4. [Operators](#operators)
5. [Basic String Operations](#basic-string-operations)
6. [Advanced String Operations](#advanced-string-operations)
7. [Utility Methods](#utility-methods)
8. [Static Methods](#static-methods)
9. [Usage Examples](#usage-examples)
10. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The `String` class is a feature-rich wrapper around the standard C++ `std::string`, providing a wide range of string manipulation operations. It is designed to be more convenient and powerful than the standard string class, offering methods for common string operations and some advanced functionalities.

## Class Overview

```cpp
class String {
public:
    // Constructors
    String();
    String(const char* str);
    String(std::string_view str);
    String(std::string str);
    String(const String& other);
    String(String&& other) ATOM_NOEXCEPT;

    // Operators
    auto operator=(const String& other) -> String&;
    auto operator=(String&& other) ATOM_NOEXCEPT -> String&;
    auto operator==(const String& other) const -> bool;
    auto operator<=>(const String& other) const;
    auto operator+=(const String& other) -> String&;
    auto operator+=(const char* str) -> String&;
    auto operator+=(char c) -> String&;

    // Methods
    // ... (various methods listed in the following sections)

    static constexpr size_t NPOS = std::string::npos;

private:
    std::string m_data_;
};
```

## Constructors

1. Default constructor: `String()`
2. C-style string constructor: `String(const char* str)`
3. String view constructor: `String(std::string_view str)`
4. Standard string constructor: `String(std::string str)`
5. Copy constructor: `String(const String& other)`
6. Move constructor: `String(String&& other) ATOM_NOEXCEPT`

## Operators

1. Copy assignment: `auto operator=(const String& other) -> String&`
2. Move assignment: `auto operator=(String&& other) ATOM_NOEXCEPT -> String&`
3. Equality comparison: `auto operator==(const String& other) const -> bool`
4. Three-way comparison: `auto operator<=>(const String& other) const`
5. Concatenation:
   - `auto operator+=(const String& other) -> String&`
   - `auto operator+=(const char* str) -> String&`
   - `auto operator+=(char c) -> String&`

## Basic String Operations

1. Get C-style string: `auto cStr() const -> const char*`
2. Get length: `auto length() const -> size_t`
3. Get substring: `auto substr(size_t pos, size_t count = std::string::npos) const -> String`
4. Find substring: `auto find(const String& str, size_t pos = 0) const -> size_t`
5. Replace first occurrence: `auto replace(const String& oldStr, const String& newStr) -> bool`
6. Replace all occurrences: `auto replaceAll(const String& oldStr, const String& newStr) -> size_t`
7. Convert to uppercase: `auto toUpper() const -> String`
8. Convert to lowercase: `auto toLower() const -> String`
9. Split string: `auto split(const String& delimiter) const -> std::vector<String>`
10. Join strings: `static auto join(const std::vector<String>& strings, const String& separator) -> String`
11. Insert character: `void insert(size_t pos, char c)`
12. Erase substring: `void erase(size_t pos = 0, size_t count = std::string::npos)`
13. Reverse string: `auto reverse() const -> String`

## Advanced String Operations

1. Case-insensitive comparison: `auto equalsIgnoreCase(const String& other) const -> bool`
2. Check if starts with: `auto startsWith(const String& prefix) const -> bool`
3. Check if ends with: `auto endsWith(const String& suffix) const -> bool`
4. Trim whitespace: `void trim()`, `void ltrim()`, `void rtrim()`
5. Replace character: `auto replace(char oldChar, char newChar) -> size_t`
6. Remove character: `auto remove(char ch) -> size_t`
7. Pad left: `auto padLeft(size_t totalLength, char paddingChar = ' ') -> String&`
8. Pad right: `auto padRight(size_t totalLength, char paddingChar = ' ') -> String&`
9. Remove prefix: `auto removePrefix(const String& prefix) -> bool`
10. Remove suffix: `auto removeSuffix(const String& suffix) -> bool`
11. Check if contains substring: `auto contains(const String& str) const -> bool`
12. Check if contains character: `auto contains(char c) const -> bool`
13. Compress spaces: `void compressSpaces()`
14. Reverse words: `auto reverseWords() const -> String`
15. Replace using regex: `auto replaceRegex(const std::string& pattern, const std::string& replacement) -> String`

## Utility Methods

1. Get underlying data: `auto data() const -> std::string`
2. Check if empty: `auto empty() const -> bool`

## Static Methods

1. Format string: `static auto format(std::string_view format_str, Args&&... args) -> std::string`

## Usage Examples

### Basic String Operations

```cpp
#include "string.hpp"
#include <iostream>

int main() {
    String s1("Hello, World!");
    String s2 = " Welcome to C++!";

    // Concatenation
    s1 += s2;
    std::cout << "Concatenated string: " << s1 << std::endl;

    // Substring
    String sub = s1.substr(0, 5);
    std::cout << "Substring: " << sub << std::endl;

    // Find
    size_t pos = s1.find("World");
    if (pos != String::NPOS) {
        std::cout << "Found 'World' at position: " << pos << std::endl;
    }

    // Replace
    s1.replace("World", "Universe");
    std::cout << "After replace: " << s1 << std::endl;

    // To upper and lower
    std::cout << "Uppercase: " << s1.toUpper() << std::endl;
    std::cout << "Lowercase: " << s1.toLower() << std::endl;

    return 0;
}
```

### Advanced String Operations

```cpp
#include "string.hpp"
#include <iostream>

int main() {
    String s("  Hello,   World!  How are   you?  ");

    // Trim and compress spaces
    s.trim();
    s.compressSpaces();
    std::cout << "Trimmed and compressed: '" << s << "'" << std::endl;

    // Split and join
    auto words = s.split(" ");
    std::cout << "Word count: " << words.size() << std::endl;
    String joined = String::join(words, "-");
    std::cout << "Joined with dashes: " << joined << std::endl;

    // Reverse words
    String reversed = s.reverseWords();
    std::cout << "Reversed words: " << reversed << std::endl;

    // Pad left and right
    String num = "42";
    std::cout << "Padded left: '" << num.padLeft(5, '0') << "'" << std::endl;
    std::cout << "Padded right: '" << num.padRight(5, '*') << "'" << std::endl;

    // Case-insensitive comparison
    String s1 = "Hello";
    String s2 = "hello";
    if (s1.equalsIgnoreCase(s2)) {
        std::cout << "Strings are equal (case-insensitive)" << std::endl;
    }

    return 0;
}
```

### Regex and Formatting

```cpp
#include "string.hpp"
#include <iostream>

int main() {
    String text = "The quick brown fox jumps over the lazy dog";

    // Replace using regex
    String result = text.replaceRegex("\\b\\w{4}\\b", "XXXX");
    std::cout << "After regex replace: " << result << std::endl;

    // Formatting
    int age = 30;
    double height = 1.75;
    String formatted = String::format("Age: {}, Height: {:.2f}m", age, height);
    std::cout << "Formatted string: " << formatted << std::endl;

    return 0;
}
```

## Advanced Usage Examples

### Custom String Manipulation

```cpp
#include "string.hpp"
#include <iostream>
#include <algorithm>

// Custom function to capitalize every other character
String capitalizeAlternate(const String& input) {
    String result = input;
    for (size_t i = 0; i < result.length(); i += 2) {
        result[i] = std::toupper(result[i]);
    }
    return result;
}

int main() {
    String s = "hello world";

    // Using custom function
    String alternateCapitalized = capitalizeAlternate(s);
    std::cout << "Alternate capitalized: " << alternateCapitalized << std::endl;

    // Using lambda with replaceAll
    s.replaceAll("o", [](const String&) { return String("0"); });
    std::cout << "After replacing 'o' with '0': " << s << std::endl;

    return 0;
}
```

### String Parsing and Manipulation

```cpp
#include "string.hpp"
#include <iostream>
#include <vector>

int main() {
    String csvData = "John,Doe,30,New York,Engineer";

    // Split the CSV data
    std::vector<String> fields = csvData.split(",");

    // Process each field
    for (size_t i = 0; i < fields.size(); ++i) {
        fields[i].trim();
        std::cout << "Field " << i + 1 << ": " << fields[i] << std::endl;
    }

    // Join fields with a different delimiter
    String joined = String::join(fields, " | ");
    std::cout << "Joined data: " << joined << std::endl;

    // Extract and manipulate specific fields
    String name = fields[0] + " " + fields[1];
    String location = fields[3];

    std::cout << "Name: " << name.toUpper() << std::endl;
    std::cout << "Location: " << location.padRight(15, '.') << std::endl;

    return 0;
}
```

### Working with File Paths

```cpp
#include "string.hpp"
#include <iostream>

int main() {
    String filePath = "/home/user/documents/report.txt";

    // Extract filename
    size_t lastSlash = filePath.find("/");
    String filename = (lastSlash != String::NPOS) ? filePath.substr(lastSlash + 1) : filePath;
    std::cout << "Filename: " << filename << std::endl;

    // Extract file extension
    size_t lastDot = filename.find(".");
    String extension = (lastDot != String::NPOS) ? filename.substr(lastDot + 1) : "";
    std::cout << "Extension: " << extension << std::endl;

    // Change file extension
    if (!extension.empty()) {
        filePath.removeSuffix("." + extension);
        filePath += ".pdf";
    }
    std::cout << "New file path: " << filePath << std::endl;

    return 0;
}
```

## Best Practices and Considerations

1. **Performance**: While the `String` class provides many convenient methods, be mindful of performance when chaining multiple operations. Some operations may create temporary String objects.

2. **Memory Management**: The `String` class handles memory management internally, but be aware of potential copies when passing `String` objects by value.

3. **Thread Safety**: The `String` class is not inherently thread-safe. If you need to use `String` objects in a multi-threaded environment, ensure proper synchronization.

4. **Immutability**: Many methods return new `String` objects instead of modifying the original. This promotes immutability but may have performance implications for large strings or frequent modifications.

5. **Unicode Support**: The current implementation primarily works with ASCII characters. For full Unicode support, consider using a Unicode-aware library or extending the `String` class.

6. **Regular Expressions**: The `replaceRegex` method provides basic regex support. For more complex regex operations, consider using the full `<regex>` library directly.

7. **Formatting**: The `format` method uses C++20's `std::format`. Ensure your compiler supports this feature or provide a fallback for older compilers.

8. **Custom Operations**: The `String` class can be easily extended with custom methods. Consider subclassing or adding free functions for domain-specific string operations.

9. **Comparison with std::string**: While `String` provides many convenient methods, `std::string` might be more appropriate in some contexts, especially when interfacing with standard library functions or third-party code.

10. **Error Handling**: Most methods do not throw exceptions for invalid operations (e.g., out-of-range access). Consider adding bounds checking and exception throwing for more robust error handling in critical applications.

## Conclusion

The `String` class provides a rich set of string manipulation tools that can simplify many common text processing tasks in C++. By leveraging its methods, you can write more expressive and concise code for string-heavy applications. Remember to consider performance implications when working with very large strings or in performance-critical sections of your code.
