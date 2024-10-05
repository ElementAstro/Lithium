# LocaleWrapper Class Documentation

## Overview

The `LocaleWrapper` class is part of the `atom::extra::boost` namespace and provides a convenient interface for various localization and internationalization operations using the Boost.Locale library. It offers functionality for string conversions, Unicode normalization, tokenization, translation, case conversion, collation, date/time formatting, number formatting, currency formatting, and regular expression operations.

## Class Definition

```cpp
namespace atom::extra::boost {
class LocaleWrapper {
public:
    explicit LocaleWrapper(const std::string& localeName = "");

    // Static methods
    static auto toUtf8(const std::string& str, const std::string& fromCharset) -> std::string;
    static auto fromUtf8(const std::string& str, const std::string& toCharset) -> std::string;
    static auto normalize(const std::string& str, ::boost::locale::norm_type norm = ::boost::locale::norm_default) -> std::string;
    static auto tokenize(const std::string& str, const std::string& localeName = "") -> std::vector<std::string>;
    static auto translate(const std::string& str, const std::string& domain, const std::string& localeName = "") -> std::string;
    static auto formatDate(const ::boost::posix_time::ptime& dateTime, const std::string& format) -> std::string;
    static auto formatNumber(double number, int precision = 2) -> std::string;
    static auto formatCurrency(double amount, const std::string& currency) -> std::string;
    static auto regexReplace(const std::string& str, const ::boost::regex& regex, const std::string& format) -> std::string;

    // Instance methods
    [[nodiscard]] auto toUpper(const std::string& str) const -> std::string;
    [[nodiscard]] auto toLower(const std::string& str) const -> std::string;
    [[nodiscard]] auto toTitle(const std::string& str) const -> std::string;
    [[nodiscard]] auto compare(const std::string& str1, const std::string& str2) const -> int;

    template <typename... Args>
    [[nodiscard]] auto format(const std::string& formatString, Args&&... args) const -> std::string;

private:
    std::locale locale_;
    static constexpr std::size_t K_BUFFER_SIZE = 4096;
};
}
```

## Constructor

### `LocaleWrapper(const std::string& localeName = "")`

Creates a `LocaleWrapper` object with the specified locale. If no locale name is provided, it uses the global locale.

```cpp
atom::extra::boost::LocaleWrapper wrapper("en_US.UTF-8");
```

## Static Methods

### String Conversion

#### `static auto toUtf8(const std::string& str, const std::string& fromCharset) -> std::string`

Converts a string from the specified charset to UTF-8.

```cpp
std::string utf8String = atom::extra::boost::LocaleWrapper::toUtf8("Hello", "ISO-8859-1");
```

#### `static auto fromUtf8(const std::string& str, const std::string& toCharset) -> std::string`

Converts a UTF-8 string to the specified charset.

```cpp
std::string iso88591String = atom::extra::boost::LocaleWrapper::fromUtf8("Hello", "ISO-8859-1");
```

### Unicode Normalization

#### `static auto normalize(const std::string& str, ::boost::locale::norm_type norm = ::boost::locale::norm_default) -> std::string`

Normalizes a Unicode string using the specified normalization form.

```cpp
std::string normalizedStr = atom::extra::boost::LocaleWrapper::normalize("café", ::boost::locale::norm_nfd);
```

### Tokenization

#### `static auto tokenize(const std::string& str, const std::string& localeName = "") -> std::vector<std::string>`

Tokenizes a string into words based on the specified locale.

```cpp
std::vector<std::string> tokens = atom::extra::boost::LocaleWrapper::tokenize("Hello, world!", "en_US.UTF-8");
```

### Translation

#### `static auto translate(const std::string& str, const std::string& domain, const std::string& localeName = "") -> std::string`

Translates a string using the specified domain and locale.

```cpp
std::string translated = atom::extra::boost::LocaleWrapper::translate("Hello", "mydomain", "fr_FR.UTF-8");
```

### Date and Time Formatting

#### `static auto formatDate(const ::boost::posix_time::ptime& dateTime, const std::string& format) -> std::string`

Formats a date and time according to the specified format string.

```cpp
boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
std::string formattedDate = atom::extra::boost::LocaleWrapper::formatDate(now, "%Y-%m-%d %H:%M:%S");
```

### Number Formatting

#### `static auto formatNumber(double number, int precision = 2) -> std::string`

Formats a number with the specified precision.

```cpp
std::string formattedNumber = atom::extra::boost::LocaleWrapper::formatNumber(1234.5678, 2);
```

### Currency Formatting

#### `static auto formatCurrency(double amount, const std::string& currency) -> std::string`

Formats a currency amount with the specified currency code.

```cpp
std::string formattedCurrency = atom::extra::boost::LocaleWrapper::formatCurrency(1234.56, "USD");
```

### Regular Expression Replace

#### `static auto regexReplace(const std::string& str, const ::boost::regex& regex, const std::string& format) -> std::string`

Performs a regular expression replacement on the input string.

```cpp
boost::regex pattern("\\b(\\w+)\\b");
std::string result = atom::extra::boost::LocaleWrapper::regexReplace("Hello world", pattern, "[$1]");
```

## Instance Methods

### Case Conversion

#### `auto toUpper(const std::string& str) const -> std::string`

Converts a string to uppercase.

```cpp
atom::extra::boost::LocaleWrapper wrapper;
std::string upperCase = wrapper.toUpper("Hello");
```

#### `auto toLower(const std::string& str) const -> std::string`

Converts a string to lowercase.

```cpp
atom::extra::boost::LocaleWrapper wrapper;
std::string lowerCase = wrapper.toLower("Hello");
```

#### `auto toTitle(const std::string& str) const -> std::string`

Converts a string to title case.

```cpp
atom::extra::boost::LocaleWrapper wrapper;
std::string titleCase = wrapper.toTitle("hello world");
```

### Collation

#### `auto compare(const std::string& str1, const std::string& str2) const -> int`

Compares two strings using the current locale's collation rules.

```cpp
atom::extra::boost::LocaleWrapper wrapper("fr_FR.UTF-8");
int result = wrapper.compare("école", "ecole");
```

### Message Formatting

#### `template <typename... Args> auto format(const std::string& formatString, Args&&... args) const -> std::string`

Formats a string with named arguments.

```cpp
atom::extra::boost::LocaleWrapper wrapper;
std::string message = wrapper.format("Hello, {1}! Today is {2}.", "Alice", "Monday");
```

## Usage Example

Here's a comprehensive example demonstrating various features of the `LocaleWrapper` class:

```cpp
#include "locale_wrapper.hpp"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

int main() {
    // Create a LocaleWrapper instance with a specific locale
    atom::extra::boost::LocaleWrapper wrapper("en_US.UTF-8");

    // String conversion
    std::string utf8String = atom::extra::boost::LocaleWrapper::toUtf8("Héllo", "ISO-8859-1");
    std::cout << "UTF-8 string: " << utf8String << std::endl;

    // Unicode normalization
    std::string normalizedStr = atom::extra::boost::LocaleWrapper::normalize("café", ::boost::locale::norm_nfd);
    std::cout << "Normalized string: " << normalizedStr << std::endl;

    // Tokenization
    std::vector<std::string> tokens = atom::extra::boost::LocaleWrapper::tokenize("Hello, world! How are you?");
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;

    // Case conversion
    std::cout << "Uppercase: " << wrapper.toUpper("hello") << std::endl;
    std::cout << "Lowercase: " << wrapper.toLower("HELLO") << std::endl;
    std::cout << "Title case: " << wrapper.toTitle("hello world") << std::endl;

    // Collation
    int comparisonResult = wrapper.compare("apple", "banana");
    std::cout << "Comparison result: " << comparisonResult << std::endl;

    // Date formatting
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::string formattedDate = atom::extra::boost::LocaleWrapper::formatDate(now, "%Y-%m-%d %H:%M:%S");
    std::cout << "Formatted date: " << formattedDate << std::endl;

    // Number formatting
    std::string formattedNumber = atom::extra::boost::LocaleWrapper::formatNumber(1234.5678, 2);
    std::cout << "Formatted number: " << formattedNumber << std::endl;

    // Currency formatting
    std::string formattedCurrency = atom::extra::boost::LocaleWrapper::formatCurrency(1234.56, "USD");
    std::cout << "Formatted currency: " << formattedCurrency << std::endl;

    // Regular expression replace
    boost::regex pattern("\\b(\\w+)\\b");
    std::string regexResult = atom::extra::boost::LocaleWrapper::regexReplace("Hello world", pattern, "[$1]");
    std::cout << "Regex replace result: " << regexResult << std::endl;

    // Message formatting
    std::string message = wrapper.format("Hello, {1}! Today is {2}.", "Alice", "Monday");
    std::cout << "Formatted message: " << message << std::endl;

    return 0;
}
```

This example demonstrates the usage of various methods provided by the `LocaleWrapper` class, including string conversion, Unicode normalization, tokenization, case conversion, collation, date/time formatting, number formatting, currency formatting, regular expression operations, and message formatting.

## Best Practices

1. **Locale Initialization**: Initialize the `LocaleWrapper` with the appropriate locale at the beginning of your program or when switching contexts. This ensures consistent behavior across all localization operations.

   ```cpp
   atom::extra::boost::LocaleWrapper wrapper("en_US.UTF-8");
   ```

2. **Error Handling**: Wrap operations that may throw exceptions in try-catch blocks to handle potential errors gracefully.

   ```cpp
   try {
       std::string utf8String = atom::extra::boost::LocaleWrapper::toUtf8("Héllo", "ISO-8859-1");
   } catch (const boost::locale::conv::conversion_error& e) {
       std::cerr << "Conversion error: " << e.what() << std::endl;
   }
   ```

3. **Reuse Instances**: When performing multiple operations with the same locale, reuse the `LocaleWrapper` instance to avoid unnecessary object creation and improve performance.

4. **Consistent Charset Usage**: When converting between charsets, ensure you're using consistent charset names across your application to avoid unexpected behavior.

5. **Normalization**: When comparing or processing Unicode strings, consider normalizing them first to ensure consistent results.

   ```cpp
   std::string str1 = atom::extra::boost::LocaleWrapper::normalize(input1, ::boost::locale::norm_nfc);
   std::string str2 = atom::extra::boost::LocaleWrapper::normalize(input2, ::boost::locale::norm_nfc);
   bool areEqual = (str1 == str2);
   ```

## Advanced Usage Scenarios

### Custom Collation

You can create a custom collation by extending the `LocaleWrapper` class:

```cpp
class CustomCollationWrapper : public atom::extra::boost::LocaleWrapper {
public:
    explicit CustomCollationWrapper(const std::string& localeName) : LocaleWrapper(localeName) {}

    [[nodiscard]] auto customCompare(const std::string& str1, const std::string& str2) const -> int {
        // Implement your custom collation logic here
        // For example, ignore case and diacritics
        std::string normalized1 = normalize(toLower(str1), ::boost::locale::norm_nfd);
        std::string normalized2 = normalize(toLower(str2), ::boost::locale::norm_nfd);
        return compare(normalized1, normalized2);
    }
};
```

### Handling Multiple Locales

When dealing with multiple locales in an application, you can create a `LocaleManager` class:

```cpp
class LocaleManager {
public:
    LocaleManager() {
        locales_["en_US"] = std::make_unique<atom::extra::boost::LocaleWrapper>("en_US.UTF-8");
        locales_["fr_FR"] = std::make_unique<atom::extra::boost::LocaleWrapper>("fr_FR.UTF-8");
        // Add more locales as needed
    }

    auto getWrapper(const std::string& localeName) -> atom::extra::boost::LocaleWrapper& {
        auto it = locales_.find(localeName);
        if (it != locales_.end()) {
            return *(it->second);
        }
        throw std::runtime_error("Unsupported locale: " + localeName);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<atom::extra::boost::LocaleWrapper>> locales_;
};
```

### Thread-Safe Locale Handling

When using `LocaleWrapper` in a multi-threaded environment, consider using thread-local storage for locale-specific operations:

```cpp
class ThreadSafeLocaleWrapper {
public:
    static auto getInstance() -> atom::extra::boost::LocaleWrapper& {
        static thread_local atom::extra::boost::LocaleWrapper instance;
        return instance;
    }

    static void setLocale(const std::string& localeName) {
        getInstance() = atom::extra::boost::LocaleWrapper(localeName);
    }
};

// Usage
void threadFunction() {
    ThreadSafeLocaleWrapper::setLocale("fr_FR.UTF-8");
    auto& wrapper = ThreadSafeLocaleWrapper::getInstance();
    // Use wrapper for locale-specific operations
}
```
